using System;
using System.Windows.Controls;
using LoWeExposer.Handlers;

namespace LoWeExposer
{
    public partial class AlsaExposer : UserControl, ILineLogger
    {
        private AlsaHandler _alsaHandler;
        private int _lineCount = 0;

        public AlsaExposer()
        {
            InitializeComponent();
        }

        public void Start()
        {
            _alsaHandler = new AlsaHandler(this);
            _alsaHandler.Start();
        }

        public void Stop()
        {
            _alsaHandler.Stop();
        }

        public int Port => _alsaHandler.Port;

        private void AddLine(string text)
        {
            if (_lineCount > 100)
            {
                textBox.Text = "";
                _lineCount = 0;
            }
            textBox.Text += text + Environment.NewLine;

            if (_lineCount%5 == 0)
            {
                textBox.ScrollToEnd();
            }
            _lineCount++;
        }

        void ILineLogger.LogLine(string line)
        {
            Dispatcher.BeginInvoke((Action)(() => AddLine(line)));
        }
    }
}
