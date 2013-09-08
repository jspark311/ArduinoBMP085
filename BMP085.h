/*
File:   BMP085.h
Author: J. Ian Lindsay
Date:   2013.08.08

I have adapted the code written by Jim Lindblom, and located at this URL:
https://www.sparkfun.com/tutorial/Barometric/BMP085_Example_Code.pde


This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef BMP085_H
#define BMP085_H
#define BMP085_ADDRESS 0x77

#ifdef __cplusplus

class BMP085 {
    public:
      double temperature;    // The present temperature reading, in degerees C.
      double pressure;       // The present pressure reading, in Pa.
      bool calibrated;       // Is the sensor ready for use?

      BMP085();
      void readSensor(void);  // This is a multi-step function.
      void calibrate(void);   // This must be called at least once. Also: idempotent.


    private:
      const uint16_t OSS = 3;  // Oversampling Setting
      int8_t read_step;        // Used to keep track of which stage of the read we are in.

      int16_t ac1;	// Calibration values
      int16_t ac2; 
      int16_t ac3; 
      uint16_t ac4;
      uint16_t ac5;
      uint16_t ac6;
      int16_t b1; 
      int16_t b2;
      int16_t mb;
      int16_t mc;
      int16_t md;

      // b5 is calculated in getTemperature(...), this variable is also used in getPressure(...)
      // so getTemperature(...) must be called before getPressure(...).
      long b5;
      long uncomp_temp;  // Uncompensated temperature.
      long uncomp_pres;  // Uncompensated pressure.

      char bmp085Read(unsigned char address);      // Read 1 byte from the BMP085 at 'address'
      int bmp085ReadInt(unsigned char address);    // Read 2 bytes from the BMP085

      void initiate_UT_read(void);
      long getTemperature(void);   // Given uncompensated temperature, return degrees Celcius.
      
      void initiate_UP_read(void);
      long read_up(void);
      long getPressure(void);      // Given uncompensated pressure, return Pascals.
};

#endif
#endif
