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

namespace macmidish {
	class PortSelector {
	public:
		PortSelector(int argc, char ** argv) : _argc(argc), _argv(argv), _ready(false) {}
		int inPort();
		int outPort();
	private:
		void scanSelect();
		bool _ready;
		int _argc;
		char **_argv;
	};
		
	
	class CmdParser {
	public:
		CmdParser(unsigned int inPort, unsigned int outPort):
			_inPort(inPort), _outPort(outPort), _portsOpen(false), logfile(NULL) {}
		void startRepl();
		void setLogfile(const std::string &);
	private:
		char * issueprompt();
		void parsecmd(char *);
		bool _portsOpen;
		std::string *logfile;
		unsigned int _inPort;
		unsigned int _outPort;
	};
	
	enum ParseMode {
		ASCII,
		HEX,
		DECIMAL,
		OCTAL
	};
	
	class Cmd {
	public:
		static Cmd * newCmd(const char *);
		std::string execute(unsigned int inPort, unsigned int outPort);
		virtual std::string executeHelper(unsigned int inPort, unsigned int outPort) = 0;
		virtual ~Cmd() {}
		// return string to put into readline history
	private:
		Cmd();
		void asHex(const char *);
		void asDecimal(const char *);
		void asOctal(const char *);
		void asAscii(const char *);
		std::vector<unsigned char> _msg;
	};

	class SetInputMode : public Cmd {
	public:
		virtual std::string execute(unsigned int inPort, unsigned int outPort);
		virtual ~SetInput();
	};

	class SendFile : public Cmd {
	public:
		virtual std::string execute(unsigned int inPort, unsigned int outPort);
		virtual ~SendFile();
	private:
	};

	class SetLogfile : public Cmd {
	public:
		virtual std::string execute(unsigned int inPort, unsigned int outPort);
		virtual ~SetLogfile();
	};
	
	class SendMsg : public Cmd {
	public:
		virtual std::string execute(unsigned int inPort, unsigned int outPort);
		virtual ~SendMsg();
	};
	
	class SendMsgWithResponse : public SendMsg {
	public:
		virtual std::string execute(unsigned int inPort, unsigned int outPort);
		virtual ~SendMsgWithResponse();
	};
	
	class SendMsgWithResponseToFile : public SendMsgWithResponse {
	public:
		virtual std::string execute(unsigned int inPort, unsigned int outPort);
		virtual ~SendMsgWithResponseToFile();
	};
	
	class Quit : public Cmd {
	public:
		virtual std::string execute(unsigned int inPort, unsigned int outPort);
		virtual ~SendMsgWithResponseToFile();
	};
	
	
}

#endif //H2P_MMS_CMDS