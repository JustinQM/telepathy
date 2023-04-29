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

void TELEPATHY_INIT(Telepathy* telepathy,char* name)
{
    telepathy->internal = malloc(sizeof(Linux_Telepathy));
    Linux_Telepathy* lin_telepathy = (Linux_Telepathy*)telepathy->internal;

    TELEPATHY_GET_WINDOW_HANDLE(telepathy,name);
}

void TELEPATHY_GET_WINDOW_HANDLE(Telepathy* telepathy, char* name)
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
            return;
        }

        free(commStr);
    }
}

unsigned long long int TELEPATHY_LOAD_UINT(Telepathy* telepathy, unsigned long long int address)
{
    Linux_Telepathy* lin_telepathy = (Linux_Telepathy*)telepathy->internal;

    unsigned long long result = 0;

    lin_telepathy->iovLocal[0].iov_base = &result;
    lin_telepathy->iovLocal[0].iov_len = 64;

    lin_telepathy->iovRemote[0].iov_base = (void*)address;
    lin_telepathy->iovRemote[0].iov_len = 64;
    ssize_t debug = process_vm_readv(lin_telepathy->pID,lin_telepathy->iovLocal,1,lin_telepathy->iovRemote,1,0);

    if(debug != 64) printf("frick\n");

    return (unsigned long long int)result;
}

#endif
