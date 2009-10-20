
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
using System.Windows;

namespace FolderSize.WPF
{
   public partial class App
   {

      // ----------------------------------------------------------------------

      public App()
      {
         AppDomain.CurrentDomain.UnhandledException += CurrentDomain_UnhandledException;
      }

      // ----------------------------------------------------------------------

      static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
      {
         MessageBox.Show (
            string.Format (
               "We are sorry but a program fault occurred while running FolderSize.WPF\r\n{0}",
               e.ExceptionObject));
      }

      // ----------------------------------------------------------------------

   }
}
