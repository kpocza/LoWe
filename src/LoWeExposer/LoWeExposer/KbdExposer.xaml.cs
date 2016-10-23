using System;
using System.ComponentModel;
using System.Windows.Controls;
using LoWeExposer.Handlers;

namespace LoWeExposer
{
    public partial class KbdExposer : UserControl, ILineLogger
    {
        private FrameBufferExposer _frameBufferExposer;
        private KbdHandler _kbdHandler;
        private int _lineCount = 0;

        public KbdExposer()
        {
            InitializeComponent();
        }

        public void Start(FrameBufferExposer frameBufferExposer)
        {
            _frameBufferExposer = frameBufferExposer;
            _frameBufferExposer.EnableKeyboardSupport(this);

            _kbdHandler = new KbdHandler(this);
            _kbdHandler.Start();
        }

        public void Stop()
        {
            _kbdHandler?.Stop();
        }
        public int Port => _kbdHandler.Port;

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
