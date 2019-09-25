// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>

#include "std.h"
#include "alloc.h"
#include "list.h"
#include "signal.h"

typedef struct
{   void *context;
    void (*handler)(int sigNum, void *context);
} sigReg_t;

typedef struct 
{   int sigNum;
    csc_list_t *regLst;
} sigRegs_t;


static csc_list_t *sigRegLists = NULL;


static void sigHandler(int sigNum, siginfo_t *sigInfo, void *notUsed)
{   csc_list_t *pls;
    sigRegs_t *regs=NULL;
    sigReg_t *reg=NULL;
    
// Search for sigNum on sigRegLists.
    for (pls=sigRegLists; pls!=NULL; pls=pls->next)
    {   regs = pls->data;
        if (regs->sigNum == sigNum)
            break;
    }
 
// Call the handler for each registration on the list.
    if (pls != NULL)
    {   for (pls=regs->regLst; pls!=NULL; pls=pls->next)
        {   reg = pls->data;
            reg->handler(sigNum, reg->context);
        }
    }
}


int csc_signal_delHndl(int sigNum, void *context)
{   csc_list_t *pls, **prevPt;
    sigRegs_t *regs = NULL;
    sigReg_t *reg = NULL;
 
// Search for sigNum on sigRegLists.
    for (pls=sigRegLists; pls!=NULL; pls=pls->next)
    {   regs = pls->data;
        if (regs->sigNum == sigNum)
            break;
    }
    if (pls == NULL)
        return csc_FALSE;
 
// Search for context on the regList
    prevPt = &regs->regLst;
    for (pls=regs->regLst; pls!=NULL; pls=pls->next)
    {   reg = pls->data;
        if (reg->context == context)
            break;
        prevPt = &pls->next;
    }
    if (pls == NULL)
        return csc_FALSE;
 
// Unlink the matched context record.
    *prevPt = pls->next;
 
// Free the unlinked record.
    free(pls->data);
    free(pls);
    return csc_TRUE;
}
                

void csc_signal_addHndl( int sigNum
                    , void (*handler)(int sigNum, void *context)
                    , void *context
                    )
{   csc_list_t *pls;
    sigRegs_t *regs;
    sigReg_t *reg;
 
// Search for sigNum on sigRegLists.
    for (pls=sigRegLists; pls!=NULL; pls=pls->next)
    {   regs = pls->data;
        if (regs->sigNum == sigNum)
            break;
    }
 
// Add a new reglist if sigNum not already present.
    if (pls == NULL)
    {   struct sigaction sigAction;

    // Add the list for sigNum.
        regs = csc_allocOne(sigRegs_t);
        regs->sigNum = sigNum;
        regs->regLst = NULL;
        csc_list_add(&sigRegLists, regs);
 
    // Register the signal sigNum.
        bzero(&sigAction, sizeof(sigAction));
        sigAction.sa_sigaction = &sigHandler;
        sigAction.sa_flags = SA_SIGINFO;
        sigemptyset(&sigAction.sa_mask);
        sigaction(sigNum, &sigAction, NULL);
    }
 
// Add registration to list of registrations.
    reg = csc_allocOne(sigReg_t);
    reg->handler = handler;
    reg->context = context;
    csc_list_add(&regs->regLst, reg);
}


#if 0

typedef struct
{   char *color;
} color_t;


void handler(int sigNum, void *context)
{   color_t *color = context;
    fprintf(csc_stderr, "Got signal %d %s\n", signum, color->color);
}


void main(int argc, char **argv)
{   
    color_t red;
    red.color = "red";
    csc_signal_addHndl(SIGINT, handler, &red);
 
    color_t green;
    green.color = "green";
    csc_signal_addHndl(SIGINT, handler, &green);
 
    color_t blue;
    blue.color = "blue";
    csc_signal_addHndl(SIGINT, handler, &blue);
 
    color_t yellow;
    yellow.color = "yellow";
    csc_signal_addHndl(SIGINT, handler, &yellow);
 
    color_t purple;
    purple.color = "purple";
    csc_signal_addHndl(SIGTERM, handler, &purple);
 
    color_t orange;
    orange.color = "orange";
    csc_signal_addHndl(SIGTERM, handler, &orange);
 
    sleep(200);
    sleep(200);
    csc_signal_delHndl(SIGINT, &red);
    csc_signal_delHndl(SIGINT, &green);
    csc_signal_delHndl(SIGINT, &yellow);
    sleep(200);
    sleep(200);
}

#endif

