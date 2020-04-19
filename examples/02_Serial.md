## Overview
* Serial out with LED Blink

##  Pin connections
|BluePill  |USB Serial Board  |
|---|---|
|GND  |GND  |
|A2  |RX  |

## Code
```main.c
#include "mbed.h"

DigitalOut led(PC_13);
Serial      pc(PA_2, PA_3); // TX, RX

int main()
{
    pc.baud(115200);
    printf("hello, Mbed.\n");
    while (1) {
        led = !led;
        wait(1.0);
        pc.printf("Blink.\n");
    }
}
```
