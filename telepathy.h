#include <windows.h>
#include <winuser.h>
#include <tlhelp32.h>

typedef struct Telepathy
{
	HANDLE hProc;
	UINT64 pID;
	UINT64 dllAddress;
}Telepathy;

void TELEPATHY_INIT(Telepathy* telepathy);

HANDLE TELEPATHY_GET_WINDOW_HANDLE(Telepathy* telepathy, LPCSTR name);

UINT64 TELEPATHY_GET_DLL_ADDRESS(Telepathy* telepathy, LPCSTR name);

void TELEPATHY_SEND_KEYBOARD_INPUT(UINT key);

UINT64 TELEPATHY_LOAD_UINT(Telepathy* telepathy, UINT64 address);
INT64 TELEPATHY_LOAD_INT(Telepathy* telepathy, UINT64 address);
BYTE TELEPATHY_LOAD_BYTE(Telepathy* telepathy, UINT64 address);
FLOAT TELEPATHY_LOAD_FLOAT(Telepathy* telepathy, UINT64 address);
