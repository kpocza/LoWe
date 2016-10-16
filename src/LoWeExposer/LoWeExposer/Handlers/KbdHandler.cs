using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace LoWeExposer.Handlers
{
    class KbdHandler
    {
        private readonly int _port;
        private readonly ILineLogger _lineLogger;
        private BackgroundWorker _backgroundWorker;
        private CancellationTokenSource _cancellationTokenSource;
        private CancellationToken _cancellationToken;
        private readonly Queue<byte> _keyData;
        private readonly object _lockObj;
        private byte _lastItem;
        private TcpListener _tcpListener;

        public KbdHandler(int port, ILineLogger lineLogger)
        {
            _port = port;
            _lineLogger = lineLogger;
            _keyData = new Queue<byte>();
            _lockObj = new object();
            _lastItem = 0xff;
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

        public void KeyDownPerformed(byte scanCode)
        {
            lock (_lockObj)
            {
                EnqueueItem(scanCode);
            }
        }

        public void KeyUpPerformed(byte scanCode)
        {
            lock (_lockObj)
            {
                EnqueueItem((byte)(scanCode | 0x80));
            }
        }

        private void EnqueueItem(byte item)
        {
            if (_lastItem != item)
                _keyData.Enqueue(item);

            _lastItem = item;
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

                        if (IsOperation(opCode, "KEYB"))
                        {
                            WriteAll(networkStream, Encoding.ASCII.GetBytes("BYEK"));
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

                            var lenData = new byte[4];
                            if (!ReadAll(networkStream, lenData))
                                break;
                            int len = BitConverter.ToInt32(lenData, 0);

                            byte[] array;
                            lock (_lockObj)
                            {
                                var items = new List<byte>();

                                while (_keyData.Count > 0 && items.Count < len)
                                {
                                    items.Add(_keyData.Dequeue());
                                }
                                array = items.ToArray();
                                _keyData.Clear();
                            }

                            WriteAll(networkStream, BitConverter.GetBytes(array.Length));
                            if (array.Length > 0)
                            {
                                WriteAll(networkStream, array);
                                _lineLogger.LogLine($"Keyboard events sent to agent ({array.Length} items).");
                            }
                            else
                            {
                                _lineLogger.LogLine("No new keyboard events to be sent to the agent");
                            }
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
            catch(Exception ex)
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
