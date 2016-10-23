using System.Collections.Generic;
using System.Linq;
using System.Windows;
using LoWeExposer.Handlers;

namespace LoWeExposer
{
    public partial class MainWindow : Window, IInitializer
    {
        private FrameBufferExposer _frameBufferExposer;
        private readonly MainWindowViewModel _viewModel;
        private readonly MainHandler _mainHandler;

        public MainWindow()
        {
            InitializeComponent();

            _viewModel = new MainWindowViewModel();
            DataContext = _viewModel;

            _mainHandler = new MainHandler(this);
        }

        private void MainWindow_OnLoaded(object sender, RoutedEventArgs e)
        {
            _mainHandler.Start();
        }
        private void MainWindow_OnClosed(object sender, System.EventArgs e)
        {
            _mainHandler.Stop();
        }

        private void btnFramebuffer_OnChecked(object sender, RoutedEventArgs e)
        {
            _frameBufferExposer = new FrameBufferExposer();
            _frameBufferExposer.Show();
            _frameBufferExposer.Closed += _frameBufferExposer_Closed;
        }

        private void btnFramebuffer_OnUnchecked(object sender, RoutedEventArgs e)
        {
            _frameBufferExposer?.Close();
        }

        private void _frameBufferExposer_Closed(object sender, System.EventArgs e)
        {
            _frameBufferExposer = null;

            _viewModel.KeyboardEnabled = false;
            _viewModel.MouseEnabled = false;
            _viewModel.FramebufferEnabled = false;
        }

        private void btnKeyboard_OnChecked(object sender, RoutedEventArgs e)
        {
            kbdExposer.Start(_frameBufferExposer);
        }

        private void btnKeyboard_OnUnchecked(object sender, RoutedEventArgs e)
        {
            kbdExposer.Stop();
        }

        private void btnMouse_OnChecked(object sender, RoutedEventArgs e)
        {
            miceExposer.Start(_frameBufferExposer);
        }

        private void btnMouse_OnUnchecked(object sender, RoutedEventArgs e)
        {
            miceExposer.Stop();
        }

        private void btnAlsa_OnChecked(object sender, RoutedEventArgs e)
        {
            alsaExposer.Start();
        }

        private void btnAlsa_OnUnchecked(object sender, RoutedEventArgs e)
        {
            alsaExposer.Stop();
        }

        public IDictionary<string, int> GetResult(ICollection<string> names)
        {
            var result = new Dictionary<string, int>();
            Dispatcher.Invoke(() =>
            {
                if (names.Contains("FBUF"))
                    _viewModel.FramebufferEnabled = true;
                if (names.Contains("MICE"))
                    _viewModel.MouseEnabled = true;
                if (names.Contains("KEYB"))
                    _viewModel.KeyboardEnabled = true;
                if (names.Contains("ALSA"))
                    _viewModel.AlsaEnabled = true;

                if (names.Contains("FBUF"))
                    result.Add("FBUF", _viewModel.FramebufferEnabled == true ? 1 : 0);
                if (names.Contains("MICE"))
                    result.Add("MICE", miceExposer.Port);
                if (names.Contains("KEYB"))
                    result.Add("KEYB", kbdExposer.Port);
                if (names.Contains("ALSA"))
                    result.Add("ALSA", alsaExposer.Port);
            });
            return result;
        }
    }
}
