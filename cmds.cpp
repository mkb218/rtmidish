/*
 *  cmds.cpp
 *  macmidish
 *
 *  Created by Matthew Kane on 3/4/11.
 *  Copyright 2011 Matthew Kane. All rights reserved.
 *
 */

#include "cmds.h"
#include "RtMidi.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <cstring>
#include <cctype>
#include <cassert>
#include <readline/readline.h>
#include <errno.h>
#include <iomanip>
#include "proxy.h"

using namespace rtmidish;

static MidiOutProxy midiout;
static MidiInProxy midiin;

static void eatwhitespace(const char ** msg) {
	while (isspace(**msg)) {
		++(*msg);
	}
}

static void findunescapedws(const char ** msg) {
	bool lastbs = false;
	while (!isspace(**msg) || lastbs) {
		lastbs = (**msg == '\\');
		++(*msg);
	}
}

void PortSelector::scanSelect(bool inPort, bool outPort) {
	unsigned int outPorts = midiout.getPortCount();
	unsigned int inPorts = midiin.getPortCount();
	unsigned int max = std::max(inPorts, outPorts);
	std::cout << "Please select the MIDI port you want to communicate with:" << std::endl;
	for (unsigned int i = 0; i < max; ++i) {
		std::cout << i;
		std::cout << ".) ";
		std::cout << ((i<inPorts)?(midiin.getPortName(i)):("(output only)"));
		std::cout << "/" ;
		std::cout << ((i<outPorts)?(midiout.getPortName(i)):("(input only)"));
		std::cout << std::endl;
	}
	
	char *resp = NULL;
	std::istringstream is;
	if (inPort) {
		do {
			resp = readline("input > ");
		} while (!resp || !(*resp));
		
		is.str(resp);
		is >> _inPort;
		free(resp);
	}
	
	if (outPort) {
		resp = NULL;
		do {
			resp = readline("output >");
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
			if (strncmp(_argv[i], "-in", 4) == 0 && i + 1 < _argc) {
				++i;
				inPort = false;
				_inPort = strtol(_argv[i], NULL, 10);
			} else if (strncmp(_argv[i], "-out", 4) == 0 && i + 1 < _argc) {
				++i;
				outPort = false;
				_outPort = strtol(_argv[i], NULL, 10);
			}
		}
	}
	if (inPort || outPort) {
		scanSelect(inPort, outPort);
	}
	_ready = true;
}

unsigned int PortSelector::inPort() {
	if (!_ready) {
		parseArgs();
	}
	return _inPort;
}

unsigned int PortSelector::outPort() {
	if (!_ready) {
		parseArgs();
	}
	return _outPort;
}

void CmdParser::openPorts() {
	midiin.openPort(_inPort);
	midiout.openPort(_outPort);
}

void CmdParser::closePorts() {
	// noop for now
}

void CmdParser::startRepl() {
	while (!_quit) {
		char * cmd = issueprompt();
		if (cmd) {
			if (*cmd) {
				parsecmd(cmd);
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
		resp = readline("> ");
	} while (!resp || !(*resp));
	return resp;
}

void CmdParser::parsecmd(char * cmd) {
	Cmd * c = Cmd::newCmd(_parseMode, cmd);
	c->execute(*this);
	delete c;
}

void Cmd::sendMsg() {
	midiout.sendMessage(&_msg);
}

void Cmd::parseMsg(ParseMode mode, const char * msg) {
	switch (mode) {
		case ASCII:
			asAscii(msg);
			break;
		case HEX:
			asHex(msg);
			break;
		case DECIMAL:
			asDecimal(msg);
			break;
		case OCTAL:
			asOctal(msg);
			break;
	}
}
	

Cmd * Cmd::newCmd(ParseMode mode, const char * msg) {
	const char * orig = msg;
	eatwhitespace(&msg);
	switch (*(msg++)) {
		case 'i':
			if (isspace(*msg)) {
				eatwhitespace(&msg);
				return new SetInputMode(msg);
			} else if (*msg == 'o') { 
				eatwhitespace(&msg);
				return new SendMsgWithResponse(mode, msg);
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
				return new SendMsg(mode, msg);
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
				return new SendMsgWithResponseToFile(mode, msg);
			}
			break;
		case 'q':
			if (isspace(*msg)) {
				eatwhitespace(&msg);
				return new Quit;
			}
		case EOF:
			return new Quit;
	}
	return new UnrecognizedCmd(orig);
}

void Cmd::execute(CmdParser & parser) {
	executeHelper(parser);
	if (parser.hasLogfile()) {
		parser.openlog();
		fwrite(this->logmsg().c_str(), this->logmsg().size()+1, 1, parser.getLogFilehandle());
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

SetInputMode::SetInputMode(const char * str) {
	_set = false;
	eatwhitespace(&str);
	parseCmd(str);
}

void SetInputMode::parseCmd(const char * str) {
	switch (str[0]) {
		case 'h':
			_parseMode = HEX;
			_set = (str[1] == 0);
			break;
		case 'i':
			_parseMode = DECIMAL;
			_set = (str[1] == 0);
			break;
		case 'o':
			_parseMode = OCTAL;
			_set = (str[1] == 0);
			break;
		case 'a':
			_parseMode = ASCII;
			_set = (str[1] == 0);
			break;
	}
}	

void SetInputMode::executeHelper(CmdParser & parser) {
	while (!_set) {
		char * msg = readline("Mode? h - hex, i - ints, o - octal, a - ascii > ");
		parseCmd(msg);
	}
	parser.setParseMode(_parseMode);
}

std::string SetInputMode::logmsg() {
	const char *names[] = {"ascii", "hex", "decimal", "octal"};
	return (std::string("Set input mode to ") + names[(int)_parseMode] + ".");
}

SendFile::SendFile(const char * str):filename(NULL) {
	size_t len = strlen(str) + 1;
	filename = new char[len];
	strncpy(filename, str, len);
}
						
SendFile::~SendFile() {
	delete [] filename;
}				

void SendFile::executeHelper(CmdParser & parser) {
	FILE * in = fopen(filename, "r");
	if (in == NULL) {
		throw std::exception();
	}
	
	size_t read = 0;
	char buffer[1024];
	while (!feof(in)) {
		read = fread(buffer, 1, 1024, in);
		if (read < 1024) {
			if (ferror(in)) {
				fclose(in);
				throw std::exception();
			}
		}
		for ( int i = 0; i < 1024; ++i) {
			_msg.push_back(buffer[i]);
		}
	}
}

std::string SendFile::logmsg() {
	return (std::string("Sent file to device: ") + filename);
}

SetLogfile::SetLogfile(const char * msg) : line(NULL) {
	size_t len = strlen(msg) + 1;
	line = new char[len];
	strncpy(line, msg, len);
}

SetLogfile::~SetLogfile(){
	delete [] line;
}

void SetLogfile::executeHelper(CmdParser & parser) {
	parser.setLogfile(std::string(line));
}

std::string SetLogfile::logmsg() {
	return (std::string("Set log file to ") + line);
}

SendMsg::SendMsg(ParseMode mode, const char * msg) {
	parseMsg(mode, msg);
}

void SendMsg::executeHelper(CmdParser & parser) {
	sendMsg();
}

std::string SendMsg::logmsg() {
	return std::string("sent msg");
}

SendMsgWithResponse::SendMsgWithResponse(ParseMode mode, const char * msg) {
	char *end = NULL;
	timeout = strtol(msg, &end, 10);
	eatwhitespace((const char **)&end);
	parseMsg(mode, end);
}

void SendMsgWithResponse::helperHelper() {
	sendMsg();
	_msg.erase(_msg.begin(), _msg.end());
	static bool ready = false;
	if (!ready) {
		midiin.ignoreTypes(false, false, false);
	}
	midiin.getMessage(&_msg);
}

void SendMsgWithResponse::executeHelper(CmdParser & parser) {
	helperHelper();
	size_t count = 0;
	for (std::vector<unsigned char>::iterator it = _msg.begin(); it < _msg.end(); ++it) {
		std::cout << std::hex << std::setw(2) << *it;
		if (++count >= 8) {
			std::cout << std::endl;
			count = 0;
		} else {
			std::cout << " ";
		}
	}
}

SendMsgWithResponseToFile::SendMsgWithResponseToFile(ParseMode mode, const char * msg):
	SendMsgWithResponse() {
	char *end = NULL;
	timeout = strtol(msg, &end, 10);
	eatwhitespace((const char **)&end);
	const char **filenameend = (const char **)&end;
	findunescapedws(filenameend);
	filename = std::string(end, *filenameend-end);
}

void SendMsgWithResponseToFile::executeHelper(CmdParser & parser) {
	helperHelper();
	FILE *out = fopen(filename.c_str(), "w");
	std::vector<unsigned char>::iterator it = _msg.begin();
	fwrite(&(*it), 1, _msg.size(), out);
	fclose(out);
}

void Quit::executeHelper(CmdParser & parser) {
	parser.quit();
}

std::string Quit::logmsg() {
	return "quitting";
}

std::string PortChange::logmsg() {
	return "changed ports";
}

void PortChange::executeHelper(CmdParser & parser) {
	PortSelector ps(0, NULL);
	parser.closePorts();
	parser.setInPort(ps.inPort());
	parser.setOutPort(ps.outPort());
	parser.openPorts();
}

UnrecognizedCmd::UnrecognizedCmd(const char * msg):cmd(msg) {
}

std::string UnrecognizedCmd::logmsg() {
	return std::string("Unrecognized command: ") + cmd;
}

void UnrecognizedCmd::executeHelper(CmdParser & parser) {
	std::cout << logmsg() << std::endl;
}