#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__

#include "kernel.h"

// Create a hex dump of a region of ram, displayed in BBC Miro/Archimedes *DUMP style,
// dumped to current_console.
void DumpHex(unsigned char* address, u32int length);

#endif
