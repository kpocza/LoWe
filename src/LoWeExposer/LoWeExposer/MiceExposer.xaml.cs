using System;
using System.ComponentModel;
using System.Windows.Controls;
using LoWeExposer.Handlers;

namespace LoWeExposer
{
    public partial class MiceExposer : UserControl, ILineLogger
    {
        private FrameBufferExposer _frameBufferExposer;
        private MiceHandler _miceHandler;
        private int _lineCount = 0;

        public MiceExposer()
        {
            InitializeComponent();
        }

        public void Start(FrameBufferExposer frameBufferExposer)
        {
            _frameBufferExposer = frameBufferExposer;
            _frameBufferExposer.EnableMouseSupport(this);

            _miceHandler = new MiceHandler(this);
            _miceHandler.Start();
        }

        public void Stop()
        {
            _miceHandler?.Stop();
        }

        public int Port => _miceHandler.Port;

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

        internal void AddState(MiceState miceState)
        {
            _miceHandler.AddState(miceState);

            ((ILineLogger)this).LogLine($"X: {miceState.X}, Y: {miceState.Y}, L: {miceState.LeftButtonDown}, R: {miceState.RightButtonDown}");
        }

        internal void ClearQueue()
        {
            _miceHandler.ClearQueue();
        }

        void ILineLogger.LogLine(string line)
        {
            Dispatcher.BeginInvoke((Action)(() => AddLine(line)));
        }
    }
}
