using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PgPacker
{
    class Logger
    {
        public static void Introduction()
        {
            Console.Title = "PgPacker by 0x34c";
            Console.ForegroundColor = ConsoleColor.Blue;
            Console.WriteLine("========================================");
            Console.ForegroundColor = ConsoleColor.Red;
            Console.Write(@"   ___       ___           __          
  / _ \___ _/ _ \___ _____/ /_____ ____
 / ___/ _ `/ ___/ _ `/ __/  '_/ -_) __/
/_/   \_, /_/   \_,_/\__/_/\_\\__/_/   
     /___/                             ");
            Console.ForegroundColor = ConsoleColor.Blue;
            Console.WriteLine("\r\n========================================");
            Console.ForegroundColor = ConsoleColor.Gray;
            Console.WriteLine("Coded by 0x34c");
        }
        public static void Log(string message)
        {
            Console.ForegroundColor = ConsoleColor.Gray;
            Console.WriteLine($"[LOGGER] => {message}");
            Console.ForegroundColor = ConsoleColor.Gray;
        }
        public static void Error(string message)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine($"[ERROR] => {message}");
            Console.ForegroundColor = ConsoleColor.Red;
        }
    }
}
