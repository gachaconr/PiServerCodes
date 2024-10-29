// Original Written by Larry Bank - 11/4/2017

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <math.h>
#include "ccs811_i2c.h"

static int file_i2c = -1;
static int file_i2cbme280 = -1;

unsigned char dig_T1[2];
unsigned char dig_T2[2];
unsigned char dig_T3[2];
unsigned char dig_P1[2];
unsigned char dig_P2[2];
unsigned char dig_P3[2];
unsigned char dig_P4[2];
unsigned char dig_P5[2];
unsigned char dig_P6[2];
unsigned char dig_P7[2];
unsigned char dig_P8[2];
unsigned char dig_P9[2];
unsigned char dig_H1[1];
unsigned char dig_H2[2];
unsigned char dig_H3[1];
unsigned char dig_H4[2];
unsigned char dig_H5[2];
unsigned char dig_H6[1];

//
// Opens a file system handle to the I2C device
// Starts the 'app' in the CCS811 microcontroller
// into continuous mode to read values every second
//
int ccs811Init(int iChannel, int iAddr)
{
	int i, rc;
	unsigned char ucTemp[32];
	char filename[32];

	sprintf(filename, "/dev/i2c-%d", iChannel);
	if ((file_i2c = open(filename, O_RDWR)) < 0)
	{
		fprintf(stderr, "Failed to open the i2c bus; run as sudo?\n");
		return 0;
	}

	if (ioctl(file_i2c, I2C_SLAVE, iAddr) < 0)
	{
		fprintf(stderr, "Failed to acquire bus access or talk to slave\n");
		file_i2c = 0;
		return 0;
	}

	ucTemp[0] = 0x20; // HW_ID (read the Hardware ID)
	rc = write(file_i2c, ucTemp, 1);
	i = read(file_i2c, ucTemp, 1);
	if (rc != 1 || i != 1 || ucTemp[0] != 0x81)
	{
		printf("Error, ID doesn't match 0x81; wrong device?\n");
		printf("Value returned = %02x\n", ucTemp[0]);
		return 0;
	}

	ucTemp[0] = 0xf4; // APP_START
	rc = write(file_i2c, ucTemp, 1); // tell it to start running the 'APP'
	if (rc == 1)
	{
		ucTemp[0] = 0x01; // MEAS_MODE 
		ucTemp[1] = 0x10; // constant power mode (001), no interrupts
		rc = write(file_i2c, ucTemp, 2);
		if (rc != 2) {} // suppress compiler warning
	}
	return 1;

} /* ccs811Init() */

int bme280Init(int jChannel, int jAddr, unsigned char data[33]){
	int j,	rc;
	char filename[32];
	unsigned char ucTemp[32];
	sprintf(filename, "/dev/i2c-%d", jChannel);
	if ((file_i2cbme280 = open(filename, O_RDWR)) < 0){
		fprintf(stderr, "Failed to open the i2c bus; run as sudo??\n");
		return 0;
	}
	if (ioctl(file_i2cbme280, I2C_SLAVE, jAddr ) < 0){
		fprintf(stderr, "Failed to acquire bus acces for BME280\n");
		file_i2cbme280 = 0;
		return 0 ;
	}

	bme280LoadCalibration(data);
	usleep(2000);
	ucTemp[0] = 0xF4;
	ucTemp[1] = 0x24;
	rc = write(file_i2cbme280,ucTemp,2);
	if (rc != 2){ return 0; }
	usleep(2000); // 2 ms
	ucTemp[0] = 0xF5;
	ucTemp[1] = 0x60;
	rc = write(file_i2cbme280,ucTemp,2);
	if (rc != 2){ return 0; }
	usleep(2000); // 2 ms
	ucTemp[0] = 0xF2;
	ucTemp[1] = 0x01;
	rc = write(file_i2cbme280,ucTemp,2);
	if (rc != 2){ return 0; }
	usleep(2000); // 2 ms
	ucTemp[0] = 0xF4;
	ucTemp[1] = 0x27;
	rc = write(file_i2cbme280,ucTemp,2);
	if (rc != 2){ return 0; }

	return 1;
}

int bme280LoadCalibration(unsigned char data[33]){
	unsigned char ucTemp[32];
	int i, rc;

	ucTemp[0] = 0x88;
	rc = write(file_i2cbme280,ucTemp,1);
	i = read(file_i2cbme280,dig_T1,2);
	if (rc != 1 || i != 2){ printf("Error loading dig_T1\n"); }
	memcpy(data,dig_T1,2);

	ucTemp[0] = 0x8A;
	rc = write(file_i2cbme280,ucTemp,1);
	i = read(file_i2cbme280,dig_T2,2);
	if (rc != 1 || i != 2){ printf("Error loading dig_T2\n"); }
	memcpy(data+2,dig_T2,2);	

	ucTemp[0] = 0x8C;
	rc = write(file_i2cbme280,ucTemp,1);
	i = read(file_i2cbme280,dig_T3,2);
	if (rc != 1 || i != 2){ printf("Error loading dig_T3\n"); }
	memcpy(data+4,dig_T3,2);	
	
	ucTemp[0] = 0x8E;
	rc = write(file_i2cbme280,ucTemp,1);
	i = read(file_i2cbme280,dig_P1,2);
	if (rc != 1 || i != 2){ printf("Error loading dig_P1\n"); }
	memcpy(data+6,dig_P1,2);

	ucTemp[0] = 0x90;
	rc = write(file_i2cbme280,ucTemp,1);
	i = read(file_i2cbme280,dig_P2,2);
	if (rc != 1 || i != 2){ printf("Error loading dig_P2\n"); }
	memcpy(data+8,dig_P2,2);	

	ucTemp[0] = 0x92;
	rc = write(file_i2cbme280,ucTemp,1);
	i = read(file_i2cbme280,dig_P3,2);
	if (rc != 1 || i != 2){ printf("Error loading dig_P3\n"); }
	memcpy(data+10,dig_P3,2);	

	ucTemp[0] = 0x94;
	rc = write(file_i2cbme280,ucTemp,1);
	i = read(file_i2cbme280,dig_P4,2);
	if (rc != 1 || i != 2){ printf("Error loading dig_P4\n"); }
	memcpy(data+12,dig_P4,2);	

	ucTemp[0] = 0x96;
	rc = write(file_i2cbme280,ucTemp,1);
	i = read(file_i2cbme280,dig_P5,2);
	if (rc != 1 || i != 2){ printf("Error loading dig_P5\n"); }
	memcpy(data+14,dig_P5,2);	

	ucTemp[0] = 0x98;
	rc = write(file_i2cbme280,ucTemp,1);
	i = read(file_i2cbme280,dig_P6,2);
	if (rc != 1 || i != 2){ printf("Error loading dig_P6\n"); }
	memcpy(data+16,dig_P6,2);

	ucTemp[0] = 0x9A;
	rc = write(file_i2cbme280,ucTemp,1);
	i = read(file_i2cbme280,dig_P7,2);
	if (rc != 1 || i != 2){ printf("Error loading dig_P7\n"); }
	memcpy(data+18,dig_P7,2);	

	ucTemp[0] = 0x9C;
	rc = write(file_i2cbme280,ucTemp,1);
	i = read(file_i2cbme280,dig_P8,2);
	if (rc != 1 || i != 2){ printf("Error loading dig_P8\n"); }
	memcpy(data+20,dig_P8,2);	

	ucTemp[0] = 0x9E;
	rc = write(file_i2cbme280,ucTemp,1);
	i = read(file_i2cbme280,dig_P9,2);
	if (rc != 1 || i != 2){ printf("Error loading dig_P9\n"); }
	memcpy(data+22,dig_P9,2);	

	ucTemp[0] = 0xA1;
	rc = write(file_i2cbme280,ucTemp,1);
	i = read(file_i2cbme280,dig_H1,1);
	if (rc != 1 || i != 1){ printf("Error loading dig_H1\n"); }
	memcpy(data+24,dig_H1,1);

	ucTemp[0] = 0xE1;
	rc = write(file_i2cbme280,ucTemp,1);
	i = read(file_i2cbme280,dig_H2,2);
	if (rc != 1 || i != 2){ printf("Error loading dig_H2\n"); }
	memcpy(data+25,dig_H2,2);

	ucTemp[0] = 0xE3;
	rc = write(file_i2cbme280,ucTemp,1);
	i = read(file_i2cbme280,dig_H3,1);
	if (rc != 1 || i != 1){ printf("Error loading dig_H3\n"); }
	memcpy(data+27,dig_H3,1);

	ucTemp[0] = 0xE4;
	rc = write(file_i2cbme280,ucTemp,1);
	i = read(file_i2cbme280,dig_H4,2);
	if (rc != 1 || i != 2){ printf("Error loading dig_H4\n"); }
	memcpy(data+28,dig_H4,2);

	ucTemp[0] = 0xE5;
	rc = write(file_i2cbme280,ucTemp,1);
	i = read(file_i2cbme280,dig_H5,2);
	if (rc != 1 || i != 2){ printf("Error loading dig_H5\n"); }
	memcpy(data+30,dig_H5,2);

	ucTemp[0] = 0xE7;
	rc = write(file_i2cbme280,ucTemp,1);
	i = read(file_i2cbme280,dig_H6,1);
	if (rc != 1 || i != 1){ printf("Error loading dig_H6\n"); }
	memcpy(data+32,dig_H6,1);
}

int bme280ReadID(char *id){
	unsigned char ucTemp[32];
	int i, rc;
	*id = 0;
	ucTemp[0] = 0xD0;
	rc = write(file_i2cbme280, ucTemp, 1);
	i = read(file_i2cbme280, ucTemp, 1);
	if (rc != 1 || i != 1){
		printf("Error with the BME208 sensor communication\n");
		return 0;
	}
	*id = ucTemp[0];
	return  1;
}

int bme280Read(float *bme280Temp, float *bme280Press, float *bme280Hum, unsigned char data[8]){
	unsigned char ucTemp[32];
	int i, rc;
	int raw;
	float ut, var1, var2;
	unsigned short T1f;
	signed short T2f, T3f;
	unsigned short P1f;
	signed short P2f, P3f, P4f, P5f, P6f, P7f, P8f, P9f;
	unsigned char H1f, H3f, H6f;
	signed short H2f, H4f, H5f;
	ucTemp[0] = 0xF3;
	rc = write(file_i2cbme280, ucTemp, 1);
	i = read(file_i2cbme280, ucTemp, 1);
	while(ucTemp[0] & 0x08){
		ucTemp[0] = 0xF3;
		rc = write(file_i2cbme280, ucTemp, 1);
		i = read(file_i2cbme280, ucTemp, 1);
	}
	// data read from bme280
	usleep(2000);
	ucTemp[0] = 0xF7;
	rc = write(file_i2cbme280, ucTemp, 1);
	i = read(file_i2cbme280, ucTemp, 8);
	if (rc != 1  || i != 8){ 
		printf("Error reading data from bme280\n");
		return 0;
	}
	memcpy(data,ucTemp,8);
	// Temperature
	T1f = (dig_T1[1] << 8) | dig_T1[0];
	T2f = (dig_T2[1] << 8) | dig_T2[0];
	T3f = (dig_T3[1] << 8) | dig_T3[0];
	raw = ((ucTemp[3] << 16) | (ucTemp[4] << 8) | ucTemp[5] ) >> 4;
	ut = (float) raw;
	var1 = ( (ut/16384.0) - (((float)T1f)/1024.0) )*((float)T2f);
	var2 = ( ( (ut/131072.0) - (((float)T1f)/8192.0) )*((ut/131072.0) - (((float)T1f)/8192.0)))*((float)T3f);
	*bme280Temp = (var1 + var2)/5120.0;
	// Pressure
	P1f = (dig_P1[1] << 8) | dig_P1[0];
	P2f = (dig_P2[1] << 8) | dig_P2[0];
	P3f = (dig_P3[1] << 8) | dig_P3[0];
	P4f = (dig_P4[1] << 8) | dig_P4[0];
	P5f = (dig_P5[1] << 8) | dig_P5[0];
	P6f = (dig_P6[1] << 8) | dig_P6[0];
	P7f = (dig_P7[1] << 8) | dig_P7[0];
	P8f = (dig_P8[1] << 8) | dig_P8[0];
	P9f = (dig_P9[1] << 8) | dig_P9[0];
	raw = ((ucTemp[0] << 16) | (ucTemp[1] << 8) | ucTemp[2] ) >> 4;
	ut = (float) raw;
	var1 = -64000.0;
	var2 = var1*var1*((float)P6f)/32768.0;
	var2 = var2 + var1*((float)P5f)*2.0;
	var2 = (var2/4.0) + ((float)P4f)*65536.0;
	var1 = ( ((float)P3f)*var1*var1/524288.0 + ((float)P2f)*var1)/524288.0;
	var1 = (1.0 + var1/32768.0)*((float)P1f);
	if (var1 == 0){
		printf("No pressure\n");
		*bme280Press = 0.0;
	} else {
		ut = 1048576.0 - ut;
		ut = ((ut - var2/4096.0)*6250.0)/var1;
		var1 = ((float)P9f)*ut*ut/2147483648.0;
		var2 = ut*((float)P8f)/32768.0;
		*bme280Press = ut + (var1 + var2 + ((float)P7f))/16.0;
	}
	// Humidity
	H1f = dig_H1[0];
	H2f = (dig_H2[1] << 8) | dig_H2[0];
	H3f = dig_H3[0];
	H4f = (dig_H4[0] << 4) | (dig_H4[1] & 0x0F);
	H5f = (dig_H5[1] << 4) | (dig_H5[0] & 0x0F);
	H6f = dig_H6[0];
	raw = (ucTemp[6] << 8) | ucTemp[7];
	ut = -76800.0;
	ut = (((float)raw)-(((float)H4f)*64.0+((float)H5f)/16384.0 *ut))*(((float)H2f)/65536.0*(1.0+((float)H6f)/67108864.0*ut*(1.0+((float)H3f)/67108864.0*ut)));
	ut = ut*(1.0 - ((float)H1f)*ut/524288.0);
	if (ut > 100){
		*bme280Hum = 100.0;
	} else {
		if (ut < 0){ 
			*bme280Hum = 0.0; 
		} else {
			*bme280Hum = ut;
		}
	}
	//////////////
	return 1;
}

//
// Turn off the sensor and close the I2C handle
//
void ccs811Shutdown(void)
{
	unsigned char ucTemp[2];
	int rc;

	if (file_i2c != -1)
	{
		ucTemp[0] = 0x01; // MEAS_MODE
		ucTemp[1] = 0; // return to idle
		rc = write(file_i2c, ucTemp, 2);
		if (rc != 2) {} // suppress compiler warning
		close(file_i2c);
		file_i2c = -1;
	}
} /* ccs811Shutdown() */

//
// Set the calibration values of temperature and humidity
// to provide more accurate air quality readings
// Temperature in Celcius and Humidity as percent (50 = 50%)
//
int ccs811SetCalibration(float fTemp, float fHumid)
{
	unsigned char ucTemp[5];
	int i, rc;
	if (file_i2c != -1)
	{
		i = (int)(fHumid * 512.0); // convert to 512th fractions
		ucTemp[0] = 0x05; // environmental data
		ucTemp[1] = (unsigned char)(i >> 8); // high byte
		ucTemp[2] = (unsigned char)i; // low byte

		i = (int)((fTemp  - 25.0) * 512.0); // offset of -25C
		ucTemp[3] = (unsigned char)(i >> 8); // high byte
		ucTemp[4] = (unsigned char)i; // low byte
		rc = write(file_i2c, ucTemp, 5);
		if (rc != 5) {} // suppress compiler warning
		return 1;
	}
	return 0;
} /* ccs811SetCalibration() */

//
// Read the sensor's algorithm values
//
int ccs811ReadValues(int *eCO2, int *TVOC, unsigned char data[5])
{
	unsigned char ucTemp[16];
	int i, rc;
	if (file_i2c != -1)
	{
		ucTemp[0] = 0x02; // Algorithm data results
		rc = write(file_i2c, ucTemp, 1); // write address of register to read
		i = read(file_i2c, ucTemp, 5); // read results and status
		if (rc != 1 || i != 5)
		{
			return 0; // something went wrong
		}
		if (ucTemp[4] & 1) // error, read the error value
		{
			ucTemp[0] = 0xe0;
			rc = write(file_i2c, ucTemp, 1);
			i = read(file_i2c, ucTemp, 1); // read the error
			printf("error = %02x\n", ucTemp[0]);
			return 0;
		}
		memcpy(data,ucTemp,5);
		if ((ucTemp[4] & 0x99) == 0x98) // firmware valid and data ready
		{	
			*eCO2 = (ucTemp[0] << 8) + ucTemp[1];
			*TVOC = (ucTemp[2] << 8) + ucTemp[3];
			if (*eCO2 > 2000 || *TVOC > 1200) // invalid values
				return 0;
			return 1;
		}
		else
		{
			return 0;
		}
	}
	return 0;
} /* ccs811ReadValues() */

int ccs811ReadTemp(float *Tempf){

	unsigned char ucTemp[16];
	int i, rc;
	int vref, vrntc;
	float rntc, temp;
	if (file_i2c != -1){
		ucTemp[0] = 0x06;
		rc = write(file_i2c, ucTemp, 1);
		i = read(file_i2c, ucTemp, 4);
		vref = (ucTemp[0] << 8) | ucTemp[1];
		vrntc = (ucTemp[2] << 8) | ucTemp[3];
		rntc = ((float)vrntc)*((float)100000)/((float)vref);
		temp = log(rntc/10000.0);
		temp /= 3380.0;
		temp += 1.0/(25.0 + 273.15);
		temp = 1.0/temp;
		temp -= 273.15;
		*Tempf = temp;
	}
}
