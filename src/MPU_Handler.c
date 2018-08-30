//----------------------------------------------------------------------------------------------------------
//     /$$$$$  /$$$$$$  /$$    /$$  /$$$$$$        /$$$$$$       /$$       /$$$$$$$$ /$$   /$$  /$$$$$$   //
//    |__  $$ /$$__  $$| $$   | $$ /$$__  $$      |_  $$_/      | $$      | $$_____/| $$  /$$/ /$$__  $$  //
//       | $$| $$  \ $$| $$   | $$| $$  \ $$        | $$        | $$      | $$      | $$ /$$/ | $$  \ $$  //
//       | $$| $$  | $$|  $$ / $$/| $$$$$$$$        | $$        | $$      | $$$$$   | $$$$$/  | $$$$$$$$  //
//  /$$  | $$| $$  | $$ \  $$ $$/ | $$__  $$        | $$        | $$      | $$__/   | $$  $$  | $$__  $$  //
// | $$  | $$| $$  | $$  \  $$$/  | $$  | $$        | $$        | $$      | $$      | $$\  $$ | $$  | $$  //
// |  $$$$$$/|  $$$$$$/   \  $/   | $$  | $$       /$$$$$$      | $$$$$$$$| $$$$$$$$| $$ \  $$| $$  | $$  //
//  \______/  \______/     \_/    |__/  |__/      |______/      |________/|________/|__/  \__/|__/  |__/  //
//----------------------------------------------------------------------------------------------------------

// ----- Includes -------------------------------------------------------------

#include "MPU_Handler.h"

// ------------ MadgwickAHRS variables ----------------------------------------

float realGyroX = 0.0f;
float realGyroY = 0.0f;
float realGyroZ = 0.0f;
float realAccelX = 0.0f;
float realAccelY = 0.0f;
float realAccelZ = 0.0f;
float realMagX = 0.0f;
float realMagY = 0.0f;
float realMagZ = 0.0f;

// ----- Extern variables -----------------------------------------------------

extern float accel_sum_x;
extern float accel_sum_y;
extern float accel_sum_z;
extern I2C_HandleTypeDef I2cHandle;


// ----- Private Definitions --------------------------------------------------

#if defined ( __CC_ARM   )
uint8_t PrivilegedReadOnlyArray[32] __attribute__((at(0x20002000)));

#elif defined ( __ICCARM__ )
#pragma location=0x20002000
__no_init uint8_t PrivilegedReadOnlyArray[32];

#elif defined   (  __GNUC__  )
uint8_t PrivilegedReadOnlyArray[32] __attribute__((section(".ROarraySection")));

#endif

// ----- MPU-9250 definitions ---------------------------------------------------

#define GYRO_SENSITIVITY_FS_0 131
#define GYRO_SENSITIVITY_FS_1 65.5

#define ACCEL_SENSITIVITY_FS_0 16384
#define ACCEL_SENSITIVITY_FS_1 8192

#define MAG_SENSITIVITY_FS_0 0.6
#define MAG_SENSITIVITY_FS_1 0.15

#define LOW_PASS_FILTER_RANGE 1.5

// ---- Function Definitions ----------------------------------

/* -------------------- Parsing the temperature -------------------- */
void readingTheTemperature(char i2c_mpu_temp_buffer[], char i2c_mpu_temp_write_buffer[])
{
	// char i2c_mpu_write_buffer[7];
	// char i2c_mpu_buffer[2];
	uint16_t integerTemperature = 0;
	uint16_t mask = 0;
	float realTemp = 0.0f;

	integerTemperature |= i2c_mpu_temp_buffer[0];
	integerTemperature &= 0x00FF;
	integerTemperature <<= 8;
	mask = i2c_mpu_temp_buffer[1];
	mask &= 0x00FF;
	integerTemperature |= mask;
	realTemp = ((integerTemperature) / 333.87) + 21;
	snprintf(i2c_mpu_temp_write_buffer, sizeof(i2c_mpu_temp_write_buffer), "Temperature:   %.2f\r", realTemp);
}

/* -------------------- Parsing the gyroscope -------------------- */
void readingTheGyroscope(char i2c_mpu_gyro_buffer[], char i2c_mpu_gyro_write_buffer[])
{
	int16_t gyroX = 0;
	int16_t gyroY = 0;
	int16_t gyroZ = 0;
	int16_t gyroMaskX = 0;
	int16_t gyroMaskY = 0;
	int16_t gyroMaskZ = 0;
	realGyroX = 0.0f;
	realGyroY = 0.0f;
	realGyroZ = 0.0f;

	for(int i = 0; i < sizeof(i2c_mpu_gyro_write_buffer); ++i){
		i2c_mpu_gyro_write_buffer[i] = 0;
	}

	gyroX |= i2c_mpu_gyro_buffer[0];
	gyroX <<= 8;
	gyroMaskX = i2c_mpu_gyro_buffer[1];
	gyroMaskX &= 0x00FF;
	gyroX |= gyroMaskX;

	gyroY |= i2c_mpu_gyro_buffer[2];
	gyroY <<= 8;
	gyroMaskY = i2c_mpu_gyro_buffer[3];
	gyroMaskY &= 0x00FF;
	gyroY |= gyroMaskY;

	gyroZ |= i2c_mpu_gyro_buffer[4];
	gyroZ <<= 8;
	gyroMaskZ = i2c_mpu_gyro_buffer[5];
	gyroMaskZ &= 0x00FF;
	gyroZ |= gyroMaskZ;

	realGyroX = (float)gyroX / (float)GYRO_SENSITIVITY_FS_1;
	realGyroY = (float)gyroY / (float)GYRO_SENSITIVITY_FS_1;
	realGyroZ = (float)gyroZ / (float)GYRO_SENSITIVITY_FS_1;

	/*--------------  Low-pass filter  :) ---------------
	if(realGyroX < LOW_PASS_FILTER_RANGE && realGyroX > -LOW_PASS_FILTER_RANGE) realGyroX = 0;
	if(realGyroY < LOW_PASS_FILTER_RANGE && realGyroY > -LOW_PASS_FILTER_RANGE) realGyroY = 0;
	if(realGyroZ < LOW_PASS_FILTER_RANGE && realGyroZ > -LOW_PASS_FILTER_RANGE) realGyroZ = 0;
	----------------- End of low-pass filter -------------------- */

	snprintf(i2c_mpu_gyro_write_buffer, sizeof(i2c_mpu_gyro_write_buffer), "Gyroscope:   X: %.2f Y: %.2f Z: %.2f\n", realGyroX, realGyroY, realGyroZ);
}

/* -------------------- Parsing the accelerometer  -------------------- */
void readingTheAccel(char i2c_mpu_accel_buffer[], char i2c_mpu_accel_write_buffer[])
{
	int16_t accelX = 0;
	int16_t accelY = 0;
	int16_t accelZ = 0;
	int16_t accelMaskX = 0;
	int16_t accelMaskY = 0;
	int16_t accelMaskZ = 0;
	realAccelX = 0.0f;
	realAccelY = 0.0f;
	realAccelZ = 0.0f;

	for(int i = 0; i < sizeof(i2c_mpu_accel_write_buffer); ++i){
		i2c_mpu_accel_write_buffer[i] = 0;
	}

	accelX |= i2c_mpu_accel_buffer[0];
	accelX <<= 8;
	accelMaskX = i2c_mpu_accel_buffer[1];
	accelMaskX &= 0x00FF;
	accelX |= accelMaskX;

	accelY |= i2c_mpu_accel_buffer[2];
	accelY <<= 8;
	accelMaskY = i2c_mpu_accel_buffer[3];
	accelMaskY &= 0x00FF;
	accelY |= accelMaskY;

	accelZ |= i2c_mpu_accel_buffer[4];
	accelZ <<= 8;
	accelMaskZ = i2c_mpu_accel_buffer[5];
	accelMaskZ &= 0x00FF;
	accelZ |= accelMaskZ;

	realAccelX = (float)accelX / (float)ACCEL_SENSITIVITY_FS_1;
	realAccelY = (float)accelY / (float)ACCEL_SENSITIVITY_FS_1;
	realAccelZ = (float)accelZ / (float)ACCEL_SENSITIVITY_FS_1;

	accel_sum_x+=realAccelX;
	accel_sum_y+=realAccelY;
	accel_sum_z+=realAccelZ;

	snprintf(i2c_mpu_accel_write_buffer, sizeof(i2c_mpu_accel_write_buffer), "Acceleration:   X: %.2f Y: %.2f Z: %.2f\n", realAccelX, realAccelY, realAccelZ);
}

void readingTheMagnetometer(char i2c_mpu_mag_buffer[], char i2c_mpu_mag_write_buffer[])
{
	int16_t magX = 0;
	int16_t magY = 0;
	int16_t magZ = 0;
	int16_t magMaskX = 0;
	int16_t magMaskY = 0;
	int16_t magMaskZ = 0;
	realMagX = 0.0f;
	realMagY = 0.0f;
	realMagZ = 0.0f;

	for(int i = 0; i < sizeof(i2c_mpu_mag_write_buffer); ++i){
		i2c_mpu_mag_write_buffer[i] = 0;
	}

	magX |= i2c_mpu_mag_buffer[1];
	magX <<= 8;
	magMaskX = i2c_mpu_mag_buffer[0];
	magMaskX &= 0x00FF;
	magX |= magMaskX;

	magY |= i2c_mpu_mag_buffer[3];
	magY <<= 8;
	magMaskY = i2c_mpu_mag_buffer[2];
	magMaskY &= 0x00FF;
	magY |= magMaskY;

	magZ |= i2c_mpu_mag_buffer[5];
	magZ <<= 8;
	magMaskZ = i2c_mpu_mag_buffer[4];
	magMaskZ &= 0x00FF;
	magZ |= magMaskZ;

	realMagX = (float)magX * (float)MAG_SENSITIVITY_FS_1;
	realMagY = (float)magY * (float)MAG_SENSITIVITY_FS_1;
	realMagZ = (float)magZ * (float)MAG_SENSITIVITY_FS_1;

	snprintf(i2c_mpu_mag_write_buffer, sizeof(i2c_mpu_mag_write_buffer), "Magnetometer:   X: %.5f Y: %.5f Z: %.5f\n", realMagX, realMagY, realMagZ);
}

void resetMagnetometer()
{
	HAL_I2C_Mem_Write(&I2cHandle, 0x18u, 0x0A, 1, 0x00, 1, HAL_MAX_DELAY);
	HAL_Delay(10);
	HAL_I2C_Mem_Write(&I2cHandle, 0x18u, 0x0A, 1, 0x0F, 1, HAL_MAX_DELAY);
	HAL_Delay(10);
	HAL_I2C_Mem_Write(&I2cHandle, 0x18u, 0x0A, 1, 0x00, 1, HAL_MAX_DELAY);
	HAL_Delay(10);
	HAL_I2C_Mem_Write(&I2cHandle, 0x18u, 0x0A, 1, 1 << 4 | 2, 1, HAL_MAX_DELAY);
	HAL_Delay(10);
}
