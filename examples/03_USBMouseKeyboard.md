## Overview
* USB Mouse & Keyboard sample code

##  Pin connections
|BluePill  |USB Connector |
|---|---|
|5V  |VCC  |
|GND  |GND  |
|A11  |D-  |
|A12  |D+  |

## Code
```main.c
#include "stm32f103c8t6.h"
#include "mbed.h"
#include "USBMouseKeyboard.h"
 
int main() {
    confSysClock();         //Configure system clock (72MHz HSE clock, 48MHz USB clock)
    USBMouseKeyboard key_mouse;
    
    while (1) {
        key_mouse.printf("Hello World from Mbed\r\n");
        key_mouse.move(20, 0);
        wait(1);
    }
}
```
