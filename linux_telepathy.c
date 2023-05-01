#include "telepathy.h"

#ifdef __linux__

#define _GNU_SOURCE
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <errno.h>

typedef struct Linux_Telepathy
{
    pid_t pID;
    struct iovec iovLocal[1];
    struct iovec iovRemote[1];
}Linux_Telepathy;

void TELEPATHY_INIT(Telepathy* telepathy, const char* name)
{
    telepathy->internal = malloc(sizeof(Linux_Telepathy));
    Linux_Telepathy* lin_telepathy = (Linux_Telepathy*)telepathy->internal;

    TELEPATHY_GET_WINDOW_HANDLE(telepathy,name);
}

void TELEPATHY_GET_WINDOW_HANDLE(Telepathy* telepathy, const char* name)
{
    Linux_Telepathy* lin_telepathy = (Linux_Telepathy*)telepathy->internal;

    DIR *pDir = opendir("/proc/");
    if(pDir == NULL) return;

    struct dirent *pDirent;

    char buf[100];

    //Loop over all directories in proc
    while(pDirent = readdir(pDir), pDirent != NULL)
    {
        //Only check directories
        if(pDirent->d_type != DT_DIR) continue;

        char* strProcID = pDirent->d_name;

        char* address = malloc(sizeof(char) * strlen(strProcID) + 10);
        sprintf(address,"%s%s%s","/proc/",strProcID,"/comm");

        FILE* infile = fopen(address,"r");

        free(address);

        if(infile == NULL) continue;
        
        //fgets will continue and include the new line character
        fgets(buf,100,infile);
        //sanitize input by getting the length and strcpy to char*
        int length = 0;
        for(int i=0;i<100;i++)
        {
            if(buf[i]=='\n') break;

            length++;
        }

        char* commStr;
        commStr = malloc(sizeof(char) * length);
        strncpy(commStr,buf,length);

        if(strcmp(commStr,name) == 0)
        {
            lin_telepathy->pID = (pid_t)atoi(strProcID);
            free(commStr);
            printf("PROCESS PID: %d\n",(int)lin_telepathy->pID);
            return;
        }

        free(commStr);
    }
}

//ALLOCATES MEMORY TO "lin_telepathy->iovLocal[0].iov_base".
//THIS MUST BE FREED MANUALLY
void* _readMemory(Telepathy* telepathy, unsigned long long int address, size_t bufferLength)
{
    Linux_Telepathy* lin_telepathy = (Linux_Telepathy*)telepathy->internal;

    lin_telepathy->iovLocal[0].iov_base = calloc(1, bufferLength);
    lin_telepathy->iovLocal[0].iov_len = bufferLength;

    lin_telepathy->iovRemote[0].iov_base = (void*)address;
    lin_telepathy->iovRemote[0].iov_len = bufferLength;
    ssize_t debug = process_vm_readv(lin_telepathy->pID,lin_telepathy->iovLocal,1,lin_telepathy->iovRemote,1,0);

    if(debug <0 )
    {
        switch (errno)
        {
            case EINVAL:
                printf("PROCESS_VM_READV: INVALID ARGUMENTS\n");
                break;
            case EFAULT:
                printf("PROCESS_VM_READV: UNABLE TO ACCESS TARGET MEMORY\n");
                break;
            case ENOMEM:
                printf("PROCESS_VM_READV: UNABLE TO ALLOCATE MEMORY\n");
                break;
            case EPERM:
                printf("PROCESS_VM_READYV INSUFFICENT PRIVILEGES TO TARGET PROCESS\n");
                break;
            case ESRCH:
                printf("PROCESS_VM_READV: PROCESS DOES NOT EXIST\n");
                break;
            default:
                printf("PROCESS_VM_READV: UNKNOWN ERROR\n");
                break;

            return 0;
        }
    }

    void* result = lin_telepathy->iovLocal[0].iov_base;
    return result;

}

unsigned long long int TELEPATHY_LOAD_UINT(Telepathy* telepathy, unsigned long long int address)
{
    Linux_Telepathy* lin_telepathy = (Linux_Telepathy*)telepathy->internal;

    unsigned long long int result = 0;
    size_t bufferLength = sizeof(unsigned long long int);

    result = *(unsigned long long int*)_readMemory(telepathy,address,bufferLength);
    free(lin_telepathy->iovLocal[0].iov_base);

    return result;
}

long long int TELEPATHY_LOAD_INT(Telepathy* telepathy, unsigned long long int address)
{
    Linux_Telepathy* lin_telepathy = (Linux_Telepathy*)telepathy->internal;

    long long int result = 0;
    size_t bufferLength = sizeof(long long int);

    result = *(long long int*)_readMemory(telepathy,address,bufferLength);
    free(lin_telepathy->iovLocal[0].iov_base);

    return result;
}

char TELEPATHY_LOAD_BYTE(Telepathy* telepathy, unsigned long long int address)
{
    Linux_Telepathy* lin_telepathy = (Linux_Telepathy*)telepathy->internal;

    char result = 0;
    size_t bufferLength = sizeof(char);

    result = *(char*)_readMemory(telepathy,address,bufferLength);
    free(lin_telepathy->iovLocal[0].iov_base);

    return result;
}

float TELEPATHY_LOAD_FLOAT(Telepathy* telepathy, unsigned long long int address)
{
    Linux_Telepathy* lin_telepathy = (Linux_Telepathy*)telepathy->internal;

    float result = 0;
    size_t bufferLength = sizeof(float);

    result = *(float*)_readMemory(telepathy,address,bufferLength);
    free(lin_telepathy->iovLocal[0].iov_base);

    return result;
}

void TELEPATHY_LOAD_STRING(Telepathy* telepathy, unsigned long long int address, char** dest, unsigned long long int length)
{
    if(length < 1) return;

    *dest = (char*)malloc(sizeof(char) * length);

    for(int i = 0; i < length; i++)
    {
        *(*dest + i) = TELEPATHY_LOAD_BYTE(telepathy, address + i);
        free(lin_telepathy->iovLocal[0].iov_base);
    }

    *(*dest + length) = '\0';
}

void TELEPATHY_LOAD_UNITYSTRING(Telepathy* telepathy, unsigned long long int address, char** dest)
{
    address = TELEPATHY_LOAD_UINT(telepathy, address);

    unsigned long long int length = TELEPATHY_LOAD_UINT(telepathy, address + 0x10) + 1;

    if(length < 1) return;

    *dest = (char*)malloc(sizeof(char) * length);

    for(int i = 0; i < length; i++)
    {
        *(*dest + i) = TELEPATHY_LOAD_BYTE(telepathy, address + 0x14 + (i*2));
        free(lin_telepathy->iovLocal[0].iov_base);
    }

    *(*dest + (length*2)) = '\0';
}

#endif
