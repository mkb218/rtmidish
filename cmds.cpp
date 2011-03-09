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
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <cstring>
#include <cctype>
#include <cassert>

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

void CmdParser::openlog() {
	if (_logfile == NULL || _logfilehandle != NULL) {
		return;
	}
	
	_logfilehandle = fopen(_logfile->c_str(), "a");
	if (!_logfilehandle) {
		std::cerr << "couldn't open logfile: " << errno << std::endl;
		throw std::exception();
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
	Cmd * c = Cmd::newCmd(cmd);
	c->execute(*this);
	delete c;
}

Cmd * Cmd::newCmd(const char * msg) {
	const char * orig = msg;
	eatwhitespace(&msg);
	switch (*(msg++)) {
		case 'i':
			if (isspace(*msg)) {
				eatwhitespace(&msg);
				return new SetInputMode(msg);
			} else if (*msg == 'o') { 
				eatwhitespace(&msg);
				return new SendMsgWithResponse(msg);
			}
			break;
		case 's':
			if (isspace(*msg)) {
				eatwhitespace(&msg);
				return new SendFile(msg);
			} 
			break;
		case 'l':
			if (isspace(*msg)) {
				eatwhitespace(&msg);
				return new SetLogfile(msg);
			}
			break;
		case 'o':
			if (isspace(*msg)) {
				eatwhitespace(&msg);
				return new SendMsg(msg);
			}
			break;
		case 'p':
			if (isspace(*msg)) {
				eatwhitespace(&msg);
				return new PortChange;
			}
			break;
		case 'f':
			if (*msg == 'o') {
				++msg;
				eatwhitespace(&msg);
				return new SendMsgWithResponseToFile(msg);
			}
			break;
		case 'q':
			if (isspace(*msg)) {
				eatwhitespace(&msg);
				return new Quit;
			}
	}
	return new UnrecognizedCmd(orig);
}

std::string Cmd::execute(CmdParser & parser) {
	executeHelper();
	if (parser._logfile != NULL) {
		parser.openlog();
		fwrite(this->logmsg().c_str(), this->logmsg().size()+1, 1, parser._logfilehandle);
	}
}

void Cmd::asHex(const char *str) {
	parseInts(str, 16);
}

void Cmd::asOctal(const char *str) {
	parseInts(str, 8);
}

void Cmd::asDecimal(const char *str) {
	parseInts(str, 10);
}

void Cmd::asAscii(const char * str) {
	while (*str) {
		_msg.push_back(*str);
		++str;
	}
}

void Cmd::parseInts(const char *str, int base) {
	char * end = NULL;
	while (*str) {
		long answer = strtol(str, &end, base);
		if (answer & 0x7f != answer) { // too big
			std::cout << "bad input string! " << answer << " was not <= 127 and >= 0." << std::endl;
			return;
		} else if (answer == 0 && errno == EINVAL) {
			std::cout << "bad input string! " << str << " had a bad input character!" << std::endl;
			return;
		}
		_msg.push_back(static_cast<unsigned char>(answer));
		if (!isspace(*end)) {
			std::cout << "bad input string! skipping " << end << std::endl;
			return;
		}	
		str = end;
	}
}

void SetInputMode::SetInputMode(const char * str) : _set(false) {
	eatwhitespace(&str);
	parseCmd(str);
}

void SetInputMode::parseCmd(const char * str) {
	switch (str[0]) {
		case 'h':
			mode = HEX;
			_set = (str[1] == 0);
			break;
		case 'i':
			mode = DECIMAL;
			_set = (str[1] == 0);
			break;
		case 'o':
			mode = OCTAL;
			_set = (str[1] == 0);
			break;
		case 'a':
			mode = ASCII;
			_set = (str[1] == 0);
			break;
	}
	return mode;
}	
void SetInputMode::executeHelper(CmdParser & parser) {
	if (!_set) {
		char * msg = readline("Mode? h - hex, i - ints, o - octal, a - ascii >");
		
	}
	parser.
}