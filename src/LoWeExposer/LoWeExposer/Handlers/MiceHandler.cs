using System;
using System.Collections.Generic;
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
        private readonly Queue<MiceState> _states;
        private MiceState _lastReadState;
        private readonly object _lockObj;
        private TcpListener _tcpListener;

        public MiceHandler(int port, ILineLogger lineLogger)
        {
            _port = port;
            _lineLogger = lineLogger;
            _states = new Queue<MiceState>();
            _lockObj = new object();
        }

        public void Start()
        {
            _cancellationTokenSource = new CancellationTokenSource();
            _cancellationToken = _cancellationTokenSource.Token;

            _tcpListener = new TcpListener(IPAddress.Parse("127.0.0.1"), _port);
            _tcpListener.Start();

            _backgroundWorker = new BackgroundWorker();
            _backgroundWorker.DoWork += backgroundWorker_DoWork;
            _backgroundWorker.RunWorkerAsync();
        }

        public void Stop()
        {
            _tcpListener.Stop();
            _cancellationTokenSource.Cancel();
        }

        public void AddState(MiceState miceState)
        {
            lock (_lockObj)
            {
                _states.Enqueue(miceState);
            }
        }

        public void ClearQueue()
        {
            lock (_lockObj)
            {
                _states.Clear();
            }
        }

        private void backgroundWorker_DoWork(object sender, DoWorkEventArgs e)
        {

            try
            {
                while (!_cancellationToken.IsCancellationRequested)
                {
                    var client = _tcpListener.AcceptTcpClient();
                    var networkStream = client.GetStream();
                    bool isInititalized = false;
                    while (!_cancellationToken.IsCancellationRequested && !_tcpListener.Pending())
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

                            var respData = new byte[1 + 2*4 + 1];
                            lock (_lockObj)
                            {
                                if (_states.Count == 0)
                                {
                                    if (_lastReadState != null)
                                    {
                                        respData[0] = (byte) ((_lastReadState.LeftButtonDown ? 1 : 0) +
                                                              (_lastReadState.RightButtonDown ? 2 : 0));
                                    }
                                }
                                else
                                {
                                    var actualCurrentState = _states.Dequeue();
                                    if (_lastReadState == null)
                                    {
                                        _lastReadState = actualCurrentState;
                                    }
                                    else
                                    {
                                        while (_states.Count > 0)
                                        {
                                            var peekItem = _states.Peek();

                                            if (peekItem.Wheel != actualCurrentState.Wheel)
                                                break;

                                            if (peekItem.LeftButtonDown != actualCurrentState.LeftButtonDown ||
                                                peekItem.RightButtonDown != actualCurrentState.RightButtonDown)
                                                break;

                                            if (Math.Abs(peekItem.X - _lastReadState.X) >= 50 ||
                                                Math.Abs(peekItem.Y - _lastReadState.Y) >= 50)
                                                break;

                                            actualCurrentState = _states.Dequeue();
                                        }

                                    }

                                    int xdiff = actualCurrentState.X - _lastReadState.X;
                                    int ydiff = actualCurrentState.Y - _lastReadState.Y;

                                    respData[0] = (byte) ((actualCurrentState.LeftButtonDown ? 1 : 0) +
                                                          (actualCurrentState.RightButtonDown ? 2 : 0));
                                    Array.Copy(BitConverter.GetBytes(xdiff), 0, respData, 1, 4);
                                    Array.Copy(BitConverter.GetBytes(ydiff), 0, respData, 5, 4);

                                    byte wheel = 0;
                                    if (actualCurrentState.Wheel > 0)
                                        wheel = 1;
                                    if (actualCurrentState.Wheel < 0)
                                        wheel = 0xff;
                                    respData[9] = wheel;

                                    _lastReadState = actualCurrentState;
                                }
                            }

                            WriteAll(networkStream, respData);

                            _lineLogger.LogLine("Mouse state sent to agent");
                        }
                        else if (IsOperation(opCode, "CLOS"))
                        {
                            if (!isInititalized)
                                break;

                            _lineLogger.LogLine("Close");
                            break;
                        }
                    }

                    networkStream.Close();
                    client.Close();
                }
            }
            catch (Exception ex)
            {
                _lineLogger.LogLine($"Exception: {ex}");
                _tcpListener.Stop();
            }
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
