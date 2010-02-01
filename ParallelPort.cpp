///	\file
///	\brief Implementation of the Linux parallel port class
///	\author Yi Yao (http://yyao.ca/)
///
///	$LastChangedDate: 2009-11-27 23:18:55 -0500 (Fri, 27 Nov 2009) $
///
///	This library helps you interface and control the parallel port of your
///	computer.
///
///	Add the ParallelPort.h and ParallelPort.cpp file to your project to use this
///	library.
///
///	This library is distributed under the zlib license:
///	\verbatim
///	  Copyright (C) 2000-2009 Yi Yao
///
///	  This software is provided 'as-is', without any express or implied
///	  warranty. In no event will the authors be held liable for any damages
///	  arising from the use of this software.
///
///	  Permission is granted to anyone to use this software for any purpose,
///	  including commercial applications, and to alter it and redistribute it
///	  freely, subject to the following restrictions:
///
///	  1. The origin of this software must not be misrepresented; you must not
///	     claim that you wrote the original software. If you use this software
///	     in a product, an acknowledgment in the product documentation would be
///	     appreciated but is not required.
///	  2. Altered source versions must be plainly marked as such, and must not be
///	     misrepresented as being the original software.
///	  3. This notice may not be removed or altered from any source distribution.
///	\endverbatim

//You can compile documentation for this file using Doxygen:
//http://www.stack.nl/~dimitri/doxygen/

#include "ParallelPort.h"


ParallelPort::ParallelPort() {
	mPortName = "";
	mPortFD = 0;
	mPortOpened = false;
	mDataOut = false;
};


ParallelPort::~ParallelPort() {
	if (mPortOpened) {
		Close();
	};
};


void ParallelPort::Open(const std::string& PortName) volatile throw(ParallelPort_errors) {
	int		PortDataDir;

	//Don't open port if it is already opened
	if (mPortOpened) {
		throw ParallelPort_errors(Opened);
	};

	//Open device (e.g. /dev/parport0)
	mPortFD = open(PortName.c_str(), O_RDWR);
	if (mPortFD < 0) {
		mPortFD = 0;
		throw ParallelPort_errors(Opening);
	};

	//Claim control over parallel port to deny other processes form access
	if (ioctl(mPortFD, PPCLAIM) < 0) {
		close(mPortFD);
		mPortFD = 0;
		throw ParallelPort_errors(Perm);
	};

	//Set member variables
	const_cast<std::string&>(mPortName) = PortName;
	mPortOpened = true;

	//Save existing R/W registers so that we can restore them later
	mPortSavedRegs[0] = Data();
	mPortSavedRegs[1] = Ctrl();

	//Set data register in known configuration
	PortDataDir = 0;
	ioctl(mPortFD, PPDATADIR, &PortDataDir);
	mDataOut = true;
};


void ParallelPort::Close(void) volatile throw(ParallelPort_errors) {
	//Don't close port if its not opened
	if (!mPortOpened) {
		throw ParallelPort_errors(Closed);
	};

	//Restore R/W registers to what they were before we came along
	Data(const_cast<unsigned char&>(mPortSavedRegs[0]));
	Ctrl(const_cast<unsigned char&>(mPortSavedRegs[1]));

	//Release exlusive use of parallel port
	if (ioctl(mPortFD, PPRELEASE) < 0) {
		throw ParallelPort_errors(Perm);
	};

	//Close parallel port device (file)
	if (close(mPortFD) < 0) {
		throw ParallelPort_errors(Closing);
	};

	//Set member variables to failsafe values
	const_cast<std::string&>(mPortName) = "";
	mPortFD = 0;
	mPortOpened = false;
	mDataOut = false;
};


bool ParallelPort::IsOpened(void) volatile {
	return mPortOpened;
};


bool ParallelPort::IsClosed(void) volatile {
	return !mPortOpened;
};


const std::string& ParallelPort::PortName(void) volatile {
	//Don't close port if its not opened
	if (!mPortOpened) {
		throw ParallelPort_errors(Closed);
	};

	return const_cast<std::string&>(mPortName);
};


const unsigned char ParallelPort::Data(void) volatile throw(ParallelPort_errors) {
	char	c;

	//Don't do anything if port is not opened
	if (!mPortOpened) {
		throw ParallelPort_errors(Closed);
	};

	if (ioctl(mPortFD, PPRDATA, &c)) {
		throw ParallelPort_errors(Read);
	};

	return c;
};


void ParallelPort::Data(const unsigned char& c) volatile throw(ParallelPort_errors) {
	//Don't do anything if port is not opened
	if (!mPortOpened) {
		throw ParallelPort_errors(Closed);
	};

	if (ioctl(mPortFD, PPWDATA, &c)) {
		throw ParallelPort_errors(Write);
	};
};


bool ParallelPort::DataOut(void) volatile throw(ParallelPort_errors) {
	//Don't do anything if port is not opened
	if (!mPortOpened) {
		throw ParallelPort_errors(Closed);
	};

	return mDataOut;
};


void ParallelPort::DataOut(const bool Out) volatile throw(ParallelPort_errors) {
	int		PortDataDir;

	//Don't do anything if port is not opened
	if (!mPortOpened) {
		throw ParallelPort_errors(Closed);
	};

	if (Out) {
		PortDataDir = 0;
	} else {
		PortDataDir = 1;
	};

	if (ioctl(mPortFD, PPDATADIR, &PortDataDir) < 0) {
		throw ParallelPort_errors(Perm);
	};

	mDataOut = Out;
};


const unsigned char ParallelPort::Stat(void) volatile throw(ParallelPort_errors) {
	char	c;

	//Don't do anything if port is not opened
	if (!mPortOpened) {
		throw ParallelPort_errors(Closed);
	};

	if (ioctl(mPortFD, PPRSTATUS, &c)) {
		throw ParallelPort_errors(Read);
	};

	return c;
};


const unsigned char ParallelPort::Ctrl(void) volatile throw(ParallelPort_errors) {
	char	c;

	//Don't do anything if port is not opened
	if (!mPortOpened) {
		throw ParallelPort_errors(Closed);
	};

	if (ioctl(mPortFD, PPRCONTROL, &c)) {
		throw ParallelPort_errors(Read);
	};

	return c;
};


void ParallelPort::Ctrl(const unsigned char& c) volatile throw(ParallelPort_errors) {
	//Don't do anything if port is not opened
	if (!mPortOpened) {
		throw ParallelPort_errors(Closed);
	};

	if (ioctl(mPortFD, PPWCONTROL, &c)) {
		throw ParallelPort_errors(Write);
	};
};
