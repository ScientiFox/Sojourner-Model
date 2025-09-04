#ifndef pingh
#define pingh

#include "Arduino.h"

class Ping{
	public:
		Ping(int pin);
		long getUltrasonic();
		int getInches();
		int getCentimeters();
		int getFeet();
		int getMeters();
	private:
		int _pin;
};

#endif


