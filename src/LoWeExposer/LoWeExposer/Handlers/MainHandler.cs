using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Configuration;
using System.Text;

namespace LoWeExposer.Handlers
{
    class MainHandler : HandlerBase
    {
        private readonly IInitializer _initializer;

        public MainHandler(IInitializer initializer) : base(int.Parse(ConfigurationManager.AppSettings["port"]))
        {
            _initializer = initializer;
        }

        protected override void backgroundWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            while (!_cancellationToken.IsCancellationRequested)
            {
                _socket = _tcpListener.AcceptSocket();
                _socket.NoDelay = true;

                while (!_cancellationToken.IsCancellationRequested)
                {
                    var opCode = new byte[4];
                    if (!ReadAllImpatient(opCode))
                        continue;

                    if (IsOperation(opCode, "LOWE"))
                    {
                        var countData = new byte[4];
                        if (!ReadAllPatient(countData))
                            break;

                        int count = BitConverter.ToInt32(countData, 0);

                        var handlerData = new byte[4*count];
                        if (!ReadAllPatient(handlerData))
                            break;

                        var handlers = new HashSet<string>();
                        for (int i = 0; i < count; i++)
                        {
                            handlers.Add(Encoding.ASCII.GetString(handlerData, i*4, 4));
                        }

                        if (handlers.Count != count)
                            break;

                        var items = _initializer.GetResult(handlers);

                        var responseData = new byte[count*4*2];
                        int idx = 0;
                        foreach (var item in items)
                        {
                            Array.Copy(Encoding.ASCII.GetBytes(item.Key), 0, responseData, idx*8, 4);
                            Array.Copy(BitConverter.GetBytes(item.Value), 0, responseData, idx*8 + 4, 4);
                            idx++;
                        }

                        WriteAll(Encoding.ASCII.GetBytes("EWOL"));
                        WriteAll(responseData);
                        break;
                    }
                }

                _socket.Close();
            }
        }
    }
}
