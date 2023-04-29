typedef struct Telepathy
{
	void* internal;
}Telepathy;


void TELEPATHY_INIT(Telepathy* telepathy,char* name);

void TELEPATHY_GET_WINDOW_HANDLE(Telepathy* telepathy, char* name);

// void TELEPATHY_GET_DLL_ADDRESS(Telepathy* telepathy, char* name);

// void TELEPATHY_SEND_KEYBOARD_INPUT(unsigned long int key);

unsigned long long int TELEPATHY_LOAD_UINT(Telepathy* telepathy, unsigned long long int address);
// long long int TELEPATHY_LOAD_INT(Telepathy* telepathy, unsigned long long int address);
// char TELEPATHY_LOAD_BYTE(Telepathy* telepathy, unsigned long long int address);
// float TELEPATHY_LOAD_FLOAT(Telepathy* telepathy, unsigned long long int address);
