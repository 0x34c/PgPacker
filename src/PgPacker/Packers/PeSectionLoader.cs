using System;
using System.IO;
using System.Linq;
using AsmResolver;
using AsmResolver.PE.File;
using AsmResolver.PE.File.Headers;
using System.Reflection;

namespace PgPacker.Packers
{
    class PeSectionLoader
    {
        public static void Pack(string filePath)
        {
            /* We have to load the stub, i'll use AsmResolver for that! */
            PEFile peFile = PEFile.FromFile("PgStub.exe");
            Logger.Log("Loaded stub!");

            /* Encrypting the file using xor */
            byte[] enc = File.ReadAllBytes(filePath).Select(x => (byte)(x ^ peFile.FileHeader.TimeDateStamp)).ToArray();
            Logger.Log("Encrypted payload!");

            /* Inserting it inside PESection */
            PESection section = new PESection(".pgpack", SectionFlags.MemoryRead | SectionFlags.ContentUninitializedData);
            section.Contents = new DataSegment(enc);
            peFile.Sections.Add(section);
            Logger.Log("Inserted into PESection!");

            /* Writing the file! */
            peFile.Write($"{Path.GetDirectoryName(filePath)}\\{Path.GetFileNameWithoutExtension(filePath)}_packed{Path.GetExtension(filePath)}");
            Logger.Log("Finishing...");
        }
    }
}
