#include "telepathy.h"

//Windows Specific
#ifdef _WIN32

#include <windows.h>
#include <winuser.h>
#include <tlhelp32.h>
#include <stdio.h>



typedef struct Win_Telepathy
{
	HANDLE hProc;
	UINT64 pID;
	UINT64 dllAddress;
}Win_Telepathy;

void TELEPATHY_INIT(Telepathy* telepathy, const char* name)
{
	
	telepathy->internal = malloc(sizeof(Win_Telepathy));
	Win_Telepathy *win_telepathy = (Win_Telepathy*)telepathy->internal;

	TELEPATHY_GET_WINDOW_HANDLE(telepathy,name);
}

void TELEPATHY_GET_WINDOW_HANDLE(Telepathy* telepathy, const char* name)
{
	
	Win_Telepathy *win_telepathy = (Win_Telepathy*)telepathy->internal;
	//Finding Proccess
	HANDLE handle;

	handle = FindWindowA(NULL, name);

	if (!handle)
	{
		printf("Could not load Handle\n");
		exit(1);
	}

	unsigned long int pID;
	GetWindowThreadProcessId((HWND)handle, &pID);
	win_telepathy->pID = (UINT64)pID;
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, (DWORD)pID);

	if (!hProc)
	{
		printf("Could not load hProc");
		exit(1);
	}

	win_telepathy->hProc = hProc;
}

//Yoinked from Stack Overflow https://stackoverflow.com/questions/41552466/how-do-i-get-the-physical-baseaddress-of-an-dll-used-in-a-process
void TELEPATHY_GET_DLL_ADDRESS(Telepathy* telepathy, const char* name)
{
	Win_Telepathy *win_telepathy = (Win_Telepathy*)telepathy->internal;

	
	MODULEENTRY32 ModuleEntry = { 0 };
	HANDLE SnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, win_telepathy->pID);

	if (!SnapShot) return;

	ModuleEntry.dwSize = sizeof(ModuleEntry);

	if (!Module32First(SnapShot, &ModuleEntry)) return;

	do
	{
		if (!wcscmp((wchar_t*)ModuleEntry.szModule, (wchar_t*)name))
		{
			CloseHandle(SnapShot);
			win_telepathy->dllAddress = (UINT64)ModuleEntry.modBaseAddr;
			return;
		}
	} while (Module32Next(SnapShot, &ModuleEntry));

	CloseHandle(SnapShot);
	return;
}

void TELEPATHY_SEND_KEYBOARD_INPUT(unsigned long int key)
{
	INPUT Input[2] = { 0 };
	Input[0].type = INPUT_KEYBOARD;
	Input[0].ki.wVk = key;
	Input[1] = Input[0];
	Input[1].ki.dwFlags = KEYEVENTF_KEYUP;

	SendInput(ARRAYSIZE(Input), Input, sizeof(INPUT));
}

unsigned long long int TELEPATHY_LOAD_UINT(Telepathy* telepathy, unsigned long long int address)
{
	Win_Telepathy *win_telepathy = (Win_Telepathy*)telepathy->internal;
	
	UINT64 result = 0;
	ReadProcessMemory(win_telepathy->hProc, (LPVOID*)address, &result, sizeof(UINT64), 0);

	return result;
}

long long int TELEPATHY_LOAD_INT(Telepathy* telepathy, UINT64 address)
{
	Win_Telepathy *win_telepathy = (Win_Telepathy*)telepathy->internal;
	
	INT64 result = 0;
	ReadProcessMemory(win_telepathy->hProc, (LPVOID*)address, &result, sizeof(INT), 0);

	return result;
}

char TELEPATHY_LOAD_BYTE(Telepathy* telepathy, unsigned long long int address)
{
	Win_Telepathy *win_telepathy = (Win_Telepathy*)telepathy->internal;
	
	BYTE result = 0;
	ReadProcessMemory(win_telepathy->hProc, (LPVOID*)address, &result, sizeof(BYTE), 0);

	return result;
}

float TELEPATHY_LOAD_FLOAT(Telepathy* telepathy, unsigned long long int address)
{
	Win_Telepathy *win_telepathy = (Win_Telepathy*)telepathy->internal;
	
	FLOAT result = 0;
	ReadProcessMemory(win_telepathy->hProc, (LPVOID*)address, &result, sizeof(FLOAT), 0);

	return result;
}

void TELEPATHY_LOAD_UNITYSTRING(Telepathy* telepathy, unsigned long long int address, char** dest)
{
	address = TELEPATHY_LOAD_UINT(telepathy, address);

	UINT64 length = TELEPATHY_LOAD_INT(telepathy, address + 0x10) + 1;

	if (length == 0x1) return;

	*dest = (char*)malloc(sizeof(char) * length);

	UINT64 stringOffset = 0x14;

	for (int i = 0; i < length; i++)
	{
		*(*dest + i) = TELEPATHY_LOAD_BYTE(telepathy, address + stringOffset + (i*2));
	}
}

#endif
