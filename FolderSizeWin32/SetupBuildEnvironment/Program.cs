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
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Net;
using System.Diagnostics;

namespace SetupBuildEnvironment
{
   class ExitCodeException : Exception
   {
      public readonly int ExitCode;

      public ExitCodeException(int exitCode)
      {
         ExitCode = exitCode;
      }
   }

   class Program
   {
      static void StartProcess(
         string exePath,
         string workingDirectory,
         string arguments)
      {
         var psi = new ProcessStartInfo
         {
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            WindowStyle = ProcessWindowStyle.Minimized,
            FileName = exePath,
            CreateNoWindow = true,
            WorkingDirectory = workingDirectory,
            Arguments = arguments,
            UseShellExecute = false,
         };
         var pi = new Process
         {
            StartInfo = psi,
         };

         Console.WriteLine("Starting: {0} {1}", psi.FileName, psi.Arguments);
         Console.WriteLine(">> --------------------------------");
         if (pi.Start() && pi.WaitForExit(100000))
         {
            //var output = pi.StandardOutput.ReadToEnd();
            var error = pi.StandardError.ReadToEnd();
            //Console.WriteLine(output);
            Console.WriteLine(error);

            Console.WriteLine("<< --------------------------------");

            if (pi.ExitCode != 0)
            {
               Console.WriteLine("Completed with failure code: {0}", pi.ExitCode);
               throw new ExitCodeException (103);
            }

            Console.WriteLine("Completed");
            return;
         }
         else
         {
            pi.Kill();

            //var output = pi.StandardOutput.ReadToEnd();
            var error = pi.StandardError.ReadToEnd();
            //Console.WriteLine(output);
            Console.WriteLine(error);
            Console.WriteLine("<< --------------------------------");
            Console.WriteLine("Timed-out");

            throw new ExitCodeException(104);
         }

      }



      static void Main(string[] args)
      {
         try
         {
            Console.WriteLine("---------------------------------");
            Console.WriteLine("SetupBuildEnvironment starting...");

            var basePath = AppDomain.CurrentDomain.BaseDirectory;

            var boostWebAddress = @"http://downloads.sourceforge.net/project/boost/boost/1.42.0/boost_1_42_0.7z";

            var externalPath = Path.GetFullPath (Path.Combine(basePath, @"..\..\External"));
            var _7zPath = Path.GetFullPath (Path.Combine (externalPath, @"7z.exe"));
            var boost7zPath = Path.GetFullPath (Path.Combine (externalPath, @"boost_1_42_0.7z"));
            var boostDirPath = Path.GetFullPath (Path.Combine (externalPath, @"boost_1_42_0"));

            Console.WriteLine ("base path                   : {0}", basePath);
            Console.WriteLine ("external path               : {0}", externalPath);
            Console.WriteLine ("7z path                     : {0}", _7zPath);
            Console.WriteLine ("zipped boost source path    : {0}", boost7zPath);
            Console.WriteLine ("unzipped boost source path  : {0}", boostDirPath);

            if (!File.Exists(_7zPath))
            {
               Console.WriteLine("{0}: not found, try svn update", _7zPath);
               throw new ExitCodeException (102);
            }

            if (!File.Exists(boost7zPath))
            {
               Console.WriteLine(
                     "Zipped boost source missing, attempting to downloading it from '{0}'"
                  ,  boostWebAddress);

               var wc = new WebClient ();
               wc.DownloadFile(
                     boostWebAddress
                  ,  boost7zPath
                  );

               Console.WriteLine("Zipped boost source downloaded successfully");
            }

            if (!Directory.Exists(boostDirPath))
            {
               Console.WriteLine(
                     "Unzipped boost source not found, unzipping the downloaded zipped boost source"
                  );

               StartProcess (
                     _7zPath
                  ,  externalPath
                  ,  "x -y boost_1_42_0.7z"
                  );

               Console.WriteLine(
                     "Boost source unzipped"
                  );
            }

            Console.WriteLine("SetupBuildEnvironment SUCCESSFUL");
            Console.WriteLine("---------------------------------");
            Environment.ExitCode = 0;
         }
         catch (ExitCodeException exc)
         {
            Console.WriteLine("SetupBuildEnvironment FAILED");
            Console.WriteLine("---------------------------------");

            Environment.ExitCode = exc.ExitCode;
         }
         catch (Exception exc)
         {
            Console.WriteLine("SetupBuildEnvironment EXCEPTION CAUGHT\r\n{0}" ,exc);
            Console.WriteLine("---------------------------------");

            Environment.ExitCode = 101;
         }

      }
   }
}
