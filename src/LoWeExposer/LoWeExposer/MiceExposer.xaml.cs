using System;
using System.ComponentModel;
using System.Windows;
using LoWeExposer.Handlers;

namespace LoWeExposer
{
    public partial class MiceExposer : Window, ILineLogger
    {
        private readonly FrameBufferExposer _frameBufferExposer;
        private MiceHandler _miceHandler;
        private int _lineCount = 0;

        public MiceExposer(FrameBufferExposer frameBufferExposer)
        {
            _frameBufferExposer = frameBufferExposer;
            InitializeComponent();
        }

        private void MiceExposer_Loaded(object sender, EventArgs e)
        {
            if (_frameBufferExposer == null)
            {
                Close();
                return;
            }

            _frameBufferExposer.EnableMouseSupport(this);

            _miceHandler = new MiceHandler(12346, this);
            _miceHandler.Start();
        }

        private void MiceExposer_OnClosing(object sender, CancelEventArgs e)
        {
            _miceHandler?.Stop();
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

        internal void AddState(MiceState miceState)
        {
            _miceHandler.SetState(miceState);

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
