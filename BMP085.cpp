/*
File:   BMP085.cpp
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

#ifndef I2C_T3_H
  #include <i2c_t3.h>
  #ifdef I2C_DEBUG
      #include <rbuf.h> // linker fix
  #endif
#endif


#ifndef BMP085_H
    #include <BMP085/BMP085.h>
#endif


BMP085::BMP085() {
  this->temperature  = 0;      // Number is <deg-C>
  this->pressure     = 0;      // Number is in Pascals
  this->read_step    = -1;     // No reading in progress.
  this->calibrated   = false;  // We don't want to do this until the i2c bus is ready.
}


/**
* This is meant to be called from the main program.
*/
void BMP085::readSensor(void) {
  // Depending on which stage of the read we are on, jump to the next stage.
  if (!this->calibrated) {
    this->calibrate();
  }

  switch(this->read_step) {
    case -1:
      this->initiate_UT_read();
      this->read_step++;
      break;
    case 0:
      // Read two bytes from registers 0xF6 and 0xF7
      this->uncomp_temp = bmp085ReadInt(0xF6);
      this->temperature = ((double) this->getTemperature() / 10.0);
      this->initiate_UP_read();
      this->read_step++;
      break;
    case 1:
      this->read_up();
      this->pressure = ((double) this->getPressure() / 1000);
      this->read_step = -1;
      break;
    default:
      // We should never see this block.
      this->read_step = -1;
      break;
  }
}


// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
void BMP085::calibrate(void) {
  if (!this->calibrated) {
    this->ac1 = this->bmp085ReadInt(0xAA);
    this->ac2 = this->bmp085ReadInt(0xAC);
    this->ac3 = this->bmp085ReadInt(0xAE);
    this->ac4 = this->bmp085ReadInt(0xB0);
    this->ac5 = this->bmp085ReadInt(0xB2);
    this->ac6 = this->bmp085ReadInt(0xB4);
    this->b1  = this->bmp085ReadInt(0xB6);
    this->b2  = this->bmp085ReadInt(0xB8);
    this->mb  = this->bmp085ReadInt(0xBA);
    this->mc  = this->bmp085ReadInt(0xBC);
    this->md  = this->bmp085ReadInt(0xBE);
    this->calibrated = true;
  }
}


// Calculate temperature given ut.
// Value returned will be in units of 0.1 deg C
long BMP085::getTemperature() {
  long x1, x2;

  x1 = (((long)this->uncomp_temp - (long)this->ac6)*(long)this->ac5) >> 15;
  x2 = ((long)this->mc << 11)/(x1 + this->md);
  this->b5 = x1 + x2;

  return ((this->b5 + 8)>>4);  
}


// Calculate pressure given up
// calibration values must be known
// b5 is also required so getTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
long BMP085::getPressure() {
  long x1, x2, x3, b3, b6, p;
  unsigned long b4, b7;

  b6 = this->b5 - 4000;
  // Calculate B3
  x1 = (this->b2 * (b6 * b6)>>12)>>11;
  x2 = (this->ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((long)this->ac1)*4 + x3)<<this->OSS) + 2)>>2;
  
  // Calculate B4
  x1 = (this->ac3 * b6)>>13;
  x2 = (this->b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (this->ac4 * (unsigned long)(x3 + 32768))>>15;
  
  b7 = ((unsigned long)(this->uncomp_pres - b3) * (50000>>this->OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;
    
  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;
  
  return p;
}


// Read 1 byte from the BMP085 at 'address'
char BMP085::bmp085Read(unsigned char address) {
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.send(address);
  Wire.endTransmission();
  
  Wire.requestFrom(BMP085_ADDRESS, 1);
  while(!Wire.available());  // Horrid waste of time.
    
  return Wire.receive();
}


// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int BMP085::bmp085ReadInt(unsigned char address) {
  unsigned char msb, lsb;
  
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.send(address);
  Wire.endTransmission();
  
  Wire.requestFrom(BMP085_ADDRESS, 2);
  while(Wire.available()<2);  // Horrid waste of time.
  
  msb = Wire.receive();
  lsb = Wire.receive();
  
  return (int) msb<<8 | lsb;
}


// Read the uncompensated temperature value
void BMP085::initiate_UT_read() {
  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.send(0xF4);
  Wire.send(0x2E);
  Wire.endTransmission();
}


void BMP085::initiate_UP_read() {
  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.send(0xF4);
  Wire.send(0x34 + (this->OSS<<6));
  Wire.endTransmission();
}


// Read the uncompensated pressure value
long BMP085::read_up() {
  unsigned char msb, lsb, xlsb;
  
  // Wait for conversion, delay time dependent on OSS
  //delay(2 + (3<<this->OSS));
  
  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.send(0xF6);
  Wire.endTransmission();
  Wire.requestFrom(BMP085_ADDRESS, 3);
  
  // Wait for data to become available
  while(Wire.available() < 3)
    ;
  msb = Wire.receive();
  lsb = Wire.receive();
  xlsb = Wire.receive();
  
  this->uncomp_pres = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-this->OSS);

  return this->uncomp_pres;
}


