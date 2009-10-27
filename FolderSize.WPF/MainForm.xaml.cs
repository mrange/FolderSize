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
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Animation;


namespace FolderSize.WPF
{
   partial class MainForm
   {

      // ======================================================================
      // DllImports
      // ======================================================================
      // ReSharper disable InconsistentNaming
      // ----------------------------------------------------------------------

      [StructLayout(LayoutKind.Sequential)]
      struct MARGINS
      {
         public int cxLeftWidth;      // width of left border that retains its size
         public int cxRightWidth;     // width of right border that retains its size
         public int cyTopHeight;      // height of top border that retains its size
         public int cyBottomHeight;   // height of bottom border that retains its size
      };

      [DllImport("DwmApi.dll")]
      static extern int DwmExtendFrameIntoClientArea(
          IntPtr hwnd,
          ref MARGINS pMarInset);

      // ----------------------------------------------------------------------

      public enum ShowCommands
      {
         SW_HIDE = 0,
         SW_SHOWNORMAL = 1,
         SW_NORMAL = 1,
         SW_SHOWMINIMIZED = 2,
         SW_SHOWMAXIMIZED = 3,
         SW_MAXIMIZE = 3,
         SW_SHOWNOACTIVATE = 4,
         SW_SHOW = 5,
         SW_MINIMIZE = 6,
         SW_SHOWMINNOACTIVE = 7,
         SW_SHOWNA = 8,
         SW_RESTORE = 9,
         SW_SHOWDEFAULT = 10,
         SW_FORCEMINIMIZE = 11,
         SW_MAX = 11
      }

      [DllImport("shell32.dll")]
      static extern IntPtr ShellExecute(
          IntPtr hwnd,
          string lpOperation,
          string lpFile,
          string lpParameters,
          string lpDirectory,
          ShowCommands nShowCmd);

      // ----------------------------------------------------------------------
      // ReSharper restore InconsistentNaming
      // ----------------------------------------------------------------------

      enum PresentState
      {
         InitialWait,
         PresentingInitialInfo,
         InitialInfoPresented,
         HidingInitialInfo,
         HintInfoHidden,
         PresentingHintInfo,
         HintInfoPresented,
         HidingHintInfo,
      }

      // ----------------------------------------------------------------------

      readonly StoryboardHolder m_presentStoryboard;
      readonly StoryboardHolder m_hideStoryboard;
      readonly StoryboardHolder m_idlingStoryboard;

      // ----------------------------------------------------------------------

      PresentState m_presentState = PresentState.InitialWait;

      // ----------------------------------------------------------------------

      PresentState State
      {
         get
         {
            return m_presentState;
         }
         set
         {
            Debug.WriteLine (
               string.Format (
                  CultureInfo.InvariantCulture,
                  "State change: {0} -> {1}",
                  m_presentState,
                  value));

            m_presentState = value;
         }
      }

      // ----------------------------------------------------------------------

      public MainForm()
      {
         InitializeComponent();

         m_presentStoryboard = new StoryboardHolder(this, "PresentInfoBlockStoryboard");
         m_hideStoryboard = new StoryboardHolder(this, "HideInfoBlockStoryboard");
         m_idlingStoryboard = new StoryboardHolder(this, "IdlingStoryboard");

         var mouseMoveEvent = MouseMoveEvent;
         MouseEventHandler windowMouseMove = Window_MouseMove;
         AddHandler (
            mouseMoveEvent,
            windowMouseMove,
            true);

         var keydownEvent = KeyDownEvent;
         KeyEventHandler windowKeyDown = Window_KeyDown;
         AddHandler(
            keydownEvent,
            windowKeyDown,
            true);

         FolderTreeView.FolderDoubleClick += FolderTreeView_FolderDoubleClick;
      }

      // ======================================================================
      // Storyboard handlers
      // ======================================================================

      void StopStoryboards()
      {
         m_idlingStoryboard.Stop();
         m_presentStoryboard.Stop();
         m_hideStoryboard.Stop();
      }

      // ----------------------------------------------------------------------

      void RunHideStoryboard()
      {
         StopStoryboards ();
         m_hideStoryboard.Restart();
      }

      // ----------------------------------------------------------------------

      void RunPresentStoryboard()
      {
         StopStoryboards();
         m_presentStoryboard.Restart();
      }

      // ----------------------------------------------------------------------

      void RunIdlingStoryboard()
      {
         StopStoryboards();
         m_idlingStoryboard.Restart();
      }

      // ----------------------------------------------------------------------

      void UserInputDetected()
      {
         switch (State)
         {
            case PresentState.InitialWait:
            case PresentState.PresentingInitialInfo:
            case PresentState.InitialInfoPresented:
            case PresentState.HidingInitialInfo:
               break;
            case PresentState.HintInfoHidden:
               RunIdlingStoryboard();
               break;
            case PresentState.PresentingHintInfo:
               break;
            case PresentState.HintInfoPresented:
               RunHideStoryboard();
               State = PresentState.HidingHintInfo;
               break;
            case PresentState.HidingHintInfo:
               break;
            default:
               throw new ArgumentOutOfRangeException();
         }
      }

      // ======================================================================
      // Event handlers
      // ======================================================================

      void Window_KeyDown(object sender,
                          KeyEventArgs e)
      {
         UserInputDetected();
      }

      // ----------------------------------------------------------------------

      void Window_MouseMove(object sender, MouseEventArgs e)
      {
         UserInputDetected ();
      }

      // ----------------------------------------------------------------------

      public void Window_Closed(object sender, EventArgs value)
      {
         var job = FolderTreeView.Job;
         if (job != null)
         {
            job.StopJob = true;
         }
      }

      // ----------------------------------------------------------------------

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
               (Brush)Application.Current.Resources["WindowGradient"];
         }

      }

      // ----------------------------------------------------------------------

      static void FolderTreeView_FolderDoubleClick(object sender, FolderEventArgs e)
      {
         var path = Path.GetFullPath(e.Folder.Path);
         if (Directory.Exists(path))
         {
            ShellExecute(
               IntPtr.Zero,
               "explore",
               path,
               null,
               null,
               ShowCommands.SW_NORMAL);
         }
      }

      // ----------------------------------------------------------------------

      void GoButton_Click(object sender, RoutedEventArgs e)
      {
         switch (State)
         {
            case PresentState.InitialWait:
               State = PresentState.HintInfoHidden;
               break;
            case PresentState.PresentingInitialInfo:
               break;
            case PresentState.InitialInfoPresented:
               RunHideStoryboard ();
               State = PresentState.HidingInitialInfo;
               break;
            case PresentState.HidingInitialInfo:
            case PresentState.HintInfoHidden:
            case PresentState.PresentingHintInfo:
            case PresentState.HintInfoPresented:
            case PresentState.HidingHintInfo:
               break;
            default:
               throw new ArgumentOutOfRangeException ();
         }
      }

      // ----------------------------------------------------------------------

      void Storyboard_Completed(object sender, EventArgs e)
      {
         var clockGroup = (ClockGroup)sender;
         Debug.WriteLine(
            string.Format(
               CultureInfo.InvariantCulture,
               "Storyboard_Completed : {0}",
               clockGroup.Timeline.Name ?? "NULL"));
         switch (State)
         {
            case PresentState.InitialWait:
               RunPresentStoryboard ();
               State = PresentState.PresentingInitialInfo;
               break;
            case PresentState.PresentingInitialInfo:
               if (FolderTreeView.Job != null)
               {
                  State = PresentState.HidingInitialInfo;
                  RunHideStoryboard();
               }
               else
               {
                  State = PresentState.InitialInfoPresented;
               }
               break;
            case PresentState.InitialInfoPresented:
               Debug.Assert (false);
               break;
            case PresentState.HidingInitialInfo:
               State = PresentState.HintInfoHidden;
               RunIdlingStoryboard();
               break;
            case PresentState.HintInfoHidden:
               State = PresentState.PresentingHintInfo;
               InitialInfoText.Visibility = Visibility.Collapsed;
               HintInfoText.Visibility = Visibility.Visible;
               RunPresentStoryboard();
               break;
            case PresentState.PresentingHintInfo:
               State = PresentState.HintInfoPresented;
               break;
            case PresentState.HintInfoPresented:
               break;
            case PresentState.HidingHintInfo:
               State = PresentState.HintInfoHidden;
               RunIdlingStoryboard();
               break;
            default:
               throw new ArgumentOutOfRangeException ();
         }
      }

      // ----------------------------------------------------------------------

   }
}