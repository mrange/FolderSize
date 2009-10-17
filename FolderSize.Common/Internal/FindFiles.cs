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
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;

namespace FolderSize.Common.Internal
{
   class FindFiles : IEnumerator<FileData>
   {
      // ReSharper disable InconsistentNaming
      // The CharSet must match the CharSet of the
      // corresponding PInvoke signature
      [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
      struct WIN32_FIND_DATA
      {
         public FileAttributes dwFileAttributes;
         public System.Runtime.InteropServices.ComTypes.FILETIME ftCreationTime;
         public System.Runtime.InteropServices.ComTypes.FILETIME ftLastAccessTime;
         public System.Runtime.InteropServices.ComTypes.FILETIME ftLastWriteTime;
         public uint nFileSizeHigh;
         public uint nFileSizeLow;
         public uint dwReserved0;
         public uint dwReserved1;
         [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
         public string cFileName;
         [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 14)]
         public string cAlternateFileName;
      }
      // ReSharper restore InconsistentNaming
      [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
      static extern IntPtr FindFirstFile(string lpFileName, out WIN32_FIND_DATA
                                                               lpFindFileData);

      [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
      static extern bool FindNextFile(IntPtr hFindFile, out WIN32_FIND_DATA
                                                           lpFindFileData);

      [DllImport("kernel32.dll")]
      static extern bool FindClose(IntPtr hFindFile);

      readonly string m_path;

      IntPtr m_hnd = IntPtr.Zero;
      WIN32_FIND_DATA m_findData;

      public FindFiles(string path)
      {
         m_path = path;
      }

      ~FindFiles()
      {
         Dispose(false);
      }

      public FileData Current
      {
         get
         {
            return new FileData(
               Path.Combine(m_path, m_findData.cFileName),
               m_findData.cFileName,
               (m_findData.dwFileAttributes & FileAttributes.Directory) == FileAttributes.Directory,
               (((long)m_findData.nFileSizeHigh) << 32) + m_findData.nFileSizeLow);
         }
      }

      public void Dispose()
      {
         GC.SuppressFinalize(this);
         Dispose(true);
      }

      void Dispose(bool p)
      {
         if (m_hnd != IntPtr.Zero)
         {
            FindClose (m_hnd);
            m_hnd = IntPtr.Zero;
         }
      }

      object IEnumerator.Current
      {
         get
         {
            return Current;
         }
      }

      public bool MoveNext()
      {
         if (m_hnd == IntPtr.Zero)
         {
            Reset();
            return m_hnd != IntPtr.Zero;
         }

         return FindNextFile(
            m_hnd,
            out m_findData);
      }

      public void Reset()
      {
         if (m_hnd != IntPtr.Zero)
         {
            FindClose(m_hnd);
            m_hnd = IntPtr.Zero;
         }

         m_hnd = FindFirstFile(
            Path.Combine(m_path, "*.*"),
            out m_findData);

      }
   }
}