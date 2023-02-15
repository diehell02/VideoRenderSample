using Render.Source;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace WPFSample
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            VideoGrid.Loaded += VideoGrid_Loaded;
            VideoView videoView = new();
            videoView.SetVideoSource(VideoSourceFactory.GetVideoSource());
            videoView.SetVideoState(true);
            //DispatcherTimer dispatcherTimer = new DispatcherTimer(TimeSpan.FromSeconds(5), DispatcherPriority.Send, async (s, e) =>
            //{
            //    //ClearVideos();
            //    //DisplayVideos();
            //    VideoGrid.Children.Clear();
            //    await Dispatcher.Yield(DispatcherPriority.Loaded);
            //    VideoGrid.Children.Add(videoView);
            //}, Dispatcher);
        }

        private void VideoGrid_Loaded(object sender, RoutedEventArgs e)
        {
            this.Activate();
            DisplayVideos();
        }

        private void DisplayVideos()
        {
            int count = Config.Instance.VideoViewNumber;
            int itemCount = 1;

            while (true)
            {
                if (count <= itemCount * itemCount)
                {
                    for (int i = 0; i < itemCount; i++)
                    {
                        VideoGrid.ColumnDefinitions.Add(new ColumnDefinition());
                    }
                    for (int i = 0; i < itemCount; i++)
                    {
                        if (count == 0)
                        {
                            break;
                        }
                        VideoGrid.RowDefinitions.Add(new RowDefinition());
                        for (int j = 0; j < itemCount; j++)
                        {
                            if (count == 0)
                            {
                                break;
                            }
                            VideoView videoView = new();
                            videoView.SetValue(Grid.RowProperty, i);
                            videoView.SetValue(Grid.ColumnProperty, j);
                            videoView.SetVideoSource(VideoSourceFactory.GetVideoSource());
                            videoView.SetVideoState(true);
                            VideoGrid.Children.Add(videoView);
                            count--;
                        }
                    }
                    break;
                }
                itemCount++;
            }
        }

        private void ClearVideos()
        {
            VideoGrid.Children.Clear();
            VideoGrid.ColumnDefinitions.Clear();
            VideoGrid.RowDefinitions.Clear();
        }
    }
}
