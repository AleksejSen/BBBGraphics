/*
 * adc.cpp
 *
 *  Created on: Oct 28, 2020
 *      Author: alex
 */
#include "adc.h"
#include<sstream>
#include<fstream>



int readAnalog(int number)
{
	// returns the input as an int
	std::stringstream ss;
	ss << LDR_PATH << number << "_raw";
	std::fstream fs;
	fs.open(ss.str().c_str(), std::fstream::in);
	fs >> number;
	fs.close();
	return number;
}
