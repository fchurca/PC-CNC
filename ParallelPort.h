///	\file
///	\brief Declaration of the Linux parallel port class
///	\author Yi Yao (http://yyao.ca/)
///
///	$LastChangedDate: 2009-11-27 23:36:01 -0500 (Fri, 27 Nov 2009) $
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

#ifndef __PARALLELPORT_H
#define __PARALLELPORT_H

#include <string>

#include <fcntl.h>
#include <unistd.h>
#include <linux/parport.h>
#include <linux/ppdev.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>


///	\brief Exceptions which can be thrown by the vect class
///
///	Exceptions are thrown during runtime if a member function of the vect
///	class encounters an error which cannot be automatically corrected.
enum ParallelPort_errors {
	None = 0,							///< No error, should never be thrown
	Opening,							///< Cannot open port (check errno)
	Closing,							///< Cannot close port (check errno)
	Opened,								///< Port already opened
	Closed,								///< Port not opened
	Perm,								///< Cannot claim control over port (check errno)
	Read,								///< Error reading port (check errno)
	Write,								///< Error writing port (check errno)
	Unknown								///< Corrupted internal state, should never happen
};


///	\brief Parallel port class
///
///	The ParallelPort class encapsulates access to a parallel port and its
///	registers and pins.
///
///	Example usage:
///	\code
///		ParallelPort	IOPort;
///
///		IOPort.Open("/dev/parport0");
///
///		//Toggle parallel port data pins
///		for (;;) {
///			IOPort.Data(~IOPort.Data());
///		};
///	\endcode
///
///	Port access is acccomplished through user mode ioctl calls. The data, status
///	and control registers are made available to the calling functions, but
///	additional registers such as the extended control register (ECR) are not
///	available.
class ParallelPort {
private:
	std::string		mPortName;			///< Port name (file name) of opened port
	int		mPortFD;					///< Port file descriptor (for system calls)
	bool	mPortOpened;				///< True if port has been successfully opened and permissions are granted
	unsigned char	mPortSavedRegs[2];	///< Saved registers which will be restored when port is closed
	bool	mDataOut;

	///	\brief Copy assignment prevention
	///
	///	Parallel port objects should not be copied to prevent copies from
	///	clobbering each other
	ParallelPort(const ParallelPort& port);

	///	\brief Copy assignment prevention
	///
	///	Parallel port objects should not be copied to prevent copies from
	///	clobbering each other
	ParallelPort operator = (const ParallelPort& port);

public:
	///	\brief Default constructor
	///
	///	Initializes internal data structures.
	ParallelPort();

	///	\brief Default destructor
	///
	///	The destructor will close any port that is left open during the scope of
	///	the ParallelPort variable.
	~ParallelPort();

	///	\brief Opens specified parallel port for access
	///
	///	Opens specified file as a parallel port and claims access for direct IO.
	///
	///	@param[in]		PortName		Name of port to open (file name)
	///	@exception		ParallelPort_errors		Can throw any error caused by
	///	unsuccessful OS operation
	void	Open(const std::string& PortName) volatile throw(ParallelPort_errors);

	///	\brief Closes opened parallel port
	///
	///	Releases exclusive IO handle and closes parallel port so that other
	///	processes can have access to it.
	///
	///	@exception		ParallelPort_errors		Can throw any error caused by
	///	unsuccessful OS operation
	void	Close(void) volatile throw(ParallelPort_errors);

	///	\brief Check to see if port is opened
	///
	///	This function will always return the opposite of IsClosed.
	///
	///	@return							true if port is opened
	bool	IsOpened() volatile;

	///	\brief Check to see if port is closed
	///
	///	This function will always return the opposite of IsOpened.
	///
	///	@return							true if port is closed
	bool	IsClosed() volatile;

	///	\brief Gets name of port that is opened
	///
	///	This function returns the file name of the port that the object has
	///	opened.
	///
	///	@return							Port name
	const std::string&		PortName(void) volatile;

	///	\brief Reads data register
	///
	///	This function reads input from the data pins and the data register.
	///
	///	@return							Value read from data register
	///	@exception		ParallelPort_errors		Can throw any error caused by
	///	unsuccessful OS operation
	const unsigned char		Data(void) volatile throw(ParallelPort_errors);

	///	\brief Sets data register
	///
	///	This function sets the data register and its associated pins.
	///
	///	@param[in]		c				Value to set data register to
	///	@exception		ParallelPort_errors		Can throw any error caused by
	///	unsuccessful OS operation
	void	Data(const unsigned char& c) volatile throw(ParallelPort_errors);

	///	\brief Gets data pin direction
	///
	///	Finds out if data pins are set for output.
	///
	///	@return							True if data pins are set for output
	///	@exception		ParallelPort_errors		Can throw any error caused by
	///	unsuccessful OS operation
	bool	DataOut(void) volatile throw(ParallelPort_errors);

	///	\brief Sets data pin direction
	///
	///	Configures the parallel port for output on datapins.
	///
	///	@param[in]		Out				Set to true to output to data pins
	///	@exception		ParallelPort_errors		Can throw any error caused by
	///	unsuccessful OS operation
	void	DataOut(const bool Out) volatile throw(ParallelPort_errors);

	///	\brief Reads status register
	///
	///	This function reads input from the status pins and the status register.
	///	Note that some of these pins are inverted by the parallel port hardware.
	/// You will have to invert these pins yourself to obtain the positive logic
	///	signals.
	///
	///	@return							Value read from status register
	///	@exception		ParallelPort_errors		Can throw any error caused by
	///	unsuccessful OS operation
	const unsigned char		Stat(void) volatile throw(ParallelPort_errors);

	///	\brief Reads control register
	///
	///	This function returns the contents of the control register. It appears
	///	that the Linux ppdev API simply caches and returns the last written
	///	value. The documentation justifies this:
	///
	/// 'This doesn't actually touch the hardware; the last value written is
	///	remembered in software. This is because some parallel port hardware does
	///	not offer read access to the control register.'
	///
	///	@return							Value read from control register
	///	@exception		ParallelPort_errors		Can throw any error caused by
	///	unsuccessful OS operation
	const unsigned char		Ctrl(void) volatile throw(ParallelPort_errors);

	///	\brief Sets control register
	///
	///	This function sets the control register and the control pins. Note that
	/// some of these pins are inverted by the parallel port hardware. You will
	///	have to invert these pins yourself to compensate for the inversion.
	///
	/// Be careful when writing to C6. It also controls the direction of the
	///	data pins. Writing to it can cause the data registers to inadvertantly
	///	switch direction. If you do intend to change the direction of the data
	///	pins, it is much more advisable to use the DataOut functions.
	///
	///	@param[in]		c				Value to set control register to
	///	@exception		ParallelPort_errors		Can throw any error caused by
	///	unsuccessful OS operation
	void	Ctrl(const unsigned char& c) volatile throw(ParallelPort_errors);
};


#endif
