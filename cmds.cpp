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
#include <sstream>
#include <stdlib.h>
#include <cstring>

using namespace macmidish;

namespace macmidish {
	static RtMidiOut midiout;
	static RtMidiIn midiin;
}

RtMidiOut midiout;
RtMidiIn midiin;

static void eatwhitespace(char ** msg) {
	while (isspace(**msg)) {
		++(*msg);
	}
}

void PortSelector::scanSelect(bool inPort, bool outPort) {
	unsigned int outPorts = midiout.getPortCount();
	unsigned int inPorts = midiin.getPortCount();
	unsigned int max = std::max(inPorts, outPorts);
	std::cout << "Please select the MIDI port you want to communicate with:" << std::endl;
	for (unsigned int i = 0; i <= max; ++i) {
		std::cout << i << ".) " << (i<inPorts)?(midiin.getPortName(i)):("(output only)")
			<< "/" << (o<outPorts)?(midiout.getPortName(i)):("(input only)");
	}
	
	char *resp = NULL;
	if (inPort) {
		do {
			resp = readline("input>");
		} while (!resp || !(*resp));
		
		std::istringstream is;
		is.str(resp);
		is >> _inPort;
		free(resp);
	}
	
	if (outPort) {
		resp = NULL;
		do {
			resp = readline("output>");
		} while (!resp || !(*resp));
		
		is.str(resp);
		is >> _outPort;
		free(resp);
	}
	
	_ready = true;
}

void PortSelector::parseArgs() {
	bool inPort = true;
	bool outPort = true;
	if (!_ready) {
		for (int i = 1; i < _argc; ++i) {
			if (strncmp(_argv[i], "-in", 4) == 0 && i + 1 < argc) {
				++i;
				inPort = false;
				_inPort = strtol(argv[i], NULL, 10);
			} else if (strncmp(_argv[i], "-out", 4) == 0 && i + 1 < argc) {
				++i;
				outPort = false;
				_outPort = strtol(argv[i], NULL, 10);
			}
		}
	}
	if (inPort || outPort) {
		scanSelect(inPort, outPort);
	}
	_ready = true;
}

void PortSelector::inPort() {
	if (!_ready) {
		parseArgs();
	}
	return _inPort;
}

void PortSelector::outPort() {
	if (!_ready) {
		parseArgs();
	}
	return _outPort;
}

void CmdParser::startRepl() {
	while (!_quit) {
		char * cmd = issueprompt();
		if (cmd) {
			if (*cmd) {
				parsecmd(foo);
				add_history(cmd);
			}
			
			free(cmd);
		}
	}
}

char * CmdParser::issueprompt() {
	char * resp = NULL;
	do {
		resp = readline(">");
	} while (!resp || !(*resp));
	return resp;
}

void CmdParser::parsecmd(char * cmd) {
	Cmd::newCmd(cmd)->execute(*this);
}

Cmd * Cmd::newCmd(const char * msg) {
	const char * orig = msg;
	eatwhitespace(&msg);
	switch (*msg) {
		case 'i':
			++msg;
			if (isspace(*msg)) {
				eatwhitespace(&msg);
				return new SetInputMode(msg);
			} else if (*msg == 'o') { 
				eatwhitespace(&msg);
				return new SendMsgWithResponse(msg);
			}
			break;
		case 's':
			++msg;
			if (isspace(*msg)) {
				eatwhitespace(&msg);
				return new SendFile(msg);
			} 
			
				return new UnrecognizedCmd(orig);
}
