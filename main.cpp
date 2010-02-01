#include <iostream>
#include <fstream>
#include "ParallelPort.h"
#include "cnc.h"

using namespace std;

int main (int argc, char * argv[]){
	string conffile = "conf";
	ifstream infile(conffile.c_str());

	string portname = "/dev/parport0";
	ParallelPort IOPort;

	Machine m;

	m.setPort(&IOPort);
	infile >> m;
	infile.close();

	try{
		IOPort.Open(portname);
		cout << m;
		m.Zero();
		for(int i = 0; i < m.steppers.size(); i++)
			m.steppers[i].Step(-4);
		for(int i = 0; i < m.onoffs.size(); i++)
			for(int j = 4; j; j--)
				m.onoffs[i].set(!m.onoffs[i].get());
	}catch (ParallelPort_errors){
		cerr << "Error on port " << portname << endl;
	}
	return 0;
}
