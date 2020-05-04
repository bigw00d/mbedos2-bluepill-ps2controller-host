#include "stm32f103c8t6.h"
#include "mbed.h"
#include "USBJoystick.h"
#include "PS_PAD.h"
#include "USBMouseKeyboard.h"

#define BUTTON_NUM 8
#define COUNT_3_SEC 30

DigitalOut led(PC_13);
Serial      pc(PA_2, PA_3); // TX, RX
// PS_PAD      ps2(PA_7, PA_6, PA_5, PB_6);  //mosi=CMD,miso=DAT,clk=CLK,ss=SEL
// USBJoystick joystick;

static Timer timer;
static Ticker ticker;
static uint8_t ticks = 0;
static uint32_t longPushCnt = 0;
static uint32_t oldLongPushCnt = 0;

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

#define MX_MIN_ABS    (-10)
#define MY_MIN_ABS    (-10)
#define MX_MAX_ABS    (20)
#define MY_MAX_ABS    (20)

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
    0x0800	//	PAD_L1
};

uint32_t ps2tojoypad(int ps2movebtn);

void joypadMode(void);
void mouseMode(void);

static void tickHandler( void ) {
    // ticks += timer.read_us( ) >> 6;
    // timer.reset( );
    if (longPushCnt > oldLongPushCnt) {
        ticks++;
    }
    else {
        ticks = 0;
    }
    oldLongPushCnt = longPushCnt;
}

int main() {
    // uint8_t ps2move = 0;    
    // int16_t x = 0;
    // int16_t y = 0;
    // uint32_t buttons = 0;    
    // int ps2movebtn = 0;

    confSysClock();         //Configure system clock (72MHz HSE clock, 48MHz USB clock)

    pc.baud(115200);
    printf("hello, Mbed.\n");
    // ps2.init();

    // led = 1; //led off
    led = 0; //led on

     __disable_irq( );
    // configure timer
    // timer.start( );
    ticker.attach_us( tickHandler, 100000 ); // reset timer every 100msec
     __enable_irq( );

    // joypadMode();
    mouseMode();

    // while(1)
    // {   
    //     ps2.poll();     

    //     // check button
    //     ps2movebtn = ps2.read(PS_PAD :: BUTTONS);
    //     buttons = ps2tojoypad(ps2movebtn);
        
    //     // check move
    //     ps2move = ps2.read_move();
    //     ps2move &= 0x0F;
    //     x = moveTable[ps2move][0];  // value -127 .. 128
    //     y = moveTable[ps2move][1];  // value -127 .. 128
    //     joystick.move_buttons(x, y, buttons);
    //     wait(0.05);
    // }

}

void mouseMode() {
    USBMouseKeyboard key_mouse;
    PS_PAD      ps2(PA_7, PA_6, PA_5, PB_6);  //mosi=CMD,miso=DAT,clk=CLK,ss=SEL
    // USBJoystick joystick;

    uint8_t ps2move = 0;    
    int16_t x = 0;
    int16_t y = 0;
    uint32_t buttons = 0;    
    int ps2movebtn = 0;

    ps2.init();

    while(1)
    {
        // check ticks
        if(ticks >= COUNT_3_SEC) { // 3sec
            ticks = 0;
            led = !led; // toggle led
        }

        ps2.poll();     

        // check button
        ps2movebtn = ps2.read(PS_PAD :: BUTTONS);
        buttons = ps2tojoypad(ps2movebtn);

        if( buttons & 0x0002 ) { // PAD_X
            // 左側がタッチされている状態
            key_mouse.press(MOUSE_LEFT);  // マウス左クリックをクリックした状態にする
        }
        else {
            // タッチされていない状態
            key_mouse.release(MOUSE_LEFT);  // マウス左クリックを解放した状態にする
        }

        if( buttons & 0x0001 ) { // PAD_CIRCLE
            // 右側がタッチされている状態
            key_mouse.press(MOUSE_RIGHT); // マウス右クリックをクリックした状態にする
        }
        else {
            // タッチされていない状態
            key_mouse.release(MOUSE_RIGHT); // マウス右クリックを解放した状態にする
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
        x = moveMouseTable[ps2move][0];  // value -127 .. 128
        y = moveMouseTable[ps2move][1];  // value -127 .. 128
        // joystick.move_buttons(x, y, buttons);
        key_mouse.move(x, y);
        wait(0.05);
    }
}

void joypadMode() {
    PS_PAD      ps2(PA_7, PA_6, PA_5, PB_6);  //mosi=CMD,miso=DAT,clk=CLK,ss=SEL
    USBJoystick joystick;

    uint8_t ps2move = 0;    
    int16_t x = 0;
    int16_t y = 0;
    uint32_t buttons = 0;    
    int ps2movebtn = 0;

    ps2.init();

    while(1)
    {
        // check ticks
        if(ticks >= COUNT_3_SEC) { // 3sec
            ticks = 0;
            led = !led; // toggle led
        }

        ps2.poll();     

        // check button
        ps2movebtn = ps2.read(PS_PAD :: BUTTONS);
        buttons = ps2tojoypad(ps2movebtn);
        
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
        joystick.move_buttons(x, y, buttons);
        wait(0.05);
    }

}

uint32_t ps2tojoypad(int ps2movebtn) {
    int i;
    uint32_t targetButton = 0;
    uint32_t joypadbtn = 0;
    uint32_t ps2btn = ps2movebtn & ~(0x0080 | 0x0040 | 0x0020 | 0x0010); // remove move key :(PAD_LEFT | PAD_BOTTOM | PAD_RIGHT | PAD_TOP)

    for ( i=0; i<BUTTON_NUM; i++ ) {
        targetButton = ps2btn & buttonFilter[i];
        switch (targetButton) {
            // PAD_START:0x0008 -> usbPadButton: 0x0080
            case 0x0008:
                joypadbtn |= 0x0080;
                break;
            // PAD_SELECT:0x0001 -> usbPadButton: 0x0040
            case 0x0001:
                joypadbtn |= 0x0040;
                break;
            // PAD_SQUARE:0x8000 -> usbPadButton: 0x0008
            case 0x8000:
                joypadbtn |= 0x0008;
                break;
            // PAD_X:0x4000 -> usbPadButton: 0x0002
            case 0x4000:
                joypadbtn |= 0x0002;
                break;
            // PAD_CIRCLE:0x2000 -> usbPadButton: 0x0001
            case 0x2000:
                joypadbtn |= 0x0001;
                break;
            // PAD_TRIANGLE:0x1000 -> usbPadButton: 0x0004
            case 0x1000:
                joypadbtn |= 0x0004;
                break;
            // PAD_R1:0x0400 -> usbPadButton: 0x0020
            case 0x0400:
                joypadbtn |= 0x0020;
                break;
            // PAD_L1:0x0800 -> usbPadButton: 0x0010
            case 0x0800:
                joypadbtn |= 0x0010;
                break;
            default:
                ;
                break;
        }
    }

    return joypadbtn;
}