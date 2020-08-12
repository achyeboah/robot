#include "ak8963.h"

#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <math.h>
#include <stdio.h>

using namespace std;

namespace samsRobot {

	/* constructor: initialise the device
	 * param i2cbus: the bus number, default is 1
	 * param i2caddress: the device address
	 */
	ak8963::ak8963(unsigned i2cbus, unsigned int i2caddress):
		i2cdev(i2cbus, i2caddress){
		this->i2caddress = i2caddress;

		// init our members
		this->magX =0;
		this->magY =0;
		this->magZ =0;
		this->fmagX = 0.0;
		this->fmagY = 0.0;
		this->fmagZ = 0.0;
		this->registers = NULL;

		// initialize the device
		this->init();
		this->updateRegisters();
	}

	/* initialise the device
	 */
	void ak8963::init(){
		// not much to do for this device!
	}

	/* read the sensor values. checks device can be correctly read,
	 * then read in up to date values, process and update class members
	 */
	int ak8963::readSensorState(){
		if(this->readRegister(AK8963_ADDR_WHO_AM_I) != this->i2caddress){
			perror("AK8963: Failure to read from correct device\n");
			return -1;
		}
		// read in accessible registers

		this->registers = this->readRegisters(AK8963_DEV_NUM_REG, 0x0);

		this->magX = this->combineRegisters(*(registers+ADDR_MAG_DATA_X_H), *(registers+ADDR_MAG_DATA_X_L));
		this->magY = this->combineRegisters(*(registers+ADDR_MAG_DATA_Y_H), *(registers+ADDR_MAG_DATA_Y_L));
		this->magZ = this->combineRegisters(*(registers+ADDR_MAG_DATA_Z_H), *(registers+ADDR_MAG_DATA_Z_L));

		this->calcCompass();
		return 0;
	}

	/* combine 8bit registers into a single short (16bit on rpi).
	 * param msb: unsigned char which is msb
	 * param lsb: unsigned char which is lsb
	 */
	short ak8963::combineRegisters(unsigned char msb, unsigned char lsb){
		return ((short)msb<<8 | (short)lsb);
	}

	/* calculate the position of the device
	*/
	void ak8963::calcCompass(){
		// full scale range is 4912uT/32760

		float factor = 4912.0f/32760.0f;
		this->fmagX = (float)magX/factor;
		this->fmagY = (float)magY/factor;
		this->fmagZ = (float)magZ/factor;
	}

	/* update the device registers as required
	 * return 0 if successful
	 */
	int ak8963::updateRegisters(){
		// nothing specific here
		this->init();
		return 0;
	}

	void ak8963::displayData(int iterations){
		int count = 0;
		while (count < iterations){
			cout << "@"<< count << "\tMag XYZ (uT): " << this->getMagX() << " " << this->getMagY() << " " << this->getMagZ() << " " << endl;
			usleep(100000); // 100ms
			this->readSensorState();
			count++;
		}
	}

	ak8963::~ak8963(){
		/* nothing to do here*/
	}

	float ak8963::getMagX(void) const {return this->fmagX;}
	float ak8963::getMagY(void) const {return this->fmagY;}
	float ak8963::getMagZ(void) const {return this->fmagZ;}


} // namespace


