/*
 * i2cdev.h
 *
 *  Created on: 19 Jul 2020
 *  Author: sam
 */

#ifndef I2CDEV_H_
#define I2CDEV_H_

#define I2C_0 "/dev/i2c-0"
#define I2C_1 "/dev/i2c-1"

namespace samsRobot {

	class i2cdev
	{
	private:
			unsigned int bus, device;
			int file;
	public:
			i2cdev(unsigned int bus, unsigned int device);
			virtual int open();
			virtual int write(unsigned char value);
			virtual unsigned char readRegister(unsigned int regAddress);
			virtual unsigned char* readRegisters(unsigned int number, unsigned int fromAddress = 0x00 );
			virtual int writeRegister(unsigned int registerAddress, unsigned char value);
			virtual void debugDumpRegisters(unsigned int number = 0xff);
			virtual void close();
			virtual ~i2cdev();

	}; /* class */

} /* namespace */

#endif /* I2CDEV_H_ */
