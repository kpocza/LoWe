using System;
using System.ComponentModel;
using System.Text;
using System.Threading;
using NAudio.Wave;

namespace LoWeExposer.Handlers
{
    class AlsaHandler : HandlerBase
    {
        private readonly ILineLogger _lineLogger;

        public AlsaHandler(ILineLogger lineLogger)
        {
            _lineLogger = lineLogger;
        }

        protected override void backgroundWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            WaveOut waveOut = null;
            AlsaPlayer alsaPlayer = null;

            try
            {
                while (!_cancellationToken.IsCancellationRequested)
                {
                    var client = _tcpListener.AcceptTcpClient();
                    var networkStream = client.GetStream();
                    while (!_cancellationToken.IsCancellationRequested)
                    {
                        var opCode = new byte[4];
                        if (!ReadAll(networkStream, opCode))
                        {
                            Thread.Sleep(10);
                            continue;
                        }

                        if (IsOperation(opCode, "ALSA"))
                        {
                            WriteAll(networkStream, Encoding.ASCII.GetBytes("ASLA"));
                            _lineLogger.LogLine("Socket check");
                            break;
                        }

                        if (IsOperation(opCode, "INIT"))
                        {
                            var initData = new byte[4*4];
                            if (!ReadAll(networkStream, initData))
                                break;

                            var rate = BitConverter.ToInt32(initData, 0);
                            var sampleBits = BitConverter.ToInt32(initData, 4);
                            var channels = BitConverter.ToInt32(initData, 8);
                            var alsaFormat = BitConverter.ToInt32(initData, 12);
                            _lineLogger.LogLine(
                                $"Rate: {rate}, sample bits: {sampleBits}, channels: {channels}, alsa format: {alsaFormat}");

                            waveOut = new WaveOut();
                            alsaPlayer = new AlsaPlayer(rate, sampleBits, channels, alsaFormat, _lineLogger);
                            waveOut.Init(alsaPlayer);
                        }
                        else if (IsOperation(opCode, "PLAY"))
                        {
                            if (alsaPlayer == null)
                                break;

                            var lenData = new byte[4];
                            if (!ReadAll(networkStream, lenData))
                                break;
                            int len = BitConverter.ToInt32(lenData, 0);

                            var buffer = new byte[len];
                            if (!ReadAll(networkStream, buffer))
                                break;

                            alsaPlayer.Write(buffer);
                            waveOut.Play();
                        }
                        else if (IsOperation(opCode, "DELA"))
                        {
                            if (alsaPlayer == null)
                                break;

                            var frames = alsaPlayer.GetDelay();

                            var framesDelay = BitConverter.GetBytes(frames);
                            WriteAll(networkStream, framesDelay);
                        }
                        else if (IsOperation(opCode, "DROP"))
                        {
                            if (alsaPlayer == null)
                                break;

                            alsaPlayer.Drop();
                            _lineLogger.LogLine("Drop all pending frames");
                        }
                        else if (IsOperation(opCode, "PAUS"))
                        {
                            if (waveOut == null)
                                break;

                            waveOut.Pause();
                            _lineLogger.LogLine("Pause");
                        }
                        else if (IsOperation(opCode, "RESU"))
                        {
                            if (waveOut == null)
                                break;

                            waveOut.Resume();
                            _lineLogger.LogLine("Resume");
                        }
                        else if (IsOperation(opCode, "CLOS"))
                        {
                            _lineLogger.LogLine("Close");
                            break;
                        }
                    }

                    networkStream.Close();
                    client.Close();
                    waveOut?.Stop();
                    alsaPlayer = null;
                }

                waveOut?.Stop();
            }
            catch (Exception ex)
            {
                _lineLogger.LogLine($"Exception: {ex}");
                _tcpListener.Stop();
            }
        }
    }
}
