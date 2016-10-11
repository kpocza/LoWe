using System;
using System.ComponentModel;
using System.Windows;
using LoWeExposer.Handlers;

namespace LoWeExposer
{
    public partial class KbdExposer : Window, ILineLogger
    {
        private readonly FrameBufferExposer _frameBufferExposer;
        private KbdHandler _kbdHandler;
        private int _lineCount = 0;

        public KbdExposer(FrameBufferExposer frameBufferExposer)
        {
            _frameBufferExposer = frameBufferExposer;
            InitializeComponent();
        }

        private void KbdExposer_Loaded(object sender, EventArgs e)
        {
            if (_frameBufferExposer == null)
            {
                Close();
                return;
            }

            _frameBufferExposer.EnableKeyboardSupport(this);

            _kbdHandler = new KbdHandler(12347, this);
            _kbdHandler.Start();
        }

        private void KbdExposer_OnClosing(object sender, CancelEventArgs e)
        {
            _kbdHandler?.Stop();
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

        public void KeyDownPerformed(byte scanCode)
        {
            _kbdHandler.KeyDownPerformed(scanCode);

            ((ILineLogger)this).LogLine($"Keydown: {scanCode}");
        }

        public void KeyUpPerformed(byte scanCode)
        {
            _kbdHandler.KeyUpPerformed(scanCode);

            ((ILineLogger)this).LogLine($"Keyup: {scanCode}");
        }
    }
}
