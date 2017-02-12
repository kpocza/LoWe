using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

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

        public int QueueLength
        {
            get
            {
                lock (_lockObj)
                {
                    return _keyData.Count;
                }
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
                    _socket = _tcpListener.AcceptSocket();
                    _socket.NoDelay = true;

                    bool isInititalized = false;
                    while (!_cancellationToken.IsCancellationRequested && !_tcpListener.Pending() && _socket.Connected)
                    {
                        var opCode = new byte[4];
                        if (!ReadAllImpatient(opCode))
                            continue;

                        if (IsOperation(opCode, "KEYB"))
                        {
                            WriteAll(Encoding.ASCII.GetBytes("BYEK"));
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
                            if (!ReadAllPatient(lenData))
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

                            WriteAll(BitConverter.GetBytes(array.Length));
                            if (array.Length > 0)
                            {
                                WriteAll(array);
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
                    _socket.Close();
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
