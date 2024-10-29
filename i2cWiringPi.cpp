#include <iostream>
#include <errno.h>
#include <wiringPiI2C.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;

int main()
{
   int fd1, fd2;
   //int fool = NULL;
   char result, resultx, resulty;
   char alg_result[4], ntcreg[4];
   unsigned int result2,result3;
   int k = 0;
   float nrtc;

   // Initialize the interface by giving it an external device ID.
   // The MCP4725 defaults to address 0x60.   
   //
   // It returns a standard file descriptor.
   // 
   fd1 = wiringPiI2CSetup(0x77);  // BME280
   fd2 = wiringPiI2CSetup(0x5A);  // CCS811

   cout << "BME280: "<< fd1 << endl;

   result = wiringPiI2CReadReg8(fd1,0xD0);
   cout << "id: " << 0x40 + (result >> 4) << endl;
/*
   result = wiringPiI2CReadReg8(fd1,0xFA);
   cout << "Temp reg 1: " << result + 0x40 << endl;
   printf("%c\n",result+0x40);
   result = wiringPiI2CReadReg8(fd1,0xFB);
   cout << "Temp reg 2: " << result + 0x40 << endl;
   printf("%c\n",result+0x40);

   result = wiringPiI2CReadReg8(fd1,0xFD);
   cout << "Hum reg 1: " << result + 0x40 << endl;
   result = wiringPiI2CReadReg8(fd1,0xFE);
   cout << "Hum reg 2: " << result + 0x40 << endl;

   result = wiringPiI2CReadReg8(fd1,0xF7);
   cout << "Press reg 1: " << result + 0x40 << endl;
   result = wiringPiI2CReadReg8(fd1,0xF8);
   cout << "Press reg 2: " << result + 0x40 << endl;
*/
   cout << "CCS811: " << fd2 << endl;

   //result2 = wiringPiI2CReadReg16(fd2,0x03);
   //cout << "raw data: " << result2 << endl;
   result = wiringPiI2CReadReg8(fd2,0x20);
   cout << "HW_ID: " << 0x40 + ((result & 0xF0) >> 4) << endl;
   cout << "HW_ID: " << 0x40 + (result & 0x0F) << endl;
   sleep(1);
   result = wiringPiI2CReadReg8(fd2,0x00);
   cout << "Status 1: " << 0x40 + ((result & 0xF0) >> 4) << endl;
   cout << "Status 2: " << 0x40 + (result & 0x0F) << endl;
   sleep(1);
   result = wiringPiI2CReadReg8(fd2,0x01);
   cout << "Mode 1: " << 0x40 + ((result & 0xF0) >> 4) << endl;
   cout << "Mode 2: " << 0x40 + (result & 0x0F) << endl;
   //sleep(1);
   //result = wiringPiI2CWriteReg8(fd2,0xF4,0);
   sleep(1);
   result = wiringPiI2CReadReg8(fd2,0x00);
   cout << "Status 1 :" << 0x40 + ((result & 0xF0) >> 4) << endl;
   cout << "Status 2 :" << 0x40 + (result & 0x0F) << endl;
   //result = wiringPiI2CWriteReg8(fd2,0xF4,0xAA);
   while (k < 5){
   	sleep(3);
	result = wiringPiI2CReadReg8(fd2,0x00);
   	if (result & 0x08) {
		cout << "k = " << k << endl;
	
		alg_result[0] = wiringPiI2CReadReg8(fd2,0x02);
  	 	alg_result[1] = wiringPiI2CReadReg8(fd2,0x02);
   		alg_result[2] = wiringPiI2CReadReg8(fd2,0x02);
   		alg_result[3] = wiringPiI2CReadReg8(fd2,0x02);
   		result2 = (alg_result[0] << 8) + alg_result[1];
                cout << "[0] MSB: " << 0x40 + ((alg_result[0] & 0xF0) >> 4) << endl;
                cout << "[0] LSB: " << 0x40 + (alg_result[0] & 0x0F) << endl;
		cout << "[1] MSB: " << 0x40 + ((alg_result[1] & 0xF0) >> 4) << endl;
		cout << "[1] LSB: " << 0x40 + (alg_result[1] & 0x0F) << endl; 
   		cout << "eCO2: " << result2 << endl;
	   	result2 = (alg_result[2] << 8) + alg_result[3];
   		cout << "TVOC: " << result2 << endl;
		
		ntcreg[0] = wiringPiI2CReadReg8(fd2,0x06);
		ntcreg[1] = wiringPiI2CReadReg8(fd2,0x06);
		ntcreg[2] = wiringPiI2CReadReg8(fd2,0x06);
		ntcreg[3] = wiringPiI2CReadReg8(fd2,0x06);
		result2 = (ntcreg[0] << 8) | ntcreg[1];
		result3 = (ntcreg[2] << 8) | ntcreg[3];
		nrtc = ((float)result3)*((float)100000)/((float)result2);
		cout << "nrtc: " << nrtc << endl;
   	}
        ++k;
   }
   result = wiringPiI2CReadReg8(fd2,0x00);
   cout << "Status 1: " << 0x40 + ((result & 0xF0) >> 4) << endl;
   cout << "Status 2: " << 0x40 + (result & 0x0F) << endl;
   sleep(1);
   result = wiringPiI2CReadReg8(fd2,0xE0);
   cout << "Error 1 :" << 0x40 + ((result & 0xF0) >> 4) << endl;
   cout << "Error 2 :" << 0x40 + (result & 0x0F) << endl;
   //result2 = wiringPiI2CReadReg16(fd2,0x02);
   //cout << "data: " << result2 << endl;

   return 0;
}

