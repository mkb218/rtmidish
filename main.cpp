// midiprobe.cpp

#include <iostream>
#include <cstdlib>
#include <readline/readline.h>
#include "RtMidi.h"

int main()
{
	// if no input port num specified, scan input ports and select
	// if no output port num specified, scan output ports and select
	// cmds
	// i (h): input mode: hex, int, ascii
	// s (filename): send sysex file
	// l (logfile): set log file, save all commands and results there
	// o (bytes): send message
	// io (bytes): send message and expect response
	// fo (filename) (bytes): send message and expect response, save response to file
	//
	return 0;
}