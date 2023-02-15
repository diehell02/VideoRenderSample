// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Render.Source;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace UnoUWP
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
            VideoGrid.Loaded += VideoGrid_Loaded;
        }

        private void VideoGrid_Loaded(object sender, RoutedEventArgs e)
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
