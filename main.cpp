// midiprobe.cpp

#include <iostream>
#include "cmds.h"

using namespace macmidish;

static void beginmsg() {
	std::cout << "macmidish, inspired by midish" << std::endl;
	std::cout << "(c) Matt Kane, RtMidi (c) 2003-2011 Gary P. Scavone" << std::endl;
}
	
int main(int argc, char** argv)
{
	// if no input port num specified, scan input ports and select
	// if no output port num specified, scan output ports and select
	// cmds
	// i (h): input mode: (h)ex, (i)nt, (a)scii, (o)ctal
	// s (filename): send sysex file
	// l (logfile): set log file, save all commands and results there
	// o (bytes): send message
	// p : re-select ports
	// io (timeout) (bytes): send message and expect response
	// fo (timeout) (filename) (bytes): send message and expect response, save response to file
	// q: quit! (also EOF)
	beginmsg();
	PortSelector ps(argc, argv);
	CmdParser cmd(ps.inPort(), ps.outPort());
	cmd.startRepl();
	return 0;
}