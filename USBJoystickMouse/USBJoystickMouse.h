/*
 * USB Joystick and Mouse Library
 * Copyright (c) 2020 Daiki Yasuda
 */

#ifndef USBJOYMOUSE_H
#define USBJOYMOUSE_H

#include "USBHID.h"

#define SPEC_PSFOUR 1

#define REPORT_ID_CUSTOM_JOYSTICK 1
#define REPORT_ID_CUSTOM_MOUSE 2

/* Joypad Button Bit */
#define JOYPAD_BTN1    0x0001
#define JOYPAD_BTN2    0x0002
#define JOYPAD_BTN3    0x0004
#define JOYPAD_BTN4    0x0008
#define JOYPAD_BTN5    0x0010
#define JOYPAD_BTN6    0x0020
#define JOYPAD_BTN7    0x0040
#define JOYPAD_BTN8    0x0080
#define JOYPAD_BTN9    0x0100
#define JOYPAD_BTN10    0x0200
#define JOYPAD_BTN11    0x0400
#define JOYPAD_BTN12    0x0800
#define JOYPAD_BTN13    0x1000
#define JOYPAD_BTN14    0x2000

/* Mouse Button Bit */
#define MOUSE_LEFT      0x01
#define MOUSE_RIGHT     0x02
#define MOUSE_MIDDLE    0x04

typedef struct JoyPadStruct {
    uint8_t dirx;
    uint8_t diry;
    uint8_t dirz;
    uint8_t rotx;
    uint8_t roty;
    uint8_t rotz;
    uint8_t hat;
    uint32_t buttons;
} joypad_Type;

class USBJoystickMouse: public USBHID {
  public:

    /**
     *   Constructor
     *
     * @param vendor_id Your vendor_id (default: 0x1234)
     * @param product_id Your product_id (default: 0x0002)
     * @param product_release Your product_release (default: 0x0001)
     */
    // 8 buttons, no padding on buttons
    //     USBJoystickMouse(uint16_t vendor_id = 0x1234, uint16_t product_id = 0x0500, uint16_t product_release = 0x0001, int waitForConnect = true):
    // 8 buttons, no padding on buttons(baffalo)
    //     USBJoystickMouse(uint16_t vendor_id = 0x0583, uint16_t product_id = 0x2060, uint16_t product_release = 0x0001, int waitForConnect = true):
    // caution: set vendor_id & product_id aas you favorite controller
    // 14 buttons, use analog stick, use cross key as hat switch(psfour)
    USBJoystickMouse(uint16_t vendor_id = 0x054C, uint16_t product_id = 0x05C4, uint16_t product_release = 0x0001, int waitForConnect = true):
      USBHID(0, 0, vendor_id, product_id, product_release, false) {
        init();
        connect(waitForConnect);
        // mouse
        _button = 0;
      };

    /**
     * Move the cursor to (x, y), Press one or several buttons
     *
     * @param x-axis position
     * @param y-axis position
     * @param buttons buttons state
     * @returns true if there is no error, false otherwise
     */
    bool joypadUpdate(int16_t x, int16_t y, uint32_t buttons);

    bool joypadUpdate(JoyPadStruct joypad);

    /**
    * Press one or several buttons
    *
    * @param button button state (ex: mousePress(MOUSE_LEFT))
    * @returns true if there is no error, false otherwise
    */
    bool mousePress(uint8_t button);

    /**
    * Release one or several buttons
    *
    * @param button button state (ex: mouseRelease(MOUSE_LEFT))
    * @returns true if there is no error, false otherwise
    */
    bool mouseRelease(uint8_t button);

    /**
    * Write a state of the mouse
    *
    * @param x x-axis position
    * @param y y-axis position
    * @returns true if there is no error, false otherwise
    */
    bool mouseMove(int16_t x, int16_t y);

    /**
     * To define the report descriptor. Warning: this method has to store the length of the report descriptor in reportLength.
     *
     * @returns pointer to the report descriptor
     */
    virtual uint8_t * reportDesc();

    // public for test
    uint8_t _dirx;                       
    uint8_t _diry;     
    uint8_t _dirz;     
    uint8_t _rotx;                       
    uint8_t _roty;     
    uint8_t _rotz;     
    uint8_t _hat;     
    uint32_t _buttons;  // joypad
    bool sendJoyPadReport();

  private:
    int8_t _x;                       
    int8_t _y;     
    // uint32_t _buttons;  // joypad
    uint8_t _button;    // mouse
    void init();                 

    // bool sendJoyPadReport();
    bool sendMouseReport(int8_t x, int8_t y, uint8_t buttons, int8_t z);
    bool mouseUpdate(int16_t x, int16_t y, uint8_t buttons, int8_t z);

};

#endif