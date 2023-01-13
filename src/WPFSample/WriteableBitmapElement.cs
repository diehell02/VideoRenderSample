using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace WPFSample
{
    internal class WriteableBitmapElement : VideoElement
    {
        public WriteableBitmapElement()
        {
            _render = new WriteableBitmapSource();
        }
    }
}
