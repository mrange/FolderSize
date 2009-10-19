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