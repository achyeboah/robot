#ifndef MPU9250_H
#define MPU9250_H

#include "i2cdev.h"

namespace samsRobot {

/* From register map (RM-MPU-9250 rev 1.6, Sep 2013)
 * of the TDK MPU9250 device (Rev 1.6) */
#define MPU9250_ADDR_WHO_AM_I 0x75 // address on device for reading device address
#define MPU9250_DEV_NUM_REG 0x7E // total number of available registers (126d). not all can be accessed.

#define MPU9250_CONFIG_DLPF_CFG 0x02	// see device register map. corresponds to approx 100hz
#define MPU9250_PWR_MGMT_1_CLK_SRC 0x01	// see device register map. corresponds to x gyro

#define MPU9250_ADDR_SMPLRT_DIV	0x19	// sample rate divider
#define MPU9250_ADDR_CONFIG	0x1A	// device filter configuration
#define MPU9250_ADDR_GYRO_CONFIG 0x1B // gyroscope configuration
#define MPU9250_ADDR_ACCEL_CONFIG 0x1C // accelerometer configuration
#define MPU9250_ADDR_ACCEL_CONFIG2 0x1D // second configuration register
#define MPU9250_ADDR_INT_PIN_CFG	0x37	// interrupt pin configuration
#define MPU9250_ADDR_INT_ENABLE	0x38	//
#define MPU9250_ADDR_USER_CNTRL	0x6A	// user control
#define MPU9250_ADDR_PWR_MGMT_1	0x6B	// configure ref clock source

#define MPU9250_ADDR_ACCEL_DATA_X_H	0x3B
#define MPU9250_ADDR_ACCEL_DATA_X_L	0x3C
#define MPU9250_ADDR_ACCEL_DATA_Y_H	0x3D
#define MPU9250_ADDR_ACCEL_DATA_Y_L	0x3E
#define MPU9250_ADDR_ACCEL_DATA_Z_H	0x3F
#define MPU9250_ADDR_ACCEL_DATA_Z_L	0x40

#define MPU9250_ADDR_TEMP_DATA_H 0x41
#define MPU9250_ADDR_TEMP_DATA_L 0x42

#define MPU9250_ADDR_GYRO_DATA_X_H	0x43
#define MPU9250_ADDR_GYRO_DATA_X_L	0x44
#define MPU9250_ADDR_GYRO_DATA_Y_H	0x45
#define MPU9250_ADDR_GYRO_DATA_Y_L	0x46
#define MPU9250_ADDR_GYRO_DATA_Z_H	0x47
#define MPU9250_ADDR_GYRO_DATA_Z_L	0x48

#define MPU9250_TEMP_SENS 340 // 340 LSB/degC
#define MPU9250_TEMP_OFFSET 36.53 // datasheet says -521 at 31degC,

	/* class mpu9250. specific class for the mpu9250 accel/gyro on i2c.
	 * protected inheritance to protect public parent methods
	 */
	class mpu9250: protected i2cdev{
	public:
		// define the accelorometer full scale range
		// [Reg ACCEL_CONFIG (0x1C: bits 4-3): AFS-SEL]
		enum ACCEL_RANGE {
			PLUSMINUS_2_G = 0, // 2g equiv to 16384 LSB/g
			PLUSMINUS_4_G = 1, // 4g equiv to 8192 LSB/g
			PLUSMINUS_8_G = 2, // 8g equiv to 4096 LSB/g
			PLUSMINUS_16_G = 3 // 16g equiv to 2048 LSB/g
		};
		// define the gyroscope full scale range
		// [Reg GYRO_CONFIG (0x1B: bits 4-3): FS-SEL]
		enum GYRO_RANGE {
			PLUSMINUS_250 = 0, // 250deg/sec
			PLUSMINUS_500 = 1, // 500deg/sec
			PLUSMINUS_1000 = 2, // 1000deg/sec
			PLUSMINUS_2000 = 3 // 2000deg/sec
		};

	private:
		unsigned int i2cbus, i2caddress;
		unsigned char* registers;
		// sample rate = gyro_out_rate/(1+div), where
		// gyro_out_rate = 8khz when dlpf is disabled, 1khz when dlpf is enabled.
		bool enable_dlpf;
		mpu9250::ACCEL_RANGE accel_range;
		mpu9250::GYRO_RANGE gyro_range;

		short accelX, accelY, accelZ; // raw 2's complement
		float pitch, roll, yaw; // in degrees

		short gyroX, gyroY, gyroZ; // raw 2's complement
		float wvelX, wvelY, wvelZ; // angular velocity along axes in deg/sec

		short tempRaw; // raw temperature 16bit data
		float temp; // converted to degrees C
		short combineRegisters(unsigned char msb, unsigned char lsb);
		void init(void); //initialise device.
		void calcPitchRollYaw(void);
		void calcAngVel(void);
		void calcTemp(void);
		virtual int updateRegisters(void);

	public:
		mpu9250(unsigned int i2cbus, unsigned int i2caddress=0x68);
		virtual int readSensorState();

		virtual void setAccelRange(mpu9250::ACCEL_RANGE range);
		virtual mpu9250::ACCEL_RANGE getAccelRange(void) const;
		virtual void setGyroRange(mpu9250::GYRO_RANGE range);
		virtual mpu9250::GYRO_RANGE getGyroRange(void) const;

		virtual short getAccelX(void) const;
		virtual short getAccelY(void) const;
		virtual short getAccelZ(void) const;

		virtual float getPitch(void) const;
		virtual float getRoll(void) const;
		virtual float getYaw(void) const;

		virtual float getAngVelX(void) const;
		virtual float getAngVelY(void) const;
		virtual float getAngVelZ(void) const;

		virtual float getTemp(void) const;

		// display values
		virtual void displayData(int iterations = 100);

		// clean up
		virtual ~mpu9250();

	};
} //namespace

#endif /* MPU9250 */
