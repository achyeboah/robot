#include "imu.h"

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
	imu::imu(unsigned i2cbus, unsigned int i2caddress, imu::IMU_TYPE type):
		i2cdev(i2cbus, i2caddress){
		this->i2caddress = i2caddress;
		// enable the lpf.
		// This makes both accel and gyro rate 1khz.
		this->enable_dlpf = true;

		// init our members
		this->accelX = 0;
		this->accelY = 0;
		this->accelZ = 0;
		this->pitch = 0.0f;
		this->roll = 0.0f;
		this->yaw = 0.0f;

		this->tempRaw = 0;
		this->temp = 0.0f;
		
		this->magX = 0;
		this->magY = 0;
		this->magZ = 0;
		this->fmagX = 0.0f;
		this->fmagY = 0.0f;
		this->fmagZ = 0.0f;

		this->registers = NULL;

		this->accel_range = imu::PLUSMINUS_2_G;
		this->gyro_range = imu::PLUSMINUS_500;
		this->sensor_type = type;

		// initialize the device
		this->init();
		this->updateRegisters();
	}

	/* initialise the device*/
	void imu::init(){
		if (this->enable_dlpf == true){
			// low pass filtered to approx 94hz bandwidth (close enough to 100hz)
			// this also makes sampling freq for temperature and gyro 1khz
			this->writeRegister(ADDR_CONFIG, CONFIG_DLPF_CFG);
		}
		// set accel dlpf
		if (this->sensor_type == imu::MPU9250)
			this->writeRegister(ADDR_ACCEL_CONFIG2, (unsigned char)2);
		// use a gyro as clock ref instead of internal oscillator
		this->writeRegister(ADDR_PWR_MGMT_1, PWR_MGMT_1_CLK_SRC);
		// set gyro range
		this->writeRegister(ADDR_GYRO_CONFIG, (unsigned char)gyro_range << 3);
		// set accel range
		this->writeRegister(ADDR_ACCEL_CONFIG, (unsigned char)accel_range << 3);
		// enable interrupts when data is ready
		this->writeRegister(ADDR_INT_ENABLE, 0x01);
	}

	/* read the sensor values. checks device can be correctly read,
	 * then read in up to date values, process and update class members */
	int imu::readSensorState(){
		unsigned int address = 0;
		unsigned int numRegs = 0;

		if (this->sensor_type == imu::MPU6050){
			address = MPU6050_ADDR_WHO_AM_I;
			numRegs = MPU6050_DEV_NUM_REG;
		}else{
			address = MPU9250_ADDR_WHO_AM_I;
			numRegs = MPU9250_DEV_NUM_REG;
		}
		if(this->readRegister(address) != this->i2caddress){
			fprintf(stderr, "MPU9250: Failure to read from correct device\n");
			return -1;
		}
		// read in accessible registers

		this->registers = this->readRegisters(numRegs, 0x0);

		this->accelX = this->combineRegisters(*(registers+ADDR_ACCEL_DATA_X_H), *(registers+ADDR_ACCEL_DATA_X_L));
		this->accelY = this->combineRegisters(*(registers+ADDR_ACCEL_DATA_Y_H), *(registers+ADDR_ACCEL_DATA_Y_L));
		this->accelZ = this->combineRegisters(*(registers+ADDR_ACCEL_DATA_Z_H), *(registers+ADDR_ACCEL_DATA_Z_L));

		this->gyroX = this->combineRegisters(*(registers+ADDR_GYRO_DATA_X_H), *(registers+ADDR_GYRO_DATA_X_L));
		this->gyroY = this->combineRegisters(*(registers+ADDR_GYRO_DATA_Y_H), *(registers+ADDR_GYRO_DATA_Y_L));
		this->gyroZ = this->combineRegisters(*(registers+ADDR_GYRO_DATA_Z_H), *(registers+ADDR_GYRO_DATA_Z_L));

		this->tempRaw = this->combineRegisters(*(registers+ADDR_TEMP_DATA_H), *(registers+ADDR_TEMP_DATA_L));
	
		// read in the sensors actual current range
		this->accel_range = (imu::ACCEL_RANGE) ((*(registers + ADDR_ACCEL_CONFIG))&0x18);
		this->gyro_range = (imu::GYRO_RANGE) ((*(registers + ADDR_GYRO_CONFIG))&0x18);

		if (this->sensor_type == imu::MPU9250){
			// read from ak8963
			if(this->readRegister(AK8963_ADDR_WHO_AM_I) != AK8963_ADDR_WHO_AM_I){
				fprintf(stderr, "AK8963 not found!\n");
				return -1;
			}
			this->registers = this->readRegisters(AK8963_DEV_NUM_REG, 0x0);

			this->magX = this->combineRegisters(*(registers+ADDR_MAG_DATA_X_H), *(registers+ADDR_MAG_DATA_X_L));
			this->magY = this->combineRegisters(*(registers+ADDR_MAG_DATA_Y_H), *(registers+ADDR_MAG_DATA_Y_L));
			this->magZ = this->combineRegisters(*(registers+ADDR_MAG_DATA_Z_H), *(registers+ADDR_MAG_DATA_Z_L));
		}

		this->calcPitchRollYaw();
		this->calcAngVel();
		this->calcTemp();
		this->calcCompass();

		return 0;
	}

	/* set range of accelerometer according to ACCEL_RANGE enum
	 * param range: one of enum values */
	void imu::setAccelRange(imu::ACCEL_RANGE range)
	{
		this->accel_range = range;
		this->updateRegisters();
	}
	imu::ACCEL_RANGE imu::getAccelRange(void) const{
		return this->accel_range;
	}
	/* set range of gyroscope according to GYRO_RANGE enum
	 * param range: one of enum values
	 */
	void imu::setGyroRange(imu::GYRO_RANGE range) {
		this->gyro_range = range;
		this->updateRegisters();
	}
	imu::GYRO_RANGE imu::getGyroRange(void) const{
		return this->gyro_range;
	}

	/* combine 8bit registers into a single short (16bit on rpi).
	 * param msb: unsigned char which is msb
	 * param lsb: unsigned char which is lsb
	 */
	short imu::combineRegisters(unsigned char msb, unsigned char lsb){
		return ((short)msb<<8 | (short)lsb);
	}

	/* calculate pitch, roll and yaw values.
	 * Accounts for range/resolution and effect of gravity
	 */
	void imu::calcPitchRollYaw()
	{
		float factor = 0.0f;

		switch(this->accel_range){
			case imu::PLUSMINUS_16_G : factor = 2048.0f; break; // AFS_SEL = 0
			case imu::PLUSMINUS_8_G : factor = 4096.0f; break; // AFS_SEL = 1
			case imu::PLUSMINUS_4_G : factor = 8192.0f; break; // AFS_SEL = 2
			default: factor = 16384.0f; break; // AFS_SEL = 3
		}
		float accXg = this->accelX * factor;
		float accYg = this->accelY * factor;
		float accZg = this->accelZ * factor;
		float accXg_sq = accXg * accXg;
		float accYg_sq = accYg * accYg;
		float accZg_sq = accZg * accZg;

		this->pitch = 180.0f * atan(accXg/sqrt(accYg_sq + accZg_sq))/M_PI;
		this->roll = 180.0f * atan(accYg/sqrt(accXg_sq + accZg_sq))/M_PI;
		this->yaw = 180.0f * atan(accZg/sqrt(accXg_sq + accYg_sq))/M_PI;
	}

	/* calculate the position of the device
	 */
	void imu::calcAngVel(){
		float factor = 0.0f;
		switch (this->gyro_range){
		case imu::PLUSMINUS_2000 : factor = 16.4f; break; //FS_SEL = 0
		case imu::PLUSMINUS_1000 : factor = 32.8f; break; //FS_SEL = 1
		case imu::PLUSMINUS_500 : factor = 65.5f; break; //FS_SEL = 2
		default: factor = 131.0f; // FS_SEL = 3
		}
		this->wvelX = (float)gyroX/factor;
		this->wvelY = (float)gyroY/factor;
		this->wvelZ = (float)gyroZ/factor;
	}

	/* calculate the temperature using the raw data
	 */
	void imu::calcTemp(){
		this->temp = ((float)(tempRaw)/TEMP_SENS) + (float)TEMP_OFFSET;
	}

	/* update the device registers as required
	 * return 0 if successful
	 */
	int imu::updateRegisters(){
		// nothing specific here
		this->init();
		return 0;
	}

	void imu::displayData(int iterations){
		int count = 0;
		while (count < iterations){
			cout << "@"<< count << "\tOrient PRY (deg):" << std::setprecision(3) << this->getPitch() << " " << this->getRoll() << " " << this->getYaw() <<" " <<endl;
			cout << "@"<< count << "\tAngVel XYZ (deg/sec): " << this->getAngVelX() << " " << this->getAngVelY() << " " << this->getAngVelZ() << " " << endl;
			cout << "@"<< count << "\tTemp (degC): " << this->getTemp()<< " raw :" << this->tempRaw << endl <<endl;
			cout << "@"<< count << "\tMag** XYZ (uT): " << this->getMagX() << " " << this->getMagY() << " " << this->getMagZ() << " " << endl;
			usleep(100000); // 100ms
			this->readSensorState();
			count++;
		}
	}


	/* calculate the position of the device
	*/
	void imu::calcCompass(){
		if (this->sensor_type == imu::MPU9250){
			// full scale range is 4912uT/32760
			float factor = 4912.0f/32760.0f;
			this->fmagX = (float)magX/factor;
			this->fmagY = (float)magY/factor;
			this->fmagZ = (float)magZ/factor;
		}else{
			this->fmagX = 0.0f;
			this->fmagY = 0.0f;
			this->fmagZ = 0.0f;
		}
	}

	imu::~imu(){
		/* nothing to do here*/
	}

	short imu::getAccelX(void) const {return this->accelX;}
	short imu::getAccelY(void) const {return this->accelY;}
	short imu::getAccelZ(void) const {return this->accelZ;}

	float imu::getPitch(void) const {return this->pitch;}
	float imu::getRoll(void) const {return this->roll;}
	float imu::getYaw(void) const {return this->yaw;}

	float imu::getAngVelX(void) const {return this->wvelX;}
	float imu::getAngVelY(void) const {return this->wvelY;}
	float imu::getAngVelZ(void) const {return this->wvelZ;}

	float imu::getMagX(void) const {return this->fmagX;}
	float imu::getMagY(void) const {return this->fmagY;}
	float imu::getMagZ(void) const {return this->fmagZ;}

	float imu::getTemp(void) const {return this->temp;}

	imu::IMU_TYPE imu::getIMUtype (void) const {return this->sensor_type;}
	void imu::setIMUtype(const imu::IMU_TYPE type){
	       	this->sensor_type = type;
		this->init();
	}

	float imu::getHeading(void) const{
		//fuse the compass readings into a heading in degrees (ie between 0 (MAG NORTH) and 359)
		return fmagX + fmagY + fmagZ;
	}

 } // namespace

