// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#ifndef csc_SIGNAL_H
#define csc_SIGNAL_H 1

// Adds a layer so that multiple signal handlers may be called whenever the
// program receives a given signal.  Routines here are currently not
// implemented in a threadsafe way, but are intended to be made threadsafe
// in the future.  

// This adds or registers a function handler() to be called with its
// context, 'context' whenever a signal number 'sigNum' is received.  The
// context serves two purposes.  It is passed as the argument of the signal
// handler, and it is also used to identify a handler for removal using
// csc_signal_delHndl(). 
void csc_signal_addHndl(int sigNum, void (*handler)(int sigNum, void *context), void *context);


// Identifies a handler by the combination of the signal number 'sigNum'
// and the context, 'context', and removes the handler.
int csc_signal_delHndl(int sigNum, void *context);

#endif

