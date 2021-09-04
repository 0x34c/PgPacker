using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using PgPacker.Packers;

namespace PgPacker
{
    class Program
    {
        static void Main(string[] args)
        {
            Logger.Introduction();
            string filePath;
            if (args.Length == 0 || !args[0].EndsWith(".exe"))
            {
                Console.WriteLine("Drag & Drop your file here: ");
                filePath = Console.ReadLine().Replace("\"", string.Empty);
            }
            else
                filePath = args[0].Replace("\"", string.Empty);
            if (!File.Exists("PgStub.exe"))
            {
                Logger.Error("Missing stub! Please make sure the stub is in the same folder!");
                Console.ReadKey();
                Environment.Exit(0);
            }
            Logger.Log("Starting packing!");
            PeSectionLoader.Pack(filePath);
            Logger.Log($"Done! File has been saved at {Path.GetDirectoryName(filePath)}\\{Path.GetFileNameWithoutExtension(filePath)}_packed{Path.GetExtension(filePath)}");
        }
    }
}
