## Overview
* Blink with default LED

## Code
```main.c
#include "mbed.h"

DigitalOut led(PC_13);

int main()
{
    printf("hello, Mbed.\n");
    while (1) {
        led = !led;
        wait(1.0);
    }
}
```
