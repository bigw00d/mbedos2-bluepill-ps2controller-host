# mbedos2-cli-blulpill-blink

## Quick start with BluePill

1. set up
* install [STM32 ST-LINK Utility(including usb device driver)](https://www.st.com/content/st_com/ja/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-programmers/stsw-link004.html?dl=Esr3641WBLOeWDySnK7N%2FA%3D%3D%2Ct2UU9IOHq6oAQxLOeevmNznGb5UNia9EEIGo7rnBkXvgp2qXK%2FEhonXMx8Lv1ONG%2BBGz6oLVvvRLC0fX4sqXLqxqRc%2FaGWkkp%2BzgVl3V8VaVax5Ek3%2B%2FnKEA2DoYmEBgGjVibstcCnqSQu3K%2FdrpwoyD%2Fjq5kyQo9fUPLD0r%2BkkH0XRI92wrVFJ9dHYPwGB7u6pTDyvFHgJWkSrDEPGqkkzKByl9zUs%2FO6htinjzwmSaoxvVa4ApHl6LOu%2BqjipivC6FasmqiGbXpOdj6xW6U7O5Jh8AJotVeWWvtEaXTJh%2FrOTkMRvp8yAwSk9P2ub0t%2Fju7tadlwUCLflzrdsBLw%3D%3D&uid=BcpcApvrxWmbd6DozNPYo6GSOuPdfRWxFiMMWdA72HaQtyscK/gjzQ==)
* update ST-Link V2 firmware with ST-LINK Utility
    * ST-LINK -> Firmware update -> Device Connect

* install[mbed-cli-windows-installer](https://github.com/ARMmbed/mbed-cli-windows-installer/releases)

1. build & upload project
```txt
C:\WorkSpace\mbed-repo\mbedos2-cli-example-basic>mbed compile -m BLUEPILL_F103C8 -t GCC_ARM
C:\WorkSpace\mbed-repo\mbedos2-cli-example-basic>ST-LINK_CLI.exe -c SWD -P .\BUILD\BLUEPILL_F103C8\GCC_ARM\mbedos2-cli-example-basic.bin 0x8000000 -Rst -Run -NoPrompt 
```

## Debug

* see [BluePillのmbed VSCode開発環境を構築してみた](https://qiita.com/kohei0302/items/6aa927907fe868ef0b50)

## Arduino

1. build & upload project(Arduino 1.8.5)
* install [Arduino STM32 device](https://github.com/rogerclarkmelbourne/Arduino_STM32/wiki)
* or board manager (I use this)
    * tool > board > board manager > Arduino SAM Boards (32-bits ARM Cortex-M3)
    * board setting
        * Board: Generic STM32F103C series
        * Optimize: Smallest
        * Variant: STM32F103C8 (20k RAM, 64k Flash) or STM32F103CB (20k RAM, 128k Flash)
        * CPU Speed(MHz): 72MHz (Normal)
        * Upload method: STLink
        * Serial port: (non selection)
        * Programmer: USBasp

## Related Links
* [Install OpenOCD](https://os.mbed.com/teams/Renesas/wiki/Exporting-to-e2studio-with-CMSIS_DAP-DBG#install-openocd)
* [Blue Pill + mbed](https://qiita.com/nanbuwks/items/26241a90004665570fe6)
* [STM32F103C8T6 でLチカ](https://qiita.com/sugasaki/items/98bbff89bbd7f235525f)