/*
 * USB Joystick and Mouse Library
 * Copyright (c) 2020 Daiki Yasuda
 */

#include "stdint.h"
#include "USBJoystickMouse.h"
 
bool USBJoystickMouse::sendJoyPadReport() {
   HID_REPORT report;
   int count = 0;

   report.data[count] = REPORT_ID_CUSTOM_JOYSTICK;
   count++;

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

bool USBJoystickMouse::joypadUpdate(int16_t x, int16_t y, uint32_t buttons) {
   _x = x;
   _y = y;
   _buttons = buttons;
   return sendJoyPadReport();
}

bool USBJoystickMouse::mouseRelease(uint8_t button) {
    _button = (_button & (~button)) & 0x07;
    return mouseUpdate(0, 0, _button, 0);
}

bool USBJoystickMouse::mousePress(uint8_t button) {
    _button = button & 0x07;
    return mouseUpdate(0, 0, button, 0);
}

bool USBJoystickMouse::mouseUpdate(int16_t x, int16_t y, uint8_t button, int8_t z) {
   while (x > 127) {
      if (!sendMouseReport(127, 0, button, z)) return false;
      x = x - 127;
   }
   while (x < -128) {
      if (!sendMouseReport(-128, 0, button, z)) return false;
      x = x + 128;
   }
   while (y > 127) {
      if (!sendMouseReport(0, 127, button, z)) return false;
      y = y - 127;
   }
   while (y < -128) {
      if (!sendMouseReport(0, -128, button, z)) return false;
      y = y + 128;
   }
   return sendMouseReport(x, y, button, z);
}

bool USBJoystickMouse::mouseMove(int16_t x, int16_t y) {
    return mouseUpdate(x, y, _button, 0);
}

bool USBJoystickMouse::sendMouseReport(int8_t x, int8_t y, uint8_t buttons, int8_t z) {
    HID_REPORT report;
    report.data[0] = REPORT_ID_CUSTOM_MOUSE;
    report.data[1] = buttons & 0x07;
    report.data[2] = x;
    report.data[3] = y;
    report.data[4] = -z; // >0 to scroll down, <0 to scroll up

    report.length = 5;

    return send(&report);
}

void USBJoystickMouse::init() {
   _x = 0;                       
   _y = 0;     
   _buttons = 0x00000000;
}

uint8_t * USBJoystickMouse::reportDesc() {    
      static uint8_t reportDescriptor[] = {
         // Joypad
         USAGE_PAGE(1), 0x01,           // Generic Desktop           
         USAGE(1), 0x04,                // Usage (Joystick)
         COLLECTION(1), 0x01,           // Application
            USAGE(1), 0x01,                 // Usage (Pointer)
            COLLECTION(1), 0x00,            // Physical
               REPORT_ID(1),      REPORT_ID_CUSTOM_JOYSTICK,
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
         END_COLLECTION(0),

         // Mouse
         USAGE_PAGE(1),      0x01,           // Generic Desktop
         USAGE(1),           0x02,           // Mouse
         COLLECTION(1),      0x01,           // Application
            USAGE(1),           0x01,           // Pointer
            COLLECTION(1),      0x00,           // Physical
               REPORT_ID(1),       REPORT_ID_CUSTOM_MOUSE,
               REPORT_COUNT(1),    0x03,
               REPORT_SIZE(1),     0x01,
               USAGE_PAGE(1),      0x09,           // Buttons
               USAGE_MINIMUM(1),       0x1,
               USAGE_MAXIMUM(1),       0x3,
               LOGICAL_MINIMUM(1),     0x00,
               LOGICAL_MAXIMUM(1),     0x01,
               INPUT(1),           0x02,
               REPORT_COUNT(1),    0x01,
               REPORT_SIZE(1),     0x05,
               INPUT(1),           0x01,
               REPORT_COUNT(1),    0x03,
               REPORT_SIZE(1),     0x08,
               USAGE_PAGE(1),      0x01,
               USAGE(1),           0x30,           // X
               USAGE(1),           0x31,           // Y
               USAGE(1),           0x38,           // scroll
               LOGICAL_MINIMUM(1),     0x81,
               LOGICAL_MAXIMUM(1),     0x7f,
               INPUT(1),           0x06,
            END_COLLECTION(0),
         END_COLLECTION(0),

      };

      reportLength = sizeof(reportDescriptor);
      return reportDescriptor;
}
