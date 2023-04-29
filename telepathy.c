#include "telepathy.h"

void TELEPATHY_INIT(Telepathy* telepathy)
{
	telepathy->hProc = 0;
	telepathy->dllAddress = NULL;
}

HANDLE TELEPATHY_GET_WINDOW_HANDLE(Telepathy* telepathy, LPCSTR name)
{
	//Finding Proccess
	HANDLE handle = NULL;

	handle = FindWindowA(NULL, name);

	if (!handle)
	{
		return NULL;
	}

	UINT32 pID = NULL;
	GetWindowThreadProcessId(handle, &pID);
	telepathy->pID = pID;
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, pID);

	if (!hProc)
	{
		return NULL;
	}

	telepathy->hProc = hProc;
	return hProc;
}

//Yoinked from Stack Overflow https://stackoverflow.com/questions/41552466/how-do-i-get-the-physical-baseaddress-of-an-dll-used-in-a-process
UINT64 TELEPATHY_GET_DLL_ADDRESS(Telepathy* telepathy, LPCSTR name)
{
	MODULEENTRY32 ModuleEntry = { 0 };
	HANDLE SnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, telepathy->pID);

	if (!SnapShot) return NULL;

	ModuleEntry.dwSize = sizeof(ModuleEntry);

	if (!Module32First(SnapShot, &ModuleEntry)) return NULL;

	do
	{
		if (!wcscmp(ModuleEntry.szModule, name))
		{
			CloseHandle(SnapShot);
			telepathy->dllAddress = (char*)ModuleEntry.modBaseAddr;
			return (char*)ModuleEntry.modBaseAddr;
		}
	} while (Module32Next(SnapShot, &ModuleEntry));

	CloseHandle(SnapShot);
	return NULL;
}

void TELEPATHY_SEND_KEYBOARD_INPUT(UINT key)
{
	INPUT Input[2] = { 0 };
	Input[0].type = INPUT_KEYBOARD;
	Input[0].ki.wVk = key;
	Input[1] = Input[0];
	Input[1].ki.dwFlags = KEYEVENTF_KEYUP;

	SendInput(ARRAYSIZE(Input), Input, sizeof(INPUT));
}

UINT64 TELEPATHY_LOAD_UINT(Telepathy* telepathy, UINT64 address)
{
	UINT64 result = 0;
	ReadProcessMemory(telepathy->hProc, (LPVOID*)address, &result, sizeof(UINT64), 0);

	return result;
}

INT64 TELEPATHY_LOAD_INT(Telepathy* telepathy, UINT64 address)
{
	UINT64 result = 0;
	ReadProcessMemory(telepathy->hProc, (LPVOID*)address, &result, sizeof(INT), 0);

	return result;
}

BYTE TELEPATHY_LOAD_BYTE(Telepathy* telepathy, UINT64 address)
{
	BYTE result = 0;
	ReadProcessMemory(telepathy->hProc, (LPVOID*)address, &result, sizeof(BYTE), 0);

	return result;
}

FLOAT TELEPATHY_LOAD_FLOAT(Telepathy* telepathy, UINT64 address)
{
	FLOAT result = 0;
	ReadProcessMemory(telepathy->hProc, (LPVOID*)address, &result, sizeof(FLOAT), 0);

	return result;
}
