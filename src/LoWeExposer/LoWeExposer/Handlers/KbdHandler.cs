using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;
using System.Threading;

namespace LoWeExposer.Handlers
{
    class KbdHandler : HandlerBase
    {
        private readonly ILineLogger _lineLogger;
        private readonly Queue<byte> _keyData;
        private readonly object _lockObj;
        private byte _lastItem;

        public KbdHandler(ILineLogger lineLogger)
        {
            _lineLogger = lineLogger;
            _keyData = new Queue<byte>();
            _lockObj = new object();
            _lastItem = 0xff;
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

        protected override void backgroundWorker_DoWork(object sender, DoWorkEventArgs e)
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
                            Thread.Sleep(10);
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
    }
}
