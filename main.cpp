#include "mbed.h"

// DigitalOut led(LED1);
DigitalOut led(PC_13);
// Serial      pc(PA_2, PA_3); // TX, RX

int main()
{
    printf("hello, Mbed.\n");
    while (1) {
        led = !led;
        // wait(0.5);
        wait(3.0);
        // printf("Blink.\n");
    }
}
