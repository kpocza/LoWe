using System;
using System.ComponentModel;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using NAudio.Wave;

namespace LoWeExposer.Handlers
{
    class AlsaHandler
    {
        private readonly int _port;
        private readonly ILineLogger _lineLogger;
        private BackgroundWorker _backgroundWorker;
        private CancellationTokenSource _cancellationTokenSource;
        private CancellationToken _cancellationToken;

        public AlsaHandler(int port, ILineLogger lineLogger)
        {
            _port = port;
            _lineLogger = lineLogger;
        }

        public void Start()
        {
            _cancellationTokenSource = new CancellationTokenSource();
            _cancellationToken = _cancellationTokenSource.Token;

            _backgroundWorker = new BackgroundWorker();
            _backgroundWorker.DoWork += backgroundWorker_DoWork;
            _backgroundWorker.RunWorkerAsync();

        }

        public void Stop()
        {
            _cancellationTokenSource.Cancel();
        }

        private void backgroundWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            WaveOut waveOut = null;
            AlsaPlayer alsaPlayer = null;

            var tcpListener = new TcpListener(IPAddress.Parse("127.0.0.1"), _port);
            tcpListener.Start();

            while (!_cancellationToken.IsCancellationRequested)
            {
                var client = tcpListener.AcceptTcpClient();
                var networkStream = client.GetStream();
                while (!_cancellationToken.IsCancellationRequested)
                {
                    var opCode = new byte[4];
                    if (!ReadAll(networkStream, opCode))
                    {
                        Thread.Sleep(1);
                        continue;
                    }

                    if (IsOperation(opCode, "ALSA"))
                    {
                        WriteAll(networkStream, Encoding.ASCII.GetBytes("ASLA"));
                        networkStream.Close();
                        _lineLogger.LogLine("Socket check");
                        break;
                    }

                    if (IsOperation(opCode, "INIT"))
                    {
                        var initData = new byte[4 * 4];
                        if (!ReadAll(networkStream, initData))
                            break;

                        var rate = BitConverter.ToInt32(initData, 0);
                        var sampleBits = BitConverter.ToInt32(initData, 4);
                        var channels = BitConverter.ToInt32(initData, 8);
                        var alsaFormat = BitConverter.ToInt32(initData, 12);
                        _lineLogger.LogLine($"Rate: {rate}, sample bits: {sampleBits}, channels: {channels}, alsa format: {alsaFormat}");

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
                        networkStream.Close();
                        _lineLogger.LogLine("Close");
                        break;
                    }
                }

                waveOut?.Stop();
                alsaPlayer = null;
            }

            waveOut?.Stop();
            tcpListener.Stop();
        }

        private bool IsOperation(byte[] buffer, string chars)
        {
            if (buffer.Length != chars.Length)
                return false;

            for (var i = 0; i < buffer.Length; i++)
            {
                if (buffer[i] != chars[i])
                    return false;
            }
            return true;
        }

        private bool ReadAll(NetworkStream networkStream, byte[] buffer)
        {
            if (!networkStream.CanRead)
                return false;

            try
            {
                int offset = 0;
                int count = buffer.Length;

                while (count > 0)
                {
                    int size = networkStream.Read(buffer, offset, count);
                    if (size < 1)
                        return false;
                    offset += size;
                    count -= size;
                }
                return true;
            }
            catch (IOException)
            {
                return false;
            }
        }

        private void WriteAll(NetworkStream networkStream, byte[] data)
        {
            networkStream.Write(data, 0, data.Length);
        }
    }
}
