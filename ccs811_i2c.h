#ifndef CCS811_H
#define CCS811_H

// Original Written by Larry Bank - 11/4/2017

int ccs811Init(int iChannel, int iAddr);
int ccs811SetCalibration(float fTemp, float fHumid);
int ccs811ReadValues(int *eCO2, int *TVOC, unsigned char data[5]);
void ccs811Shutdown(void);
int ccs811ReadTemp(float *Temp);
int bme280Init(int jChannel, int jAddr, unsigned char data[33]);
int bme280ReadID(char *id);
int bme280LoadCalibration(unsigned char data[33]);
int bme280Read(float *bme280Temp, float *bme280Press, float *bme280Hum, unsigned char data[8]);

#endif 
