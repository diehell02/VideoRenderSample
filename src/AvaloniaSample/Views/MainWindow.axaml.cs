// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Avalonia.Controls;
using Render.Source;

namespace AvaloniaSample.Views
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            this.Opened += MainWindow_Opened;
            InitializeComponent();
        }

        private void MainWindow_Opened(object? sender, System.EventArgs e)
        {
            this.Activate();

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
                            VideoView2 videoView = new();
                            videoView.SetValue(Grid.RowProperty, i);
                            videoView.SetValue(Grid.ColumnProperty, j);
                            VideoGrid.Children.Add(videoView);
                            count--;
                        }
                    }
                    break;
                }
                itemCount++;
            }
        }
    }
}
