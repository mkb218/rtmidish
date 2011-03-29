/*
 *  proxy.h
 *  rtmidish
 *
 *  Created by Matthew Kane on 3/29/11.
 *  Copyright 2011 Matthew Kane. All rights reserved.
 *
 */

#ifndef RTMIDISH_PROXY
#define RTMIDISH_PROXY
#include <string>
#import <CoreFoundation/CoreFoundation.h>

namespace rtmidish {
	class MidiProxyImpl {
	public:
		virtual unsigned int getPortCount() = 0;
		virtual std::string getPortName() = 0;
	};
	class MidiInProxyImpl : public MidiProxyImpl {
	};
	
	class MidiOutProxyImpl : public MidiProxyImpl {
	};
	
	class MidiProxy {
	protected:
		virtual MidiProxyImpl *getImpl() = 0;
	public:
		unsigned int getPortCount() { return getImpl()->getPortCount(); }
		std::string getPortName(unsigned int p) { return getImpl()->getPortName(); }
	};
	
	class MidiInProxy : public MidiProxy {
	public:
		MidiInProxy();
	protected:
		virtual MidiProxyImpl * getImpl() { return impl_; }
	private:
		MidiInProxyImpl *impl_;
	};
	
	class MidiOutProxy : public MidiProxy {
	public:
		MidiOutProxy();
	protected:
		virtual MidiProxyImpl * getImpl() { return impl_; }
	private:
		MidiOutProxyImpl *impl_;
	};
}

#endif