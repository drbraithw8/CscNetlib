// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#ifndef csc_NETCLI_H
#define csc_NETCLI_H 1

typedef struct csc_cli_t csc_cli_t ;

// Constructor.  Create new netCli object.  Returns pointer to object on
// success.  Returns NULL on failure.
csc_cli_t *csc_cli_new();


// Tell the netCli object what we will connect to.
// 
// 'conType' must be either "TCP" or "UDP".
// 
// The address may be in the form of a URL, e.g. "www.google.com".  Or it
// may be in IPV4 format, e.g. "192.168.0.3".  Or it may be in IPV6 format,
// e.g. "2001:0db8:c9d2:0012:0000:0000:0000:0051" or
// "2001:db8:c9d2:12::51".
// 
// Returns 1 on success, and 0 on failure.  Use csc_cli_getErrMsg() 
// to get details of failure.
int csc_cli_setServAddr( csc_cli_t *cli
                      , const char *conType
                      , const char *addr
                      , int portNo
                      );
                      

// Attempt to connect to selected server.  csc_cli_setServAddr() must
// have been called successfully before calling this routine.  
// 
// Returns a file descriptor on success, and -1 on failure.  Use
// csc_cli_getErrMsg() to get details of failure.
int csc_cli_connect(csc_cli_t *cli);


// Destructor.  Cleans up memory associated with a netCli object.
void csc_cli_free(csc_cli_t *cli);



// ---------------------  What was the error ------------------

// Returns a string representation of details of a previous error.  The
// string returned is valid until the next non const method call.
const char *csc_cli_getErrMsg(const csc_cli_t *cli);


// ---------------------  Might not need ------------------

// TODO
// // Returns the selected IP address to be connected to.  Returns NULL if
// // none.  The string returned is valid until the next non const method
// // call.
// const char *csc_cli_getSelectedIP(const csc_cli_t *cli);

// ---------------------  Might not need ------------------
// TODO
// // If the address used in the call to csc_cli_setServAddr() was in the
// // form of a URL, there may be more than one resulting IP address.  By
// // default that routine selects one at random.  These routines give tighter
// // control over that.
// 
// 
// // Returns the number of IP addresses discovered by a call to 
// // csc_cli_setServAddr().
// int csc_cli_getNumIp(const csc_cli_t *cli);
// 
// 
// // Returns the first IP address in the form of a string.  The string
// // returned is valid until the next non const method call.
// const char *csc_cli_getFirstIP(const csc_cli_t *cli);
// 
// // Returns the next IP address in the form of a string.  The string
// // returned is valid until the next non const method call.
// const char *csc_cli_getNextIP(const csc_cli_t *cli);
// 
// 
// // Tells the netCli object to use the iIp'th IP number.
// int csc_cli_chooseIp(csc_cli_t *cli, int iIp);

#endif
