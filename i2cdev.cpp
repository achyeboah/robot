/*
 * i2cdev.cpp
 *
 *  Created on: 19 Jul 2020
 *  Author: sam
 */

#include "i2cdev.h"
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <stdio.h>
#include <iomanip>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

using namespace std;

#define HEX(x) setw(2) << setfill('0') << hex << (int)(x)

namespace samsRobot {

	/* constructor for the i2cdev class
	 * param bus: bus number
	 * param device: the device id on the bus
	 */
	 i2cdev::i2cdev(unsigned int bus, unsigned int device)
	 {
		 this->file = -1;
		 this->bus = bus;
		 this->device = device;
		 this->open();
	 }

	 /* open a connection to an I2C device
	  * return 1 on failure to open the bus
	  * return 2 on failure to open the device,
	  * return 0 on success
	  */
	int i2cdev::open(void)
	{
		string name;
		if(this->bus==0)
			name = I2C_0;
		else
			name = I2C_1;

		if((this->file=::open(name.c_str(),O_RDWR)) < 0){
			perror("I2C: Failed to open the bus\n");
			return 1;
		}
		if(ioctl(this->file, I2C_SLAVE, this->device) < 0){
			perror("I2C: Failed to connect to the device\n");
			return 2;
		}
		return 0;
	}

	/* write a single byte value to a single register
	 * param registerAddress: the address of the register to write it to
	 * param value: the value to write to the address
	 * return 1 on failure, 0 on success
	 */
	int i2cdev::writeRegister(unsigned int registerAddress, unsigned char value){
		unsigned char buffer[2];
		buffer[0] = registerAddress;
		buffer[1] = value;
		if(::write(this->file, buffer, 2)!=2){
			perror("I2C:Failed write to the device\n");
			return 1;
		}
		return 0;
	}

	/* write a single byte to the i2c device. Used to set up the device to read from a single address
	 * param registerAddress: the address of interest
	 * param value: the value to be written
	 * return 1 on failure, 0 on success
	 */
	int i2cdev::write(unsigned char value){
		unsigned char buffer[1];
		buffer[0] = value;
		if(::write(this->file, buffer, 1) != 1){
			perror("I2C: Failed to write to the device\n");
			return 1;
		}
		return 0;
	}

	/* read a single register value from an register on the device
	 * param registerAddress the address to read from
	 * return the byte value at the register address
	 */
	unsigned char i2cdev::readRegister(unsigned int registerAddress){
		this->write(registerAddress);
		unsigned char buffer[1];
		if(::read(this->file, buffer, 1) != 1){
			perror("U2C: Failed to read in the value. \n");
			return 1;
		}
		return buffer[0];
	}

	/* read a number of registers from a single device.
	 * more efficient than reading registers individually.
	 * Starting address defaults to 0x00
	 * param number: the number of registers to read from device
	 * param fromAddress: the starting address to read from
	 * return a pointer of type unsigned char* that points to first element
	 */

	unsigned char* i2cdev::readRegisters(unsigned int number, unsigned int fromAddress){
		this->write(fromAddress);
		unsigned char* data = new unsigned char[number];

		if(::read(this->file, data, number)!=(int)number){
			perror("I2C: Failed to read in the full buffer.\n");
			return NULL;
		}
		return data;
	}

	/* function to dump registers to standard output.
	 * insert a return character after every 16values and displays the results in hex
	 * to give a standard output using the hex() macro.
	 * param number: total number of registers to dump, defaults to 0xff
	 */
	void i2cdev::debugDumpRegisters(unsigned int number){
		cout << "Dumping registers for debug: " << endl;
		unsigned char *registers = this->readRegisters(number);
		for(int i = 0; i < (int)number; i++){
			cout << HEX(*(registers+1)) << "\t";
			if (i%16==15) cout <<endl;
		}
		cout << dec;
	}

	/* close the file handles and sets a temp state to -1; */
	void i2cdev::close(){
		::close(this->file);
		this->file = -1;
	}

	/* closes the file on destruction, provided it has not already been closed
	 */
	i2cdev::~i2cdev(){
		if(file!=-1) this->close();
	}
} /* namespace */

