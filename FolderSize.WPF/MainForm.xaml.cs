
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

      readonly Storyboard m_presentStoryboard;
      readonly Storyboard m_hideStoryboard;
      readonly Storyboard m_idlingStoryboard;

      // ----------------------------------------------------------------------

      PresentState m_presentState = PresentState.InitialWait;

      // ----------------------------------------------------------------------

      [StructLayout(LayoutKind.Sequential)]
      struct MARGINS
      {
         public int cxLeftWidth;      // width of left border that retains its size
         public int cxRightWidth;     // width of right border that retains its size
         public int cyTopHeight;      // height of top border that retains its size
         public int cyBottomHeight;   // height of bottom border that retains its size
      };

      // ----------------------------------------------------------------------

      [DllImport("DwmApi.dll")]
      static extern int DwmExtendFrameIntoClientArea(
          IntPtr hwnd,
          ref MARGINS pMarInset);

      // ----------------------------------------------------------------------

      public MainForm()
      {
         InitializeComponent();
         m_presentStoryboard = (Storyboard) Resources["PresentInfoBlockStoryboard"];
         m_hideStoryboard = (Storyboard)Resources["HideInfoBlockStoryboard"];
         m_idlingStoryboard = (Storyboard) Resources["IdlingStoryboard"];

         var mouseMoveEvent = MouseMoveEvent;
         MouseEventHandler windowMouseMove = WindowMouseMove;
         AddHandler (
            mouseMoveEvent,
            windowMouseMove,
            true);

         var keydownEvent = KeyDownEvent;
         KeyEventHandler windowKeyDown = WindowKeyDown;
         AddHandler(
            keydownEvent,
            windowKeyDown,
            true);

      }

      // ----------------------------------------------------------------------

      void WindowKeyDown(object sender,
                          KeyEventArgs e)
      {
         UserInputDetected();
      }

      // ----------------------------------------------------------------------

      void WindowMouseMove(object sender, MouseEventArgs e)
      {
         UserInputDetected ();
      }

      // ----------------------------------------------------------------------

      public void WindowClosed(object sender, EventArgs value)
      {
         var job = FolderTreeView.Job;
         if (job != null)
         {
            job.StopJob = true;
         }
      }

      // ----------------------------------------------------------------------

      void WindowLoaded(object sender, RoutedEventArgs e)
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

      void GoButtonClick(object sender, RoutedEventArgs e)
      {
         switch(m_presentState)
         {
            case PresentState.InitialWait:
            case PresentState.PresentingInitialInfo:
               break;
            case PresentState.InitialInfoPresented:
               RunHideStoryboard ();
               m_presentState = PresentState.HidingInitialInfo;
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

      void StoryboardCompleted(object sender, EventArgs e)
      {
         switch(m_presentState)
         {
            case PresentState.InitialWait:
               RunPresentStoryboard ();
               m_presentState = PresentState.PresentingInitialInfo;
               break;
            case PresentState.PresentingInitialInfo:
               if (FolderTreeView.Job != null)
               {
                  m_presentState = PresentState.HidingInitialInfo;
                  RunHideStoryboard();
               }
               else
               {
                  m_presentState = PresentState.InitialInfoPresented;
               }
               break;
            case PresentState.InitialInfoPresented:
               Debug.Assert (false);
               break;
            case PresentState.HidingInitialInfo:
               m_presentState = PresentState.HintInfoHidden;
               InitialInfoText.Visibility = Visibility.Collapsed;
               HintInfoText.Visibility = Visibility.Visible;
               RunIdlingStoryboard();
               break;
            case PresentState.HintInfoHidden:
               m_presentState = PresentState.PresentingHintInfo;
               RunPresentStoryboard ();
               break;
            case PresentState.PresentingHintInfo:
               m_presentState = PresentState.HintInfoPresented;
               break;
            case PresentState.HintInfoPresented:
               break;
            case PresentState.HidingHintInfo:
               m_presentState = PresentState.HintInfoHidden;
               RunIdlingStoryboard();
               break;
            default:
               throw new ArgumentOutOfRangeException ();
         }
      }

      // ----------------------------------------------------------------------

      void RunHideStoryboard()
      {
         if (m_hideStoryboard != null)
         {
            m_hideStoryboard.Begin(this);
            m_hideStoryboard.Seek (this, TimeSpan.Zero, TimeSeekOrigin.BeginTime);
         }
      }

      // ----------------------------------------------------------------------

      void RunPresentStoryboard()
      {
         if (m_presentStoryboard != null)
         {
            m_presentStoryboard.Begin(this);
            m_idlingStoryboard.Seek(this, TimeSpan.Zero, TimeSeekOrigin.BeginTime);
         }
      }

      // ----------------------------------------------------------------------

      void RunIdlingStoryboard()
      {
         if (m_idlingStoryboard != null)
         {
            m_idlingStoryboard.Begin(this);
            m_idlingStoryboard.Seek(this, TimeSpan.Zero, TimeSeekOrigin.BeginTime);
         }
      }

      // ----------------------------------------------------------------------

      void UserInputDetected()
      {
         switch (m_presentState)
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
               m_presentState = PresentState.HidingHintInfo;
               break;
            case PresentState.HidingHintInfo:
               break;
            default:
               throw new ArgumentOutOfRangeException();
         }
      }

      // ----------------------------------------------------------------------
   }
}