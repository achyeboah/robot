#ifndef IMU_H_
#define IMU_H_

#include "i2cdev.h"
#include "GPIO.h"

using namespace exploringRPi;

namespace samsRobot{

	/* From register map (RM-MPU-9250 rev 1.6, Sep 2013)
	 * of the TDK MPU9250 device (Rev 1.6) */
#define MPU9250_ADDR_WHO_AM_I 0x75 /* address on device for reading device address */
#define MPU9250_DEV_NUM_REG 0x7E /* total number of available registers (126d). not all can be accessed. */
#define MPU9250_WHO_AM_I 0x69 /* value of who_am_i register */
	/* From register map (RM-MPU-6000A-00 rev 4.2, Aug 2013)
	 * of the TDK MPU60X0 device (Rev C) */
#define MPU6050_ADDR_WHO_AM_I 0x75 /* address to read device address
#define MPU6050_DEV_NUM_REG 0x75 /* total number of available registers. not all can be accessed.*/
#define MPU6050_WHO_AM_I 0x69 /* value of who_am_i register. we'll always use AD0 to address */
	/* From register map (RM-MPU-9250 rev 1.6, Sep 2013)
	 * of the TDK MPU9250 device (Rev 1.6) */
#define AK8963_ADDR_WHO_AM_I 0x00 /* address on device for reading device address */
#define AK8963_DEV_NUM_REG 0x0F /* total number of available registers (18d). not all can be accessed.*/
#define AK8963_WHO_AM_I 0x48 /* value of who_am_i register. */
#define AK8963_ADDR_CONFIG	0x0A	/* device filter configuration */

	/* shared address for configuration */
#define CONFIG_DLPF_CFG 0x02	/* see device register map. corresponds to approx 100hz */
#define PWR_MGMT_1_CLK_SRC 0x01	/* see device register map. corresponds to x gyro*/

#define ADDR_SMPLRT_DIV	0x19	/* sample rate divider */
#define ADDR_CONFIG	0x1A	/* device filter configuration */
#define ADDR_GYRO_CONFIG 0x1B /* gyroscope configuration */
#define ADDR_ACCEL_CONFIG 0x1C // accelerometer configuration
#define ADDR_ACCEL_CONFIG2 0x1D // second configuration register (MPU9250 only)
#define ADDR_INT_PIN_CFG	0x37	// interrupt pin configuration
#define ADDR_INT_ENABLE	0x38	//
#define ADDR_USER_CNTRL	0x6A	// user control
#define ADDR_PWR_MGMT_1	0x6B	// configure ref clock source

	// shared addresses for retrieving data
#define ADDR_ACCEL_DATA_X_H	0x3B
#define ADDR_ACCEL_DATA_X_L	0x3C
#define ADDR_ACCEL_DATA_Y_H	0x3D
#define ADDR_ACCEL_DATA_Y_L	0x3E
#define ADDR_ACCEL_DATA_Z_H	0x3F
#define ADDR_ACCEL_DATA_Z_L	0x40

#define ADDR_TEMP_DATA_H 0x41
#define ADDR_TEMP_DATA_L 0x42

#define ADDR_GYRO_DATA_X_H	0x43
#define ADDR_GYRO_DATA_X_L	0x44
#define ADDR_GYRO_DATA_Y_H	0x45
#define ADDR_GYRO_DATA_Y_L	0x46
#define ADDR_GYRO_DATA_Z_H	0x47
#define ADDR_GYRO_DATA_Z_L	0x48

#define ADDR_MAG_DATA_X_L	0x03
#define ADDR_MAG_DATA_X_H	0x04
#define ADDR_MAG_DATA_Y_L	0x05
#define ADDR_MAG_DATA_Y_H	0x06
#define ADDR_MAG_DATA_Z_L	0x07
#define ADDR_MAG_DATA_Z_H	0x08

#define TEMP_SENS 340 // 340 LSB/degC
#define TEMP_OFFSET 36.53 // datasheet says -521 at 31degC,

	class imu : protected i2cdev {
		public:
			enum IMU_TYPE {
				MPU6050 = 0, 
				MPU9250 = 1,
				AK8963 = 2
			};
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
			imu::ACCEL_RANGE accel_range;
			imu::GYRO_RANGE gyro_range;
			imu::IMU_TYPE sensor_type;
			GPIO* pinAD0; /* pin for enabling this IMU */

			short accelX, accelY, accelZ; // raw 2's complement
			float pitch, roll, yaw; // in degrees

			short gyroX, gyroY, gyroZ; // raw 2's complement
			float wvelX, wvelY, wvelZ; // angular velocity along axes in deg/sec

			short magX, magY, magZ; // raw 2's complement
			float fmagX, fmagY, fmagZ; // magnetic force

			short tempRaw; // raw temperature 16bit data
			float temp; // converted to degrees C

			short combineRegisters(unsigned char msb, unsigned char lsb);
			void init(void); //initialise device.
			void calcPitchRollYaw(void);
			void calcAngVel(void);
			void calcCompass(void);
			void calcTemp();
			virtual int updateRegisters();

		public:
			imu(unsigned int i2cbus, int pinADO, unsigned int i2caddress=0x68, imu::IMU_TYPE type = imu::MPU6050);

			virtual int readSensorState(void);

			virtual void setAccelRange(imu::ACCEL_RANGE range);
			virtual imu::ACCEL_RANGE getAccelRange(void) const;
			virtual void setGyroRange(imu::GYRO_RANGE range);
			virtual imu::GYRO_RANGE getGyroRange(void) const;

			virtual short getAccelX(void) const;
			virtual short getAccelY(void) const;
			virtual short getAccelZ(void) const;

			virtual float getPitch(void) const;
			virtual float getRoll(void) const;
			virtual float getYaw(void) const;

			virtual float getAngVelX(void) const;
			virtual float getAngVelY(void) const;
			virtual float getAngVelZ(void) const;

			virtual float getMagX(void) const;
			virtual float getMagY(void) const;
			virtual float getMagZ(void) const;

			virtual float getTemp(void) const;

			virtual float getHeading(void) const;

			virtual IMU_TYPE getIMUtype (void) const;
			virtual void setIMUtype(const imu::IMU_TYPE type);

			// display values
			virtual void displayData(int iterations = 100);

			// clean up
			virtual ~imu();

	};
}

#endif // IMU_H_
