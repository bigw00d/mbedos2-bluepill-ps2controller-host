#include "stm32f103c8t6.h"
#include "mbed.h"
#include "USBJoystickMouse.h"
#include "PS_PAD.h"

#define ENEBLE_VERBOSE 0

#ifdef SPEC_PSFOUR
#define BUTTON_NUM (12)
#else
#define BUTTON_NUM (8)
#endif
#define COUNT_3_SEC (30)
#define COUNT_1_SEC (10)
#define WAIT_100MSEC (0.1)

/* Mouse X, Y move value */
#define MX_MIN_ABS    (-5)
#define MY_MIN_ABS    (-5)
#define MX_MAX_ABS    (5)
#define MY_MAX_ABS    (5)

#define MX_MAX_MOVE   (30)
#define MX_MIN_MOVE   (-30)
#define MY_MAX_MOVE   (30)
#define MY_MIN_MOVE   (-30)


/* Joypad X, Y limits */
#define JX_MIN_ABS    (-127)     /*!< The maximum value that we can move to the left on the x-axis */
#define JY_MIN_ABS    (-127)     /*!< The maximum value that we can move up on the y-axis */
#define JX_MAX_ABS    (127)      /*!< The maximum value that we can move to the right on the x-axis */
#define JY_MAX_ABS    (127)      /*!< The maximum value that we can move down on the y-axis */

DigitalOut led(PC_13); //BluePill Default LED
#if ENEBLE_VERBOSE
Serial      pc(PA_2, PA_3); // TX, RX
#endif
PS_PAD      ps2(PA_7, PA_6, PA_5, PB_6);  //mosi=CMD,miso=DAT,clk=CLK,ss=SEL
USBJoystickMouse joymouse;

static Timer timer;
static Ticker ticker;
static uint8_t ticks = 0;
static uint32_t longPushCnt = 0;
static uint32_t oldLongPushCnt = 0;

uint8_t hatTable[16] = {
	0x0F,	//	NEUTRAL(0b0000)    
	6,	//	LEFT(0b0001)       
	4,	//	DOWN(0b0010)       
	5,	//	DOWNLEFT(0b0011)  
	2,	//	RIGHT(0b0100)      
	0x0F,	//	---(0b0101)    
	3,	//	RIGHTDOWN(0b0110) 
	0x0F,	//	---(0b0111)    
	0,	//	UP(0b1000)         
	7,	//	LEFTUP(0b1001)    
	0x0F,	//	---(0b1010)    
	0x0F,	//	---(0b1011)    
	1,	//	UPRIGHT(0b1100)   
	0x0F,	//	---(0b1101)    
	0x0F,	//	---(0b1110)    
	0x0F,	//	---(0b1111)
};

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

int16_t moveMouseTable[16][2] = {
    {0         , 0           },	//	NEUTRAL(0b0000)    
    {MX_MIN_ABS, 0           },	//	LEFT(0b0001)       
    {0         , MY_MAX_ABS  },	//	DOWN(0b0010)       
    {MX_MIN_ABS, MY_MAX_ABS  },	//	DOWNLEFT(0b0011)  
    {MX_MAX_ABS, 0           },	//	RIGHT(0b0100)      
    {0         , 0           },	//	---(0b0101)    
    {MX_MAX_ABS, MY_MAX_ABS  },	//	RIGHTDOWN(0b0110) 
    {0         , 0           },	//	---(0b0111)    
    {0         , MY_MIN_ABS  },	//	UP(0b1000)         
    {MX_MIN_ABS, MY_MIN_ABS  },	//	LEFTUP(0b1001)    
    {0         , 0           },	//	---(0b1010)    
    {0         , 0           },	//	---(0b1011)    
    {MX_MAX_ABS, MY_MIN_ABS  },	//	UPRIGHT(0b1100)   
    {0         , 0           },	//	---(0b1101)    
    {0         , 0           },	//	---(0b1110)    
    {0         , 0           },	//	---(0b1111)    
};

uint32_t buttonFilter[BUTTON_NUM] = {
    0x0008,	//	PAD_START
    0x0001,	//	PAD_SELECT
    0x8000,	//	PAD_SQUARE
    0x4000,	//	PAD_X
    0x2000,	//	PAD_CIRCLE
    0x1000,	//	PAD_TRIANGLE
    0x0400,	//	PAD_R1
    0x0800,	//	PAD_L1
    0x0200,	//	PAD_R2
    0x0100,	//	PAD_L2
    0x0002,	//	PAD_LEFT_STICK
    0x0004	//	PAD_RIGHT_STICK
};

uint32_t ps2tojoypad(int ps2movebtn);

void joypadMode(void);
void mouseMode(void);

static void tickHandler( void ) {
    if (longPushCnt > oldLongPushCnt) {
        ticks++;
    }
    else {
        ticks = 0;
    }
    oldLongPushCnt = longPushCnt;
}

int main() {

    confSysClock();         //Configure system clock (72MHz HSE clock, 48MHz USB clock)

    #if ENEBLE_VERBOSE
    pc.baud(115200);
    pc.printf("hello, Mbed.\n");
    #endif
    ps2.init();

    led = 1; //led off

     __disable_irq( );
    ticker.attach_us( tickHandler, 100000 ); // reset timer every 100msec
     __enable_irq( );

    while(1)
    {
        joypadMode();
        mouseMode();
    }

}

void mouseMode() {

    uint8_t ps2move = 0;    
    int16_t x = 0;
    int16_t y = 0;
    int16_t dx = 0;
    int16_t dy = 0;
    uint32_t buttons = 0;    
    int ps2movebtn = 0;

    led = 0; //led on

    while(1)
    {
        // check ticks
        if(ticks >= COUNT_3_SEC) { // 3sec
            ticks = 0;
            break; // retire mode
        }

        ps2.poll();     

        // check button
        ps2movebtn = ps2.read(PS_PAD :: BUTTONS);
        buttons = ps2tojoypad(ps2movebtn);

        if( buttons & 0x0002 ) { // PAD_X
            // 左側がタッチされている状態
            joymouse.mousePress(MOUSE_LEFT);  // マウス左クリックをクリックした状態にする

        }
        else {
            // タッチされていない状態
            joymouse.mouseRelease(MOUSE_LEFT);  // マウス左クリックを解放した状態にする
        }

        if( buttons & 0x0001 ) { // PAD_CIRCLE
            // 右側がタッチされている状態
            joymouse.mousePress(MOUSE_RIGHT); // マウス右クリックをクリックした状態にする
        }
        else {
            // タッチされていない状態
            joymouse.mouseRelease(MOUSE_RIGHT); // マウス右クリックを解放した状態にする
        }

        if ( buttons & 0x0080 ) { // START
            if ( buttons & 0x0040 ) { // SELECT
                longPushCnt++;
            }
            else {
                longPushCnt = 0;
            }
        }
        else {
            longPushCnt = 0;
        }

        // check move
        ps2move = ps2.read_move();
        ps2move &= 0x0F;
        dx = moveMouseTable[ps2move][0];
        dy = moveMouseTable[ps2move][1];
        if (dx != 0) {
            if( ((x+dx) <= MX_MAX_MOVE) && ((x+dx) >= MX_MIN_MOVE) ) {
                x += dx;
            }
        }
        else {
            x = 0;
        }
        if (dy != 0) {
            if( ((y+dy) <= MY_MAX_MOVE) && ((y+dy) >= MY_MIN_MOVE) ) {
                y += dy;
            }
        }
        else {
            y = 0;
        }
        joymouse.mouseMove(x, y);
        wait(0.05);
    }
}

void joypadMode() {

    uint8_t ps2move = 0;    
    int16_t x = 0;
    int16_t y = 0;
    uint32_t buttons = 0;    
    int ps2movebtn = 0;

    led = 1; //led off

    uint32_t _buttons;  // joypad

    // wait(10.0); //+:test

    JoyPadStruct joypad;
    Ps2AnalogStruct analog;

    while(1)
    {

        // check ticks
        if(ticks >= COUNT_3_SEC) { // 3sec
            ticks = 0;
            break; // retire mode
        }

        ps2.poll();     

        // check button
        ps2movebtn = ps2.read(PS_PAD :: BUTTONS);

        #ifdef  SPEC_PSFOUR
        joypad.buttons = ps2tojoypad(ps2movebtn);
        #if ENEBLE_VERBOSE
        pc.printf("ps2movebtn: %08X\n", ps2movebtn);
        pc.printf("joypad.buttons: %08X\n", joypad.buttons);
        #endif

        if ( buttons & 0x0080 ) { // START
            if ( buttons & 0x0040 ) { // SELECT
                longPushCnt++;
            }
            else {
                longPushCnt = 0;
            }
        }
        else {
            longPushCnt = 0;
        }

        // check move
        ps2move = ps2.read_move();
        ps2move &= 0x0F;
        joypad.hat = hatTable[ps2move];
        analog = ps2.read_analog();

        joypad.dirx = analog.lx;
        joypad.diry = analog.ly;
        joypad.rotz = analog.ry;
        // PAD_R2:0x0200
        if ((ps2movebtn & 0x0200) > 0) {
            joypad.roty = 255;
        }
        else {
            joypad.roty = 0;
        }
        // PAD_L2:0x0100
        if ((ps2movebtn & 0x0100) > 0) {
            joypad.rotx = 255;
        }
        else {
            joypad.rotx = 0;
        }
        joypad.dirz = analog.rx;
        joymouse.joypadUpdate(joypad);
        wait(0.05);

        #else

        buttons = ps2tojoypad(ps2movebtn);
        // pc.printf("buttons: %08X\n", buttons);

        if ( buttons & 0x0080 ) { // START
            if ( buttons & 0x0040 ) { // SELECT
                longPushCnt++;
            }
            else {
                longPushCnt = 0;
            }
        }
        else {
            longPushCnt = 0;
        }

        // check move
        ps2move = ps2.read_move();
        ps2move &= 0x0F;
        x = moveTable[ps2move][0];  // value -127 .. 128
        y = moveTable[ps2move][1];  // value -127 .. 128
        joymouse.joypadUpdate(x, y, buttons);
        wait(0.05);
        #endif
    }

}

uint32_t ps2tojoypad(int ps2movebtn) {
    int i;
    uint32_t targetButton = 0;
    uint32_t joypadbtn = 0;
    uint32_t ps2btn = ps2movebtn & ~(0x0080 | 0x0040 | 0x0020 | 0x0010); // remove move key :(PAD_LEFT | PAD_BOTTOM | PAD_RIGHT | PAD_TOP)

    for ( i=0; i<BUTTON_NUM; i++ ) {
        targetButton = ps2btn & buttonFilter[i];
        #ifdef SPEC_PSFOUR
        switch (targetButton) {
            // PAD_START:0x0008
            case 0x0008:
                joypadbtn |= JOYPAD_BTN10;
                break;
            // PAD_SELECT:0x0001
            case 0x0001:
                joypadbtn |= JOYPAD_BTN9;
                break;
            // PAD_SQUARE:0x8000
            case 0x8000:
                joypadbtn |= JOYPAD_BTN1;
                break;
            // PAD_X:0x4000
            case 0x4000:
                joypadbtn |= JOYPAD_BTN2;
                break;
            // PAD_CIRCLE:0x2000
            case 0x2000:
                joypadbtn |= JOYPAD_BTN3;
                break;
            // PAD_TRIANGLE:0x1000
            case 0x1000:
                joypadbtn |= JOYPAD_BTN4;
                break;
            // PAD_R1:0x0400
            case 0x0400:
                joypadbtn |= JOYPAD_BTN5;
                break;
            // PAD_L1:0x0800
            case 0x0800:
                joypadbtn |= JOYPAD_BTN6;
                break;
            // PAD_R2:0x0200
            case 0x0200:
                joypadbtn |= JOYPAD_BTN8;
                break;
            // PAD_L2:0x0100
            case 0x0100:
                joypadbtn |= JOYPAD_BTN7;
                break;
            // PAD_LEFT_STICK:0x0002
            case 0x0002:
                joypadbtn |= JOYPAD_BTN11;
                break;
            // PAD_RIGHT_STICK:0x0004
            case 0x0004:
                joypadbtn |= JOYPAD_BTN12;
                break;
            default:
                ;
                break;
        }
        #else
        switch (targetButton) {
            // PAD_START:0x0008 -> usbPadButton: 0x0080
            case 0x0008:
                joypadbtn |= JOYPAD_BTN8;
                break;
            // PAD_SELECT:0x0001 -> usbPadButton: 0x0040
            case 0x0001:
                joypadbtn |= JOYPAD_BTN7;
                break;
            // PAD_SQUARE:0x8000 -> usbPadButton: 0x0008
            case 0x8000:
                joypadbtn |= JOYPAD_BTN4;
                break;
            // PAD_X:0x4000 -> usbPadButton: 0x0002
            case 0x4000:
                joypadbtn |= JOYPAD_BTN2;
                break;
            // PAD_CIRCLE:0x2000 -> usbPadButton: 0x0001
            case 0x2000:
                joypadbtn |= JOYPAD_BTN1;
                break;
            // PAD_TRIANGLE:0x1000 -> usbPadButton: 0x0004
            case 0x1000:
                joypadbtn |= JOYPAD_BTN3;
                break;
            // PAD_R1:0x0400 -> usbPadButton: 0x0020
            case 0x0400:
                joypadbtn |= JOYPAD_BTN6;
                break;
            // PAD_L1:0x0800 -> usbPadButton: 0x0010
            case 0x0800:
                joypadbtn |= JOYPAD_BTN5;
                break;
            default:
                ;
                break;
        }
        #endif
    }

    return joypadbtn;
}