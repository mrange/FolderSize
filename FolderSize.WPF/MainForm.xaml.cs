
/* ****************************************************************************
 *
 * Copyright (c) Mårten Rånge.
 *
 * This source code is subject to terms and conditions of the Microsoft Public License. A 
 * copy of the license can be found in the License.html file at the root of this distribution. If 
 * you cannot locate the  Microsoft Public License, please send an email to 
 * dlr@microsoft.com. By using this source code in any fashion, you are agreeing to be bound 
 * by the terms of the Microsoft Public License.
 *
 * You must not remove this notice, or any other, from this software.
 *
 *
 * ***************************************************************************/

using System;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;
using System.Windows.Media;
using System.Threading;
using System.Windows.Threading;
using System.ComponentModel;
using FolderSize.Common;


namespace FolderSize.WPF
{
    public partial class MainForm
    {
       [StructLayout(LayoutKind.Sequential)]
       public struct MARGINS
       {
          public int cxLeftWidth;      // width of left border that retains its size
          public int cxRightWidth;     // width of right border that retains its size
          public int cyTopHeight;      // height of top border that retains its size
          public int cyBottomHeight;   // height of bottom border that retains its size
       };


       [DllImport("DwmApi.dll")]
       public static extern int DwmExtendFrameIntoClientArea(
           IntPtr hwnd,
           ref MARGINS pMarInset);

       JobId? m_jobId;

       readonly BackgroundWorker m_worker;

        public MainForm()
        {
            InitializeComponent();

            m_worker = new BackgroundWorker();

            m_worker.DoWork += DoWork;
            m_worker.WorkerSupportsCancellation = true;

            m_worker.RunWorkerAsync();
        }

        void DoWork(object sender, DoWorkEventArgs e)
        {
            EventHandler onUpdate = OnUpdate;

            while (!m_worker.CancellationPending)
            {
                Thread.Sleep(500);

                OnUpdateDispatch(
                   onUpdate,
                   this,
                   new EventArgs());
            }

            e.Cancel = true;
        }


        void OnClosing(
           object sender,
           EventArgs e)
        {
            m_worker.CancelAsync();
        }

        void OnUpdate(
           object sender,
           EventArgs e)
        {
            var job = FolderTreeView.Job;
            if (job != null)
            {

                Title = string.Format(
                   "FolderSize.WPF [{0}, {1}]",
                   (job.IsRunning ? "Running" : "Finished"),
                   job.Jobs - job.FinishedJobs);

                var newJob = JobId.Create(
                   job.Id,
                   job.Jobs);

                if (
                   !(
                      m_jobId.HasValue
                      && m_jobId.Equals(newJob)))
                {
                    FolderTreeView.Refresh();
                }

                m_jobId = newJob;
            }
            else
            {
                m_jobId = null;
            }
        }

        void OnUpdateDispatch(EventHandler eh, object sender, EventArgs e)
        {
            Dispatcher.Invoke(
               DispatcherPriority.Normal,
               eh,
               sender,
               e);
        }

        public void OnClickGo(object sender, RoutedEventArgs value)
        {
            StopJob();

            try
            {
                var job = FolderTraverser.StartTraverse(PathTextBox.Text);
                FolderTreeView.Job = job;
            }
            catch (Exception exc)
            {
                MessageBox.Show(exc.Message);
            }
        }

        public void OnClickStop(object sender, RoutedEventArgs value)
        {
            StopJob();
        }

        public void OnUnloadedForm(object sender, EventArgs value)
        {
            try
            {
                StopJob();
            }
            catch
            {

            }
        }

        void StopJob()
        {
            if (FolderTreeView.Job != null)
            {
                FolderTreeView.Job.StopJob = true;
                FolderTreeView.Job = null;
            }
        }

        void Window_Loaded(object sender, RoutedEventArgs e)
        {
           try
           {
              // Obtain the window handle for WPF application
              var mainWindowPtr = new WindowInteropHelper(this).Handle;
              var mainWindowSrc = HwndSource.FromHwnd(mainWindowPtr);
              mainWindowSrc.CompositionTarget.BackgroundColor = Color.FromArgb(
                 0,
                 0,
                 0,
                 0);

              var margins =
                 new MARGINS
                 {
                    cxLeftWidth = -1,
                    cxRightWidth = -1,
                    cyTopHeight = -1,
                    cyBottomHeight = -1
                 };

              var hresult =
                 DwmExtendFrameIntoClientArea(
                    mainWindowSrc.Handle,
                    ref margins);
              //
              if (hresult < 0)
              {
                 throw new Exception(
                    string.Format(
                       "DwmExtendFrameIntoClientArea failed: {0}", hresult)
                       );
              }
           }
           catch
           {
              // Fallback brush
              Application.Current.MainWindow.Background =
                 (Brush) Application.Current.Resources["WindowGradient"];
           }

        }
    }
}