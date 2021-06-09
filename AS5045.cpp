/*
 * AS5045.h - Arduino library for AMS AS5045 magnetic rotary encoder chip
 * version 1.0 2014-04-22
 *
 * Copyright (c) 2014 Dash (Wenchang Zhang).  All rights reserved.
 *
 
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  A copy of the GNU Lesser General Public License, <http://www.gnu.org/licenses/>.
 *
*/ 


//Mark's original statement:
/*
 * AS5040.h - Arduino library for AMS AS5040 magnetic rotary encoder chip
 * version 1.0 2014-03-05
 *
 * Copyright (c) 2014 Mark Tillotson.  All rights reserved.
 *
 * This file is part of "Mark's AS5040 library for Arduino"
 *
 *  "Mark's AS5040 library for Arduino" is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  "Mark's AS5040 library for Arduino" is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with "Mark's AS5040 library for Arduino".
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "AS5045.h"

AS5045::AS5045 (byte pinCS, byte pinCLK, byte pinDO, byte pinPROG, unsigned int clockDelay) 
{
  _pinCS   = pinCS ;
  _pinCLK  = pinCLK ;
  _pinDO   = pinDO ;
  _pinPROG = pinPROG ;
  _clockDelay = clockDelay;

  _status  = 0xFF ;  // invalid status
}

/*
if you use this function to program OTP,
The OTP would be set as default mode, no reverse, zero offset,
since OTP is a one-time stuff, I personally don't suggest use this function to program OTP,
unless you don't want no one including yourself to change the OTP afterwards.
*/
boolean AS5045::begin ( )
{
	if(_pinPROG == 0xFF)
		return init();
	else
		return progOTP (0) ;
}

boolean AS5045::begin (int mag_offset)
{
	if(_pinPROG == 0xFF)
	{
		return init();
		_mag_offset = mag_offset;
	}
	else
		return progOTP (0) ;
}

//program OTP but only set the mode byte
boolean AS5045::progOTP (byte mode)
{
  return progOTP (mode, false, 0) ;
}

//use this function to directly program the OTP
boolean AS5045::progOTP (byte mode, boolean reverse, unsigned int offset)
{
//configuration 16 BIT: CCW + Z[11:0] + PWM disable + MagCompEn + PWMhalfEn
//mode : PWM disable + MagCompEn + PWMhalfEn
  int config_word =
    (reverse ? 0x8000 : 0x0000) | 
    ((offset & 0xFFF) << 3) |
    (mode & 0x07) ;

  pinMode (_pinCLK, OUTPUT) ;  digitalWrite (_pinCLK, HIGH) ;
  if (_pinPROG != 0xFF)
  {
    pinMode (_pinPROG, OUTPUT) ; digitalWrite (_pinPROG, LOW) ;
  }
  pinMode (_pinCS, OUTPUT) ;   digitalWrite (_pinCS, LOW) ;
  pinMode (_pinDO, INPUT_PULLUP) ;

  byte count = 0 ;
  while (read (), (_status & AS5045_STATUS_OCF) == 0)
  {
    if (count > 30)
      return false ; // failed to initialize
    delay (1) ;
    count ++ ;
  }

  if (_pinPROG == 0xFF)
  {
    // no initial program sequence, we're done already
    return true ;
  }
  
  //begin program OTP register sequence
  digitalWrite (_pinCLK, LOW) ;
  digitalWrite (_pinCS, LOW) ;
  digitalWrite (_pinPROG, LOW) ;
  delayMicroseconds (5) ;
  
  digitalWrite (_pinPROG, HIGH) ;
  delayMicroseconds (5) ;
  digitalWrite (_pinCS, HIGH) ;
  delayMicroseconds (5) ;

  unsigned int MSK = 0x8000 ;
  for (byte i = 0 ; i < 16 ; i++)
  {
    digitalWrite (_pinPROG, (config_word & MSK) ? HIGH : LOW) ;
    MSK >>= 1 ;
    digitalWrite (_pinCLK, HIGH) ;
    delayMicroseconds (1) ;
    digitalWrite (_pinCLK, LOW) ;
    delayMicroseconds (1) ;
  }
  
  digitalWrite (_pinPROG, LOW) ;
  delayMicroseconds (1) ;
  digitalWrite (_pinCS, LOW) ;
  delayMicroseconds (1) ;
  digitalWrite (_pinCLK, LOW) ;
  return true ;
}

boolean AS5045::init ()
{

  pinMode (_pinCLK, OUTPUT);
  digitalWrite (_pinCLK, LOW);
  
  pinMode (_pinCS, OUTPUT);
  digitalWrite (_pinCS, LOW);
  
  //pinMode (_pinDO, INPUT_PULLUP);
  pinMode (_pinDO, INPUT);
  
  byte count = 0 ;
  
  while (read (), (_status & AS5045_STATUS_OCF) == 0)
  {
    Serial.print("status: ");
	Serial.println(_status, BIN);
    if (count > 30)
      return false ; // failed to initialize
    delay (1) ;
    count ++ ;
  }
  
  return true;
  
}

void AS5045::clock_cycle ()
{
  if (_clockDelay == 0)
    return;

  digitalWrite (_pinCLK, LOW) ;
  delayMicroseconds (_clockDelay) ;
  digitalWrite (_pinCLK, HIGH) ;
  delayMicroseconds (_clockDelay) ;
}

// read position value, squirrel away status
unsigned int AS5045::read ()
{
  digitalWrite (_pinCS, LOW) ;
  unsigned int value = 0 ;
  for (byte i = 0 ; i < 12 ; i++)
  {
    clock_cycle () ;
    value = (value << 1) | digitalRead (_pinDO) ;
  }
  byte status = 0 ;
  for (byte i = 0 ; i < 6 ; i++)
  {
    clock_cycle () ;
    status = (status << 1) | digitalRead (_pinDO) ;
  }
  digitalWrite (_pinCS, HIGH) ;
  _parity = even_parity (value >> 2) ^ even_parity (value & 3) ^ even_parity (status) ;
  _status = status >> 1 ;
  return value ;
}

// read position value with bias (with 4096/round support only), squirrel away status 
unsigned int AS5045::read_bias ()
{
  unsigned int value = 0 ;
  value = read();
  /*
  set the offset value and take care of the potential overflow
  change if->while if you really think the plus time cost is acceptable
  running -= / += twice would indicate a mistake in previous reading stage
  so I chose to use if instead of while
  */
  
  value += _mag_offset;
  
  if(value > 4096)
	value -= 4096;
	
  if(value < 0)
	value += 4096;
  
  return value ;
}

byte AS5045::even_parity (byte val)
{
  val = (val >> 1) ^ val ;
  val = (val >> 2) ^ val ;
  val = (val >> 4) ^ val ;
  return val & 1 ;
}


// raw status from latest read, 5 bit value
byte AS5045::status ()
{
  return _status ;
}

// indicate if latest status implies valid data
boolean AS5045::valid ()
{
  return _parity == 0 && (_status & 0x18) == 0x10 && (_status & 3) != 3 ;
}

// motion in the Z-axis, +1 means mag field increasing (magnet approaching chip),
// -1 is decreasing, 0 is stable.
int AS5045::Zaxis ()
{
  switch (_status & 0x3)
    {
    case 0: return 0 ;
    case 1: return -1 ;
    case 2: return +1 ;
    }
  return 0 ; // invalid case, must return something harmless
}
