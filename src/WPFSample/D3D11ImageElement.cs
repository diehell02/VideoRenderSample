using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace WPFSample
{
    internal class D3D11ImageElement : VideoElement
    {
        public D3D11ImageElement()
        {
            _render = new D3D11ImageSource();
        }
    }
}
