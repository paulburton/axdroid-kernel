/*
 * GPIOs and interrupts for Axim X50/51(v) PDAs
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef _INCLUDE_AXIMX50_H_
#define _INCLUDE_AXIMX50_H_

#define X50_IRQ(gpio) \
    IRQ_GPIO(GPIO_NR_X50_ ## gpio)

/*********************************************************************/

#define GPIO_NR_X50_AC_IN_N           11              /* Input = 1 when externally powered */
#define GPIO_NR_X50_BACKLIGHT_ON      17              /* Tied to PWM0 when Alt function == 2 */
#define GPIO_NR_X50_PEN_IRQ_N         94              /* Input = 0 when stylus down */
#define GPIO_NR_X50_USB_CABLE_DETECT  14              /* Input = 0 when USB is connected */
#define GPIO_NR_X50_USB_PULLUP        2               /* Input = 0 when USB is connected */
#define GPIO_NR_X50_TSC2046_CS        24              /* TSC2047 Touchscreen chip select */
#define GPIO_NR_X50_BT_PWR_EN         22              /* 1 when bluetooth is enabled */

/* experimental values ?? */
#define GPIO_NR_X50_MUTE_L            83
#define GPIO_NR_X50_MUTE_R            96

/* SD Card */
#define GPIO_NR_AXIMX50_SD_POWER            120       //TODO: This is wrong
#define GPIO_NR_AXIMX50_SD_DETECT           12
#define GPIO_NR_AXIMX50_SD_READONLY         27

/* PCMCIA (WiFi) */
#define GPIO_NR_AXIMX50_PCMCIA_READY        38        //TODO: Unsure about these
#define GPIO_NR_AXIMX50_PCMCIA_POWER        107
#define GPIO_NR_AXIMX50_PCMCIA_RESET        89

/* Buttons */
#define GPIO_NR_AXIMX50_BTN_POWER           0

/* Audio */
#define GPIO_NR_AXIMX50_AUDIO_PWR           82

#endif
