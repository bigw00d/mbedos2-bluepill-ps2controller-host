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

#ifdef SPEC_PSFOUR
//4 bytes for Direction X, Y, Z, Rotate-Z(0 to 255)
   report.data[count] = _dirx & 0xff;            
   count++;         
   report.data[count] = _diry & 0xff;
   count++;
   report.data[count] = _dirz & 0xff;            
   count++;         
   report.data[count] = _rotz & 0xff;
   count++;

//4 bits for Hat Switch(0 to 7)
//14 bits for Buttons(0 to 1)
//6 bits for unk(dummy)
   report.data[count] = ((_buttons & 0x0f) << 4) | (_hat & 0x0f) ; //buttons(4bit) + hat(4bit)
   count++;
   report.data[count] = (_buttons >>  4) & 0xff;   //buttons(8bit)
   count++;
   report.data[count] = (_buttons >>  12) & 0x03;  //buttons(2bit)
   count++;

//2 bytes for Rotate-X, Y(0 to 255)
   report.data[count] = _rotx & 0xff;
   count++;
   report.data[count] = _roty & 0xff;
   count++;

   report.length = count; 
#else
//Use 8 bits for buttons   
   report.data[count] = (_buttons & 0xff) ;                                         
   count++;

//Use 2 bytes for move(x,y)
   report.data[count] = _x & 0xff;            
   count++;         
   report.data[count] = _y & 0xff;
   count++;

   report.length = count; 
#endif // SPEC_PSFOUR

   return send(&report);
}

bool USBJoystickMouse::joypadUpdate(int16_t x, int16_t y, uint32_t buttons) {
   _x = x;
   _y = y;
   _buttons = buttons;
   return sendJoyPadReport();
}

bool USBJoystickMouse::joypadUpdate(JoyPadStruct joypad) {
   _buttons = joypad.buttons;
   _dirx = joypad.dirx;
   _diry = joypad.diry;
   _dirz = joypad.dirz;
   _rotx = joypad.rotx;
   _roty = joypad.roty;
   _rotz = joypad.rotz;
   _hat = joypad.hat;

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
   _dirx = 0;
   _diry = 0;
   _dirz = 0;
   _rotx = 0;
   _roty = 0;
   _rotz = 0;
   _hat = 0;
}

uint8_t * USBJoystickMouse::reportDesc() {    
      static uint8_t reportDescriptor[] = {
      #ifdef SPEC_PSFOUR
         // USAGE_PAGE(1), 0x01,           // Generic Desktop           
         // USAGE(1), 0x04,                // Usage (Gamepad)
         // COLLECTION(1), 0x01,           // Application
         //    REPORT_ID(1),      REPORT_ID_CUSTOM_JOYSTICK,
         //    USAGE(1), 0x30,                 // X
         //    USAGE(1), 0x31,                 // Y
         //    USAGE(1), 0x32,                 // Z
         //    USAGE(1), 0x35,                 // Rotate-Z
         //       0x15, 0x00,                     // logical minimum = 0
         //       0x26, 0xFF, 0x00,               // logical maximum = 255
         //       0x75, 0x08,                     // report size = 8 bits
         //       0x95, 0x04,                     // report count = 4
         //       INPUT(1), 0x02,                 // Data, Variable, Absolute
         //    USAGE(1), 0x39,                 // Hat Switch
         //       0x15, 0x00,                     // logical minimum = 0
         //       0x25, 0x07,                     // logical maximum = 7
         //       0x35, 0x00,                     // Physical Minimum (0)
         //       0x46, 0x3B, 0x01,               // Physical Maximum (315)
         //       0x65, 0x14,                     // Unit (0x14)
         //       0x75, 0x04,                     // Report Size (4)
         //       0x95, 0x01,                     // Report Count (1)
         //       0x81, 0x42,                     // Input (Var, NullState)
         //       0x65, 0x00,                     // Unit (0x00)
      0x05,  0x01,  // Usage	Page	(Generic	Desktop	Controls)
      0x09,  0x05,  // Usage	(Gamepad)
      0xA1,  0x01,  // Collection	(Application)
         0x85,  0x01,  // Report	ID	(0x01)
         0x09,  0x30,  // Usage	(Direction-X)
         0x09,  0x31,  // Usage	(Direction-Y)
         0x09,  0x32,  // Usage	(Direction-Z)
         0x09,  0x35,  // Usage	(Rotate-Z)
            0x15,  0x00,  // Logical	Minimum	(0)
            0x26,  0xFF,    0x00,  // Logical	Maximum	(255)
            0x75,  0x08,  // Report	Size	(8)
            0x95,  0x04,  // Report	Count	(4)
            0x81,  0x02,  // Input	(Var)
         0x09,  0x39,  // Usage	(Hat	Switch)
            0x15,  0x00,  // Logical	Minimum	(0)
            0x25,  0x07,  // Logical	Maximum	(7)
            0x35,  0x00,  // Physical	Minimum	(0)
            0x46,  0x3B,    0x01,  // Physical	Maximum	(315)
            0x65,  0x14,  // Unit	(0x14)
            0x75,  0x04,  // Report	Size	(4)
            0x95,  0x01,  // Report	Count	(1)
            0x81,  0x42,  // Input	(Var,	NullState)
            0x65,  0x00,  // Unit	(0x00)
         0x05,  0x09,  // Usage	Page	(Buttons)
         0x19,  0x01,  // Usage	Minimum	(1)
         0x29,  0x0E,  // Usage	Maximum	(14)
            0x15,  0x00,  // Logical	Minimum	(0)
            0x25,  0x01,  // Logical	Maximum	(1)
            0x75,  0x01,  // Report	Size	(1)
            0x95,  0x0E,  // Report	Count	(14)
            0x81,  0x02,  // Input	(Var)
         0x06,  0x00,    0xFF,  // Usage	Page	(unk)
         0x09,  0x20,  // Usage	(0x20)
            0x75,  0x06,  // Report	Size	(6)
            0x95,  0x01,  // Report	Count	(1)
            0x15,  0x00,  // Logical	Minimum	(0)
            0x25,  0x7F,  // Logical	Maximum	(127)
            0x81,  0x02,  // Input	(Var)
         0x05,  0x01,  // Usage	Page	(Generic	Desktop	Controls)
            0x09,  0x33,  // Usage	(Rotate-X)
            0x09,  0x34,  // Usage	(Rotate-Y)
               0x15,  0x00,  // Logical	Minimum	(0)
               0x26,  0xFF,    0x00,  // Logical	Maximum	(255)
               0x75,  0x08,  // Report	Size	(8)
               0x95,  0x02,  // Report	Count	(2)
               0x81,  0x02,  // Input	(Var)
      END_COLLECTION(0),

      #else
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
      #endif // SPEC_PSFOUR

      };

      reportLength = sizeof(reportDescriptor);
      return reportDescriptor;
}
