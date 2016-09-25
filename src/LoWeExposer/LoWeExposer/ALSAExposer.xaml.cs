using System;
using System.ComponentModel;
using System.Windows;
using LoWeExposer.Handlers;

namespace LoWeExposer
{
    public partial class AlsaExposer : Window, ILineLogger
    {
        private AlsaHandler _alsaHandler;
        private int _lineCount = 0;

        public AlsaExposer()
        {
            InitializeComponent();
        }

        private void AlsaExposer_Loaded(object sender, EventArgs e)
        {
            _alsaHandler = new AlsaHandler(12345, this);
            _alsaHandler.Start();
        }

        private void AlsaExposer_OnClosing(object sender, CancelEventArgs e)
        {
            _alsaHandler.Stop();
        }

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
