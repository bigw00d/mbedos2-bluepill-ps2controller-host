#include "stm32f103c8t6.h"
#include "mbed.h"
#include "USBJoystick.h"
#include "PS_PAD.h"

Serial      pc(PA_2, PA_3); // TX, RX

PS_PAD      ps2(PA_7, PA_6, PA_5, PB_6);  //mosi=CMD,miso=DAT,clk=CLK,ss=SEL

USBJoystick joystick;

int16_t moveTable[16][2] = {
    {0         , 0           },	//	NEUTRAL(0b0000)    
    {JX_MIN_ABS, 0           },	//	LEFT(0b0001)       
    {0         , JY_MAX_ABS  },	//	DOWN(0b0010)       
    {JX_MIN_ABS, JY_MAX_ABS  },	//	DOWNLEFT(0b0011)  
    {JX_MAX_ABS, 0           },	//	RIGHT(0b0100)      
    {0         , 0           },	//	---(0b0101)    
    {JX_MAX_ABS, JY_MAX_ABS  },	//	RIGHTDOWN(0b0110) 
    {0         , 0           },	//	---(0b0111)    
    {0         , JY_MIN_ABS  },	//	UP(0b1000)         
    {JX_MIN_ABS, JY_MIN_ABS  },	//	LEFTUP(0b1001)    
    {0         , 0           },	//	---(0b1010)    
    {0         , 0           },	//	---(0b1011)    
    {JX_MAX_ABS, JY_MIN_ABS  },	//	UPRIGHT(0b1100)   
    {0         , 0           },	//	---(0b1101)    
    {0         , 0           },	//	---(0b1110)    
    {0         , 0           },	//	---(0b1111)    
};

uint32_t ps2tojoypad(int ps2btn);

int main() {
    uint8_t ps2move = 0;    
    int16_t x = 0;
    int16_t y = 0;
    uint32_t buttons = 0;    
    int ps2btn = 0;
    int state = 0; //test 

    confSysClock();         //Configure system clock (72MHz HSE clock, 48MHz USB clock)

    pc.baud(115200);
    printf("hello, Mbed.\n");
    ps2.init();

    while(1)
    {   
        // button check
        // buttons = (0x01 << state);   // value    0 .. 7, one bit per button  
        // joystick.buttons(buttons);
        // pc.printf("%d buttons: %08x\n",state, buttons);
        // state = (state + 1) % 32;
        // wait(1);

        ps2.poll();     

        // check button
        ps2btn = ps2.read(PS_PAD :: BUTTONS);
        buttons = ps2tojoypad(ps2btn);
        if (ps2btn != 0x0000) {
            pc.printf("ps2btn: %04x ",ps2btn);
            pc.printf("buttons: %04x \n",buttons);
        }

        joystick.buttons(buttons);
        
        // check move
        ps2move = ps2.read_move();
        ps2move &= 0x0F;
        x = moveTable[ps2move][0];  // value -127 .. 128
        y = moveTable[ps2move][1];  // value -127 .. 128
        // pc.printf("ps2move: %x ",ps2move);
        // pc.printf("x: %d ",x);
        // pc.printf("y: %d \n",y);
        joystick.move(x, y);

        joystick.update();
        wait(0.05);
    }

}


uint32_t ps2tojoypad(int ps2btn) {
    uint32_t joypadbtn = 0;
    switch (ps2btn) {
        // PAD_START:0x0008 -> ps2btn: 0x0080
        case 0x0008:
            joypadbtn = 0x0080;
            break;
        // PAD_SELECT:0x0001 -> ps2btn: 0x0040
        case 0x0001:
            joypadbtn = 0x0040;
            break;
        // PAD_SQUARE:0x8000 -> ps2btn: 0x0008
        case 0x8000:
            joypadbtn = 0x0008;
            break;
        // PAD_X:0x4000 -> ps2btn: 0x0002
        case 0x4000:
            joypadbtn = 0x0002;
            break;
        // PAD_CIRCLE:0x2000 -> ps2btn: 0x0001
        case 0x2000:
            joypadbtn = 0x0001;
            break;
        // PAD_TRIANGLE:0x1000 -> ps2btn: 0x0004
        case 0x1000:
            joypadbtn = 0x0004;
            break;
        // PAD_R1:0x0400 -> ps2btn: 0x0020
        case 0x0400:
            joypadbtn = 0x0020;
            break;
        // PAD_L1:0x0800 -> ps2btn: 0x0010
        case 0x0800:
            joypadbtn = 0x0010;
            break;
        default:
            ;
            break;
    }
    return joypadbtn;
}