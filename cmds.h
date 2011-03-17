/*
 *  cmds.h
 *  macmidish
 *
 *  Created by Matthew Kane on 3/4/11.
 *  Copyright 2011 Matt Kane. All rights reserved.
 *
 */

#ifndef H2P_MMS_CMDS
#define H2P_MMS_CMDS

#include <string>
#include <vector>
#include <deque>

namespace macmidish {
	class PortSelector {
	public:
		PortSelector(int argc, char ** argv) : _argc(argc), _argv(argv), _ready(false) {}
		unsigned int inPort();
		unsigned int outPort();
		void scanSelect(bool inPort = true, bool outPort = true);
	private:
		void parseArgs();
		bool _ready;
		int _argc;
		char **_argv;
		unsigned _inPort, _outPort;
	};
	
	enum ParseMode {
		ASCII,
		HEX,
		DECIMAL,
		OCTAL
	};
		
	class CmdParser {
	public:
		CmdParser(unsigned int inPort, unsigned int outPort):
			_inPort(inPort), _outPort(outPort), _portsOpen(false), _logfile(NULL), _quit(false),
		_parseMode(HEX) { openPorts(); }
		~CmdParser() { delete _logfile; if (_logfilehandle) { fclose(_logfilehandle); } }
		void startRepl();
		void setLogfile(const std::string & f) {delete _logfile; _logfile = new std::string(f);}
		std::string getLogfile();
		bool hasLogfile() { return (_logfile != NULL); }
		void setParseMode(ParseMode mode) { _parseMode = mode; }
		void openlog();
		FILE * getLogFilehandle() { return _logfilehandle; }
		void quit() { _quit = true; }
		void setInPort(unsigned int inPort) { _inPort = inPort; }
		void setOutPort(unsigned int outPort) { _outPort = outPort; }
		void openPorts();
		void closePorts();
	private:
		char * issueprompt();
		void parsecmd(char *);
		bool _portsOpen;
		std::string *_logfile;
		FILE *_logfilehandle;
		bool _quit;
		unsigned int _inPort;
		unsigned int _outPort;
		ParseMode _parseMode;
	};
	
	class Cmd {
	public:
		static Cmd * newCmd(ParseMode, const char *);
		void execute(CmdParser & parser);
		virtual void executeHelper(CmdParser & parser) = 0;
		virtual std::string logmsg() = 0;
		virtual ~Cmd() {}
		// return string to put into readline history
	protected:
		void sendMsg();
		void parseMsg(ParseMode mode, const char *);
		void asHex(const char *);
		void asDecimal(const char *);
		void asOctal(const char *);
		void asAscii(const char *);
		Cmd() : _set(false) {}
		std::vector<unsigned char> _msg;
		bool _set;
	private:
		void parseInts(const char *, int);
	};

	class SetInputMode : public Cmd {
	public:
		virtual void executeHelper(CmdParser & parser);
		virtual std::string logmsg();
		SetInputMode(const char *);
	private:
		void parseCmd(const char *);
		ParseMode _parseMode;
	};

	class SendFile : public Cmd {
	public:
		SendFile(const char *);
		virtual void executeHelper(CmdParser & parser);
		virtual std::string logmsg();
		virtual ~SendFile();
	private:
		char * filename;
	};

	class SetLogfile : public Cmd {
	public:
		SetLogfile(const char *);
		virtual void executeHelper(CmdParser & parser);
		virtual std::string logmsg();
		virtual ~SetLogfile();
	private:
		char * line;
	};
	
	class SendMsg : public Cmd {
	public:
		SendMsg(ParseMode, const char *);
		virtual void executeHelper(CmdParser & parser);
		virtual std::string logmsg();
	protected:
		SendMsg():Cmd(){}
	};
	
	class SendMsgWithResponse : public SendMsg {
	public:
		SendMsgWithResponse(ParseMode, const char *);
		virtual void executeHelper(CmdParser & parser);
	protected:
		SendMsgWithResponse():SendMsg(){}
		void helperHelper();
	private:
		unsigned int timeout;
	};
	
	class SendMsgWithResponseToFile : public SendMsgWithResponse {
	public:
		SendMsgWithResponseToFile(ParseMode, const char *);
		virtual void executeHelper(CmdParser & parser);
	private:
		unsigned int timeout;
		std::string filename;
	};
	
	class Quit : public Cmd {
	public:
		virtual void executeHelper(CmdParser & parser);
		virtual std::string logmsg();
	};
	
	class PortChange : public Cmd {
	public:
		virtual void executeHelper(CmdParser & parser);
		virtual std::string logmsg();
	};
	
	class UnrecognizedCmd : public Cmd {
	public:
		UnrecognizedCmd(const char *);
		virtual std::string logmsg();
		virtual void executeHelper(CmdParser & parser);
	private:
		std::string cmd;
	};
}

#endif //H2P_MMS_CMDS