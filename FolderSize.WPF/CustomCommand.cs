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
using System.Windows.Input;

namespace FolderSize.WPF
{
   public class CustomCommand : ICommand
   {
      static readonly EventArgs s_eventArgs = new EventArgs ();

      readonly Func<object, bool> m_canExecute;
      readonly Action<object> m_execute;

      public CustomCommand(
         Func<object, bool> canExecute,
         Action<object> execute
         )
      {
         m_execute = execute;
         m_canExecute = canExecute;
      }

      public void Execute (object parameter)
      {
         if (m_execute != null)
         {
            m_execute (parameter);
         }
      }

      public bool CanExecute (object parameter)
      {
         if (m_canExecute != null)
         {
            return m_canExecute(parameter);
         }
         else
         {
            return false;
         }
      }

      public void RaiseCanExecuteChanged(object sender)
      {
         if(CanExecuteChanged != null)
         {
            CanExecuteChanged(sender, s_eventArgs);
         }
      }

      public event EventHandler CanExecuteChanged;
   }
}