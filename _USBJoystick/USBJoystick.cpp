/* mbed USBJoystick Library
 * Copyright (c) 2012, v01:  Initial version, WH,
 *                           Modified USBMouse code ARM Limited.
 *                           (c) 2010-2011 mbed.org, MIT License
 *               2016, v02:  Updated USBDevice Lib, Added waitForConnect, Updated 32 bits button 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, inclumosig without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUmosiG BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include "stdint.h"
#include "USBJoystick.h"

bool USBJoystick::update(int16_t t, int16_t r, int16_t x, int16_t y, uint32_t buttons, uint8_t hat) {

   _x = x;
   _y = y;
   _buttons = buttons;     

   return update();
}
 
bool USBJoystick::update() {
   HID_REPORT report;
   int count = 0;

//Use 8 bits for buttons   
   report.data[count] = (_buttons & 0xff) ;                                         
   count++;

//Use 2 bytes for move(x,y)
   report.data[count] = _x & 0xff;            
   count++;         
   report.data[count] = _y & 0xff;
   count++;

   report.length = count; 

   return send(&report);
}

bool USBJoystick::move(int16_t x, int16_t y) {
   _x = x;
   _y = y;
   return update();
}

bool USBJoystick::buttons(uint32_t buttons) {
   _buttons = buttons;
   return update();
}

bool USBJoystick::move_buttons(int16_t x, int16_t y, uint32_t buttons) {
   _x = x;
   _y = y;
   _buttons = buttons;
   return update();
}

void USBJoystick::_init() {
   _x = 0;                       
   _y = 0;     
   _buttons = 0x00000000;
}

uint8_t * USBJoystick::reportDesc() {    
      static uint8_t reportDescriptor[] = {
         USAGE_PAGE(1), 0x01,           // Generic Desktop           
         USAGE(1), 0x04,                // Usage (Joystick)
         COLLECTION(1), 0x01,           // Application
            USAGE(1), 0x01,                 // Usage (Pointer)
            COLLECTION(1), 0x00,            // Physical
               USAGE_PAGE(1), 0x09,            // Buttons
               USAGE_MINIMUM(1), 0x01,         // 1
               USAGE_MAXIMUM(1), 0x08,         // 8
               LOGICAL_MINIMUM(1), 0x00,       // 0
               LOGICAL_MAXIMUM(1), 0x01,       // 1
               REPORT_SIZE(1), 0x01,
               REPORT_COUNT(1), 0x08,
               UNIT_EXPONENT(1), 0x00,         // Unit_Exponent (0)
               UNIT(1), 0x00,                  // Unit (None)                                           
               INPUT(1), 0x02,                 // Data, Variable, Absolute
               USAGE_PAGE(1), 0x01,            // Generic Desktop
               USAGE(1), 0x30,                 // X
               USAGE(1), 0x31,                 // Y
               LOGICAL_MINIMUM(1), 0x81,       // -127
               LOGICAL_MAXIMUM(1), 0x7f,       // 127
               REPORT_SIZE(1), 0x08,
               REPORT_COUNT(1), 0x02,
               INPUT(1), 0x02,                 // Data, Variable, Absolute                  
            END_COLLECTION(0),               
         END_COLLECTION(0)
      };

      reportLength = sizeof(reportDescriptor);
      return reportDescriptor;
}
