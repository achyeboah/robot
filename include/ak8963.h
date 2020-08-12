#ifndef AK8963_H
#define AK8963_H

#include "i2cdev.h"

namespace samsRobot {

/* From register map (RM-MPU-9250 rev 1.6, Sep 2013)
 * of the TDK MPU9250 device (Rev 1.6) */
#define AK8963_ADDR_WHO_AM_I 0x00 // address on device for reading device address
#define AK8963_DEV_NUM_REG 0x0F // total number of available registers (18d). not all can be accessed.

#define AK8963_ADDR_CONFIG	0x0A	// device filter configuration

#define ADDR_MAG_DATA_X_L	0x03
#define ADDR_MAG_DATA_X_H	0x04
#define ADDR_MAG_DATA_Y_L	0x05
#define ADDR_MAG_DATA_Y_H	0x06
#define ADDR_MAG_DATA_Z_L	0x07
#define ADDR_MAG_DATA_Z_H	0x08

	class ak8963: protected i2cdev{

	private:
		unsigned int i2cbus, i2caddress;
		unsigned char* registers;

		short magX, magY, magZ; // raw 2's complement
		float fmagX, fmagY, fmagZ; // angular velocity along axes in deg/sec

		short combineRegisters(unsigned char msb, unsigned char lsb);
		void init(void); //initialise device.
		void calcCompass(void);
		virtual int updateRegisters(void);

	public:
		ak8963(unsigned int i2cbus, unsigned int i2caddress=0x68);
		virtual int readSensorState();

		virtual float getMagX(void) const;
		virtual float getMagY(void) const;
		virtual float getMagZ(void) const;

		// display values
		virtual void displayData(int iterations = 100);

		// clean up
		virtual ~ak8963();

	};
} // nmespace

#endif /* AK8963 */
