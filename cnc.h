#ifndef ___CNC_H__
#include "ParallelPort.h"
#include <iostream>
#include <string>
#include <deque>

#define second (1000000)
#define minute (60 * second)

const unsigned graycode2[4] = {0, 1, 3, 2};

class oDevice{
protected:
	unsigned short _offset;
	ParallelPort * _port;
public:
	std::string Name;
	ParallelPort * setPort(ParallelPort * port);
	ParallelPort * getPort();
};

class Onoff : public oDevice{
protected:
	bool _state;
	unsigned long _delay;
	long double _speed;
public:
	Onoff();
	void Nudge(bool state);
	bool set(bool state);
	bool get();
	unsigned long setDelay(unsigned long delay);
	unsigned long getDelay();
	long double setSpeed(long double speed);
	long double getSpeed();
	friend std::istream& operator >> (std::istream & in, Onoff& d);
	friend std::ostream& operator << (std::ostream & outfile, Onoff & d);
};

class Stepper : public oDevice{
protected:
	unsigned _steps;
	unsigned long _delay;
	long double _speed;
	unsigned _state : 2;
	long _pos;
public:
	Stepper();
	Stepper(unsigned steps, unsigned short offset, ParallelPort * port);
	long setPos(long pos);
	long getPos();
	unsigned long setDelay(unsigned long delay);
	unsigned long getDelay();
	long double setSpeed(long double speed);
	long double getSpeed();
	void Nudge();
	void Push();
	void Step(int steps);
	void goTo(long pos);
	friend std::istream& operator >> (std::istream & in, Stepper & d);
	friend std::ostream& operator << (std::ostream & outfile, Stepper & d);
};

class Machine{
protected:
	ParallelPort * _port;
public:
	std::string Name;
	std::deque<Stepper> steppers;
	std::deque<Onoff> onoffs;
	ParallelPort * setPort(ParallelPort * port);
	ParallelPort * getPort();
	void Zero();
	friend std::istream& operator >> (std::istream & infile, Machine & d);
	friend std::ostream& operator << (std::ostream & outfile, Machine & d);
};

#endif
