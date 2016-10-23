using System.Windows;

namespace LoWeExposer
{
    public class MainWindowViewModel : DependencyObject
    {
        public bool FramebufferEnabled
        {
            get { return (bool)GetValue(FramebufferEnabledProperty); }
            set { SetValue(FramebufferEnabledProperty, value); }
        }

        public bool KeyboardEnabled
        {
            get { return (bool)GetValue(KeyboardEnabledProperty); }
            set { SetValue(KeyboardEnabledProperty, value); }
        }
        public bool MouseEnabled
        {
            get { return (bool)GetValue(MouseEnabledProperty); }
            set { SetValue(MouseEnabledProperty, value); }
        }

        public bool AlsaEnabled
        {
            get { return (bool)GetValue(AlsaEnabledProperty); }
            set { SetValue(AlsaEnabledProperty, value); }
        }

        public static readonly DependencyProperty FramebufferEnabledProperty =
            DependencyProperty.Register("FramebufferEnabled", typeof(bool), typeof(MainWindowViewModel), new PropertyMetadata(false));
        public static readonly DependencyProperty KeyboardEnabledProperty =
            DependencyProperty.Register("KeyboardEnabled", typeof(bool), typeof(MainWindowViewModel), new PropertyMetadata(false));
        public static readonly DependencyProperty MouseEnabledProperty =
            DependencyProperty.Register("MouseEnabled", typeof(bool), typeof(MainWindowViewModel), new PropertyMetadata(false));
        public static readonly DependencyProperty AlsaEnabledProperty =
            DependencyProperty.Register("AlsaEnabled", typeof(bool), typeof(MainWindowViewModel), new PropertyMetadata(false));
    }
}
