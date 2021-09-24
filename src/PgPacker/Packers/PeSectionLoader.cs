using System;
using System.IO;
using System.Linq;
using AsmResolver;
using AsmResolver.PE.File;
using AsmResolver.PE.File.Headers;
using System.Reflection;

namespace PgPacker.Packers
{
    class PeSectionLoader : IPacker
    {
        private string _filePath { get; set; }
        public PeSectionLoader(string filePath)
        {
            this._filePath = filePath;
        }
        public void Pack()
        {
            /* We have to load the stub, i'll use AsmResolver for that! */
            PEFile peFile = PEFile.FromFile("PgStub.exe");
            Logger.Log("Loaded stub!");

            /* Encrypting the file using xor */
            byte[] enc = File.ReadAllBytes(_filePath).Select(x => (byte)(x ^ peFile.FileHeader.TimeDateStamp)).ToArray();
            Logger.Log("Encrypted payload!");

            /* Inserting it inside PESection */
            PESection section = new PESection(".pgpack", SectionFlags.MemoryRead | SectionFlags.ContentUninitializedData);
            section.Contents = new DataSegment(enc);
            peFile.Sections.Add(section);
            Logger.Log("Inserted into PESection!");

            /* Writing the file! */
            peFile.Write($"{Path.GetDirectoryName(_filePath)}\\{Path.GetFileNameWithoutExtension(_filePath)}_packed{Path.GetExtension(_filePath)}");
            Logger.Log("Finishing...");
        }
    }
}
