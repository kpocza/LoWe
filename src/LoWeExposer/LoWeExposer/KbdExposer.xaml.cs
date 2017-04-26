using System;
using System.Windows.Controls;
using System.Windows.Input;
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

        public void KeyDownPerformed(KeyEventArgs e)
        {
            _kbdHandler.KeyDownPerformed(e);
            
            ((ILineLogger)this).LogLine($"Keydown: {(e.Key!= Key.System ? e.Key : e.SystemKey)}");
        }

        public void KeyUpPerformed(KeyEventArgs e)
        {
            _kbdHandler.KeyUpPerformed(e);

            ((ILineLogger)this).LogLine($"Keyup: {(e.Key != Key.System ? e.Key : e.SystemKey)}");
        }

        public int QueueLength => _kbdHandler.QueueLength;
    }
}
