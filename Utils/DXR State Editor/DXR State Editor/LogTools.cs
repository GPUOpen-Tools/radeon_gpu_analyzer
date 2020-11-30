using System;
using System.Collections.Generic;
using System.Windows;

namespace DXR_State_Editor
{
    public partial class MainWindow : Window
    {
        // Todo: write to file on exit?
        public List<String> warningMessageLog = new List<String>();
        public List<String> errorMessageLog = new List<String>();

        public void postWarningMessage(String msgDetail)
        {
            String msg = "[ warning ] (" + System.DateTime.Now + "): " + msgDetail;
            warningMessageLog.Add(msg);
            Console.WriteLine(msg);
        }

        public void postErrorMessage(String msgDetail)
        {
            String msg = "[ error ]  (" + System.DateTime.Now + "): " + msgDetail;
            errorMessageLog.Add(msg);
            Console.Error.WriteLine(msg);
        }
    }
}
