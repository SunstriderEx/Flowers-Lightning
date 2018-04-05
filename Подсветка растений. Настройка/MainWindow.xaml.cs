using System;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using HidLibrary;

namespace Подсветка_растений.Настройка
{
    public enum ReportID
    {
        EnableDeviceUSB,
        DeviceRtcTime,
        DeviceTimeToSleep,
        DeviceTimeToAwake,
        SetRtcTime,
        SetTimeToSleep,
        SetTimeToAwake
    }

    /// <summary>
    /// Логика взаимодействия для MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private static MainWindow instance;

        private const int VendorId = 0x029A;
        private const int ProductId = 0x0065;
        private static HidDevice device;

        private bool timeToSleepTextBoxChanged = false;
        private bool timeToAwakeTextBoxChanged = false;
        private Brush defaultBackground;

        public MainWindow()
        {
            InitializeComponent();
            instance = this;

            defaultBackground = timeToSleepTextBox.Background;
            sendSettingsButton.IsEnabled = false;
            syncTimeButton.IsEnabled = false;
            rtcTimeTextBox.IsEnabled = false;
            timeToSleepTextBox.IsEnabled = false;
            timeToAwakeTextBox.IsEnabled = false;
            WaitForConnection();
        }

        private static void OnReport(HidReport report)
        {
            if (!device.IsConnected)
                return;

            device.ReadReport(OnReport);

            var reportId = report.Data[0];

            var data = new byte[report.Data.Length - 1];
            for (int i = 0; i < report.Data.Length - 1; i++)
                data[i] = report.Data[i + 1];

            string time = DataReader.DataToTime(data);
            
            switch ((ReportID)reportId)
            {
                case ReportID.DeviceRtcTime:
                    instance.Dispatcher.BeginInvoke(
                        (Action)(() => instance.rtcTimeTextBox.Text = time));
                    break;

                case ReportID.DeviceTimeToSleep:
                    if (!instance.timeToSleepTextBoxChanged)
                        instance.Dispatcher.BeginInvoke(
                            (Action)(() => instance.timeToSleepTextBox.Text = time));
                    break;

                case ReportID.DeviceTimeToAwake:
                    if (!instance.timeToAwakeTextBoxChanged)
                        instance.Dispatcher.BeginInvoke(
                            (Action)(() => instance.timeToAwakeTextBox.Text = time));
                    break;
            }
        }

        private static void DeviceAttachedHandler()
        {
            device.ReadReport(OnReport);

            instance.Dispatcher.BeginInvoke((Action)(() => instance.statusBarLabel.Content = "Устройство подключено"));
            instance.Dispatcher.BeginInvoke((Action)(() => instance.sendSettingsButton.IsEnabled = true));
            instance.Dispatcher.BeginInvoke((Action)(() => instance.syncTimeButton.IsEnabled = true));
            instance.Dispatcher.BeginInvoke((Action)(() => instance.rtcTimeTextBox.IsEnabled = true));
            instance.Dispatcher.BeginInvoke((Action)(() => instance.timeToSleepTextBox.IsEnabled = true));
            instance.Dispatcher.BeginInvoke((Action)(() => instance.timeToAwakeTextBox.IsEnabled = true));
        }

        private static void DeviceRemovedHandler()
        {
            instance.Dispatcher.BeginInvoke((Action)(() => instance.statusBarLabel.Content = "Устройство отключено"));
            instance.Dispatcher.BeginInvoke((Action)(() => instance.sendSettingsButton.IsEnabled = false));
            instance.Dispatcher.BeginInvoke((Action)(() => instance.syncTimeButton.IsEnabled = false));
            instance.Dispatcher.BeginInvoke((Action)(() => instance.rtcTimeTextBox.IsEnabled = false));
            instance.Dispatcher.BeginInvoke((Action)(() => instance.timeToSleepTextBox.IsEnabled = false));
            instance.Dispatcher.BeginInvoke((Action)(() => instance.timeToAwakeTextBox.IsEnabled = false));
        }

        private async void WaitForConnection()
        {
            await WaitForConnectionTask();
        }

        private async Task WaitForConnectionTask()
        {
            while (device == null)
            {
                device = HidDevices.Enumerate(VendorId, ProductId).FirstOrDefault();

                if (device != null)
                {
                    device.OpenDevice();

                    device.Inserted += DeviceAttachedHandler;
                    device.Removed += DeviceRemovedHandler;

                    device.MonitorDeviceEvents = true;

                    device.ReadReport(OnReport);

                    statusBarLabel.Content = "Устройство обнаружено";

                    device.CloseDevice();
                }
                else
                {
                    statusBarLabel.Content = "Ожидание подключения устройства";
                    await Task.Delay(250);
                }
            }
            
        }

        private void syncTimeButton_Click(object sender, RoutedEventArgs e)
        {
            syncTimeButton.IsEnabled = false;
            device.WriteFeatureData(DataReader.TimeToOutputData(DateTime.Now, ReportID.SetRtcTime));
            statusBarLabel.Content = string.Format("[{0}] Время синхронизировано", DateTime.Now.ToLongTimeString());
            syncTimeButton.IsEnabled = true;
        }

        private void sendSettingsButton_Click(object sender, RoutedEventArgs e)
        {
            sendSettingsButton.IsEnabled = false;

            var timeToSleepStrs = timeToSleepTextBox.Text.Split(':');
            var timeToAwakeStrs = timeToAwakeTextBox.Text.Split(':');
            if (timeToSleepStrs.Length != 3 || timeToAwakeStrs.Length != 3)
            {
                MessageBox.Show("Время должно иметь формат ЧЧ:ММ:СС", "Ошибка", MessageBoxButton.OK, MessageBoxImage.Error);
                sendSettingsButton.IsEnabled = true;
                return;
            }

            DateTime timeToSleep, timeToAwake;

            try
            {
                timeToSleep = new DateTime(2017, 09, 14, int.Parse(timeToSleepStrs[0]), int.Parse(timeToSleepStrs[1]), int.Parse(timeToSleepStrs[2]));
                timeToAwake = new DateTime(2017, 09, 14, int.Parse(timeToAwakeStrs[0]), int.Parse(timeToAwakeStrs[1]), int.Parse(timeToAwakeStrs[2]));
            }
            catch (Exception)
            {
                MessageBox.Show("Неверное значение часов, минут или секунд", "Ошибка", MessageBoxButton.OK, MessageBoxImage.Error);
                sendSettingsButton.IsEnabled = true;
                return;
            }
            
            device.WriteFeatureData(DataReader.TimeToOutputData(timeToSleep, ReportID.SetTimeToSleep));
            device.WriteFeatureData(DataReader.TimeToOutputData(timeToAwake, ReportID.SetTimeToAwake));

            timeToSleepTextBoxChanged = false;
            timeToAwakeTextBoxChanged = false;
            timeToSleepTextBox.Background = defaultBackground;
            timeToAwakeTextBox.Background = defaultBackground;
            sendSettingsButton.IsEnabled = true;
            statusBarLabel.Content = string.Format("[{0}] Настройки сохранены", DateTime.Now.ToLongTimeString());
            System.Media.SystemSounds.Exclamation.Play();
        }

        private void timeToSleepTextBox_PreviewTextInput(object sender, TextCompositionEventArgs e)
        {
            timeToSleepTextBoxChanged = true;
            timeToSleepTextBox.Background = Brushes.Yellow;
        }

        private void timeToAwakeTextBox_PreviewTextInput(object sender, TextCompositionEventArgs e)
        {
            timeToAwakeTextBoxChanged = true;
            timeToAwakeTextBox.Background = Brushes.Yellow;
        }
    }
}
