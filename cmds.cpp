/*
 *  cmds.cpp
 *  macmidish
 *
 *  Created by Matthew Kane on 3/4/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "cmds.h"
#include <algorithm>
#incude <sstream>

using namespace macmidish;

namespace macmidish {
	static RtMidiOut midiout;
	static RtMidiIn midiin;
}

RtMidiOut midiout;
RtMidiIn midiin;

void PortSelector::scanSelect() {
	unsigned int outPorts = midiout.getPortCount();
	unsigned int inPorts = midiin.getPortCount();
	unsigned int max = std::max(inPorts, outPorts);
	std::cout << "Please select the MIDI port you want to communicate with:" << std::endl;
	for (unsigned int i = 0; i <= max; ++i) {
		std::cout << i << ".) " << (i<inPorts)?(midiin.getPortName(i)):("(output only)")
			<< "/" << (o<outPorts)?(midiout.getPortName(i)):("(input only)");
	}
	
	char *resp = NULL;
	do {
		resp = readline("input>");
	} while (!resp || !(*resp));
	
	std::istringstream is;
	is.str(resp);
	is >> _inPort;
	free(resp);
	
	resp = NULL;
	do {
		resp = readline("output>");
	} while (!resp || !(*resp));
	
	is.str(resp);
	is >> _outPort;
	free(resp);
}

