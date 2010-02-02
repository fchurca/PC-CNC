#include "cnc.h"

using namespace std;

ParallelPort * oDevice::setPort(ParallelPort * port){
	return _port = port;
}
ParallelPort * oDevice::getPort(){
	return _port;
}

Onoff::Onoff(){
	_state = false;
	_delay = 0;
	_port = NULL;
	_offset = 0x10;
}
void Onoff::Nudge(bool state){
	_port->Data((_port->Data() & (~(1 << _offset))) | ((_state = state) << _offset));
	cout << Name << " (" << (int) _offset << "): " << (_state ? "on" : "off") << ' ' << endl;	
}
bool Onoff::set(bool state){
	Nudge(state);
	usleep(_delay);
	return _state;
}
bool Onoff::get(){
	return _state;
}
unsigned long Onoff::setDelay(unsigned long delay){
	_speed = minute / delay;
	return _delay = delay;
}
unsigned long Onoff::getDelay(){
	return _delay;
}
long double Onoff::setSpeed(long double speed){
	if (speed > 0)
		_delay = minute / (_speed = speed);
	else
		_delay = _speed = 0;
	return _speed;
}
long double Onoff::getSpeed(){
	return _speed;
}
istream& operator >> (std::istream& in, Onoff& d){
	in >> d.Name >> d._offset;
	for (int i = 0; i < d.Name.length(); i++)
		if (d.Name[i] == '_')
			if (d.Name[i+1] == '_'){
				d.Name.erase(i + 1, 1);
			}else
				d.Name[i] = ' ';
	return in;
}
ostream& operator << (std::ostream & outfile, Onoff & d){
	outfile << d.Name << ':' << endl;
	outfile << "Pin: " << d._offset << endl;
	outfile << "Speed: " << d._speed << " toggles/minute" << endl;
	outfile << "State: " << (d._state ? "on" : "off") << endl;	
	return outfile;
}

Stepper::Stepper(){
	_steps = 0;
	_state = 0;
	_delay = 0;
	_pos = 0;
	_port = NULL;
	_offset = 0x10;
}
Stepper::Stepper(unsigned steps, unsigned short offset, ParallelPort * port){
	_steps = steps;
	_state = 0;
	_delay =0;
	_pos = 0;
	_port = port;
	if (offset > 6){
		cerr << "Bad offset" << endl;
		_offset = 0x10;
	}else
		_offset = offset;
	if ((steps < 4) or (steps % 4)){
		_steps = 4;
		cerr << "Bad stepper amount: " << _steps << endl;
	}
}
long Stepper::setPos(long pos){
	return _pos = pos;
}
long Stepper::getPos(){
	return _pos;
}
unsigned long Stepper::setDelay(unsigned long delay){
	_speed = minute / delay * _steps;
	return _delay = delay;
}
unsigned long Stepper::getDelay(){
	return _delay;
}
long double Stepper::setSpeed(long double speed){
	if (speed > 0)
		_delay = minute / ((_speed = speed) * _steps);
	else
		_delay = _speed = 0;
	return _speed;
}
long double Stepper::getSpeed(){
	return _speed;
}
void Stepper::Nudge(){
	_port->Data((_port->Data() & (~(3 << _offset))) | (_state << _offset));
	cout << Name << " (" << (int) _offset << ',' << (int) _offset + 1 << "): " <<_state << ' ' << _pos << ' ' << endl;
}
void Stepper::Push(){
	Nudge();
	usleep(_delay);
}
void Stepper::Step(int steps){
	char sign = steps < 0 ? -1 : 1;
	steps *= sign;
	while(steps-- > 0){
		_pos += sign;
		_state = sign > 0 ? graycode2[(_pos) % 4] : graycode2[3 - (3 - _pos) % 4];
		Push();
	}
}
void Stepper::goTo(long pos){
	Step(pos - _pos);
}
istream& operator >> (std::istream& in, Stepper& d){
	in >> d.Name >> d._offset >> d._steps >> d.Unit;
	for (int i = 0; i < d.Name.length(); i++)
		if (d.Name[i] == '_')
			if (d.Name[i+1] == '_'){
				d.Name.erase(i + 1, 1);
			}else
				d.Name[i] = ' ';
	return in;
}
ostream& operator << (std::ostream & outfile, Stepper & d){
	outfile << d.Name << ':' << endl;
	outfile << "Pins: " << d._offset << ',' << d._offset + 1 << endl;
	outfile << "Steps: " << d._steps << " step/" << d.Unit << endl;
	outfile << "Speed: " << d._speed << ' ' << d.Unit << "/minute" << endl;
	outfile << "Position: " << d._pos << " step" << endl;
	return outfile;
}

ParallelPort * Machine::setPort(ParallelPort * port){
	_port = port;
	for(int i = 0; i < steppers.size(); i++)
		steppers[i].setPort(port);
	for(int i = 0; i < onoffs.size(); i++)
		onoffs[i].setPort(port);
}
ParallelPort * Machine::getPort(){
	return _port;
}
void Machine::Zero(){
	_port->Data(0);
	for(int i = 0; i < steppers.size(); i++){
		steppers[i].setPos(0);
		steppers[i].Push();
	}
	for(int i = 0; i < onoffs.size(); i++){
		onoffs[i].set(false);
	}
}
istream& operator >> (istream & infile, Machine& d){
	string type;
	Stepper Sdump;
	Onoff Odump;
	infile >> d.Name;
	while (!infile.eof()){
		infile >> type;
		if(infile.good()){
			if (type == "Stepper"){
				infile >> Sdump;
				Sdump.setPort(d._port);
				Sdump.setSpeed(1);
				d.steppers.push_back(Sdump);
			}else if (type == "Onoff"){
				infile >> Odump;
				Odump.setPort(d._port);
				Odump.setSpeed(60);
				d.onoffs.push_back(Odump);
			}
		}
	}
	return infile;
}
ostream& operator << (ostream & outfile, Machine & d){
	outfile << d.Name << endl;
	outfile << "Using ";
	if (d._port->IsOpened())
		outfile << "open port: " << d._port->PortName() << endl;
	else
		outfile << "closed port";
	outfile << "Actuators: " << endl;
	outfile << d.steppers.size() << " Stepper motor" << ((d.steppers.size() != 1) ? "s" : "") << (d.steppers.size() ? ":" : "") << endl;
	for(int i = 0; i < d.steppers.size(); i++)
		cout << d.steppers[i];
	outfile << d.onoffs.size() << " On/off device" << ((d.onoffs.size() != 1) ? "s" : "") << (d.steppers.size() ? ":" : "") << endl;
	for(int i = 0; i < d.onoffs.size(); i++)
		cout << d.onoffs[i];
	return outfile;
}
