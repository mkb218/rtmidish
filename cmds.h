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
		int inPort();
		int outPort();
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
			_parseMode(HEX) {}
		void startRepl();
		void setLogfile(const std::string &);
		friend class Cmd;
	private:
		void openlog();
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
		static Cmd * newCmd(const char *);
		std::string execute(CmdParser & parser);
		virtual std::string executeHelper(CmdParser & parser) = 0;
		virtual std::string logmsg() = 0;
		virtual ~Cmd() {}
		// return string to put into readline history
	protected:
		void asHex(const char *);
		void asDecimal(const char *);
		void asOctal(const char *);
		void asAscii(const char *);
		std::deque<unsigned char> _msg;
		bool _set;
	private:
		void parseInts(const char *, int);
	};

	class SetInputMode : public Cmd {
	public:
		virtual std::string executeHelper(CmdParser & parser);
		virtual std::string logmsg();
		SetInputMode(const char *);
	private:
		ParseMode _parseMode;
	};

	class SendFile : public Cmd {
	public:
		SendFile(const char *);
		virtual std::string executeHelper(CmdParser & parser);
		virtual std::string logmsg();
		virtual ~SendFile();
	private:
		const char * filename;
	};

	class SetLogfile : public Cmd {
	public:
		SetLogfile(const char *);
		virtual std::string executeHelper(CmdParser & parser);
		virtual std::string logmsg();
		virtual ~SetLogfile();
	private:
		const char * line;
	};
	
	class SendMsg : public Cmd {
	public:
		SendMsg(const char *);
		virtual std::string executeHelper(CmdParser & parser);
	};
	
	class SendMsgWithResponse : public SendMsg {
	public:
		SendMsgWithResponse(const char *);
		virtual std::string executeHelper(CmdParser & parser);
	private:
		unsigned int timeout;
	};
	
	class SendMsgWithResponseToFile : public SendMsgWithResponse {
	public:
		SendMsgWithResponse(const char *);
		virtual std::string executeHelper(CmdParser & parser);
		virtual ~SendMsgWithResponseToFile();
	private:
		unsigned int timeout;
		const char * filename;
	};
	
	class Quit : public Cmd {
	public:
		virtual std::string executeHelper(CmdParser & parser);
	};
	
	class PortChange : public Cmd {
	public:
		virtual std::string executeHelper(CmdParser & parser);
	};
	
	class UnrecognizedCmd : public Cmd {
	public:
		UnrecognizedCmd(const char *);
		virtual std::string executeHelper(CmdParser & parser);
	private:
		const char* cmd;
	};
}

#endif //H2P_MMS_CMDS