using System;
using System.ComponentModel;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace LoWeExposer.Handlers
{
    class MiceHandler
    {
        private readonly int _port;
        private readonly ILineLogger _lineLogger;
        private BackgroundWorker _backgroundWorker;
        private CancellationTokenSource _cancellationTokenSource;
        private CancellationToken _cancellationToken;
        private MiceState _currentState;
        private MiceState _lastReadState;

        public MiceHandler(int port, ILineLogger lineLogger)
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

        public void SetState(MiceState miceState)
        {
            _currentState = miceState.Clone();
        }

        private void backgroundWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            var tcpListener = new TcpListener(IPAddress.Parse("127.0.0.1"), _port);
            tcpListener.Start();

            while (!_cancellationToken.IsCancellationRequested)
            {
                var client = tcpListener.AcceptTcpClient();
                var networkStream = client.GetStream();
                bool isInititalized = false;
                while (!_cancellationToken.IsCancellationRequested)
                {
                    var opCode = new byte[4];
                    if (!ReadAll(networkStream, opCode))
                    {
                        Thread.Sleep(1);
                        continue;
                    }

                    if (IsOperation(opCode, "MICE"))
                    {
                        WriteAll(networkStream, Encoding.ASCII.GetBytes("ECIM"));
                        networkStream.Close();
                        _lineLogger.LogLine("Socket check");
                        break;
                    }

                    if (IsOperation(opCode, "INIT"))
                    {
                        isInititalized = true;
                    }
                    else if (IsOperation(opCode, "READ"))
                    {
                        if (!isInititalized)
                            break;

                        var respData = new byte[1 + 2 * 4];
                        if (_currentState == null)
                        {
                            respData[0] = 0xff;
                            WriteAll(networkStream, respData);
                            continue;
                        }

                        var actualCurrentState = _currentState.Clone();
                        if (_lastReadState == null)
                            _lastReadState = actualCurrentState;

                        int xdiff = actualCurrentState.X - _lastReadState.X;
                        int ydiff = actualCurrentState.Y - _lastReadState.Y;

                        respData[0] = (byte) ((actualCurrentState.LeftButtonDown ? 1 : 0) +
                                              (actualCurrentState.RightButtonDown ? 2 : 0));
                        Array.Copy(BitConverter.GetBytes(xdiff), 0, respData, 1, 4);
                        Array.Copy(BitConverter.GetBytes(ydiff), 0, respData, 5, 4);

                        WriteAll(networkStream, respData);

                        _lastReadState = actualCurrentState;
                    }
                    else if (IsOperation(opCode, "CLOS"))
                    {
                        if (!isInititalized)
                            break;

                        networkStream.Close();
                        _lineLogger.LogLine("Close");
                        break;
                    }
                }
            }
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
