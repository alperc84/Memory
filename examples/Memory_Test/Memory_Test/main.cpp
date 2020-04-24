#include "pch.h"
#include "../../../mem/mem.h"

#if defined(UCS) //Unicode
#define PROCESS_NAME L"Memory_Test.exe"
#define WINDOW_TITLE L"Memory_Test"
#elif defined(MBCS) //Multibyte
#define PROCESS_NAME "Memory_Test.exe"
#define WINDOW_TITLE "Memory_Test"
#endif

#define DELAY 1000
#define NEW_VALUE (int)500

#if defined(ARCH_X86) //x86 Process
#define CONVENTION __cdecl
#define HOOK_LENGTH 9
#define JUMP_INSTRUCTION_LENGTH 5
#elif defined(ARCH_X64) //x64 Process
#define CONVENTION __fastcall
#define HOOK_LENGTH 13
#define JUMP_INSTRUCTION_LENGTH 5
#endif

void CONVENTION Function(int number)
{
	std::cout << "Your number: " << std::dec << number << std::endl;
}

typedef void(CONVENTION* Function_t)(int number);
Function_t oFunction;

void CONVENTION hkFunction(int number)
{
	std::cout << "Hooked!" << std::endl;
	std::cout << "Old number: " << std::dec << number << std::endl;
	number = NEW_VALUE;
	std::cout << "New number: " << std::dec << number << std::endl;
	return oFunction(number);
}

#if defined(MEMORY)
int main()
{
	int targetVariable = 100;
	//--Windows--
#	if defined(WIN)
	SetConsoleTitle(WINDOW_TITLE); //Making sure the console window title is WINDOW_TITLE
	Sleep(DELAY);
	//External Test (Targetting this process)
	std::cout << "External" << std::endl;
	//Process Info
	HANDLE ntHandle = Memory::Ex::Nt::GetProcessHandle(PROCESS_NAME);

	pid_t pid = Memory::Ex::GetProcessIdByName(PROCESS_NAME);
	pid_t window_pid = Memory::Ex::GetProcessIdByWindow(WINDOW_TITLE);
	pid_t ntpid = Memory::Ex::Nt::GetProcessID(PROCESS_NAME);
	pid_t ntHandlePid = Memory::Ex::GetProcessIdByHandle(ntHandle);
	HANDLE hProc = Memory::Ex::GetProcessHandle(pid);
	HWND window_handle = Memory::Ex::GetWindowHandle(pid);
	HANDLE ntOpenHandle = Memory::Ex::Nt::OpenProcessHandle(ntpid);

	std::cout << "NtHandle: " << ntHandle << std::endl;
	std::cout << "NtOpenHandle: " << ntOpenHandle << std::endl;
	std::cout << "PID: " << pid << std::endl;
	std::cout << "Window PID: " << window_pid << std::endl;
	std::cout << "Window Handle: " << window_handle << std::endl;
	std::cout << "Nt PID: " << ntpid << std::endl;
	std::cout << "Nt Handle PID: " << ntHandlePid << std::endl;
	std::cout << "Handle: " << hProc << std::endl;

	//Reading / Writing Memory
	int buffer;
	Memory::Ex::WriteBuffer(ntOpenHandle, (mem_t)&buffer, new int(500), sizeof(buffer));
	std::cout << "Buffer (NtOpenHandle): " << buffer << std::endl;

	Memory::Ex::Nt::CloseProcessHandle(ntOpenHandle);
	Memory::Ex::WriteBuffer(ntOpenHandle, (mem_t)&buffer, new int(10), sizeof(buffer));
	std::cout << "Buffer (NtOpenHandle closed): " << buffer << std::endl;

	Memory::Ex::ReadBuffer(hProc, (mem_t)&targetVariable, &buffer, sizeof(buffer));
	std::cout << "Buffer: " << buffer << std::endl;

	Memory::Ex::ReadBuffer(ntHandle, (mem_t)&targetVariable, &buffer, sizeof(buffer));

	std::cout << "Buffer (Nt): " << buffer << std::endl;

	Memory::Ex::WriteBuffer(hProc, (mem_t)&targetVariable, new int(NEW_VALUE), sizeof(targetVariable));
	std::cout << "TargetVariable: " << targetVariable << std::endl;

	Memory::Ex::WriteBuffer(ntHandle, (mem_t)&targetVariable, new int(buffer), sizeof(targetVariable));
	std::cout << "TargetVariable (resetted): " << targetVariable << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
	//Internal Test (Targetting this process)
	std::cout << "Internal" << std::endl;
	std::cout << "PID: " << Memory::In::GetCurrentProcessID() << std::endl;
	std::cout << "Handle: " << Memory::In::GetCurrentProcessHandle() << std::endl;
	std::cout << "Window Handle: " << Memory::In::GetCurrentWindowHandle() << std::endl;

	//Reading / Writing memory
	int ibuffer;
	Memory::In::ReadBuffer((mem_t)&targetVariable, &ibuffer, sizeof(ibuffer));
	std::cout << "Buffer: " << ibuffer << std::endl;
	Memory::In::WriteBuffer((mem_t)&targetVariable, new int(NEW_VALUE), sizeof(targetVariable));
	std::cout << "Target Variable: " << targetVariable << std::endl;
	Memory::In::Write<int>((mem_t)&targetVariable, buffer);
	buffer = Memory::In::Read<int>((mem_t)&targetVariable);
	std::cout << "Target Variable (resetted): " << buffer << std::endl;

	//Hooking
	std::cout << "Function Address: " << Function << std::endl;
	std::cout << "Press ENTER to Hook Function...";
	getchar();
	mem_t FunctionOffset = 0;
	memcpy(&FunctionOffset, (void*)((mem_t)Function + 1), sizeof(DWORD)); //Reading as DWORD, as it is a short jump
	mem_t FunctionAddress = ((mem_t)Function + JUMP_INSTRUCTION_LENGTH) + FunctionOffset;
	std::cout << "FunctionOffset: " << std::uppercase << std::hex << FunctionOffset << std::endl;
	std::cout << "FunctionAddress: " << std::uppercase << std::hex << FunctionAddress << std::endl;
	oFunction = (Function_t)Memory::In::Hook::TrampolineHook((byte_t*)FunctionAddress, (byte_t*)hkFunction, HOOK_LENGTH);
	Function(50);
#	endif

	//Pause
	std::cout << "Press ENTER to close...";
	getchar();
	return 0;
}
#endif