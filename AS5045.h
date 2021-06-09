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
 
#ifndef AS5045_h
#define AS5045_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

//define mode : PWM disable + MagCompEn + PWMhalfEn
#define AS5045_PWM_DIS 0x04
#define AS5045_MAG_COMP_EN 0x02
#define AS5045_PWM_HALF_EN 0xx01

// defines for 5 bit _status value
#define AS5045_STATUS_OCF 0x10
#define AS5045_STATUS_COF 0x08
#define AS5045_STATUS_LIN 0x04
#define AS5045_STATUS_MAGINC 0x02
#define AS5045_STATUS_MAGDEC 0x01

class AS5045
{
 public:
  AS5045 (byte pinCS, byte pinCLK, byte pinDO, byte pinPROG = 0xFF, unsigned int clockDelay = 0) ;

  boolean begin () ;
  boolean begin (int mag_offset) ;
  boolean progOTP (byte mode) ;
  boolean progOTP (byte mode, boolean reverse, unsigned int offset) ;
  boolean init ();

  unsigned int read () ;
  unsigned int read_bias () ;
  byte status () ;
  boolean valid () ;
  int Zaxis () ;

 private:

  byte _pinCLK ;
  byte _pinCS ;
  byte _pinDO ;
  byte _pinPROG ;
  unsigned int _clockDelay;

  byte _status ;
  byte _parity ;

  byte even_parity (byte val) ;
  void clock_cycle (void);
  
  int _mag_offset;
  
} ;


#endif
