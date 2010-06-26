/*
 * Hardware definitions for Dell Axim X50/51(v)
 *
 * Copyright (c) 2009 Ertan Deniz
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/gpio_keys.h>
#include <linux/i2c.h>

#include <linux/mfd/htc-egpio.h>

#include <linux/spi/spi.h>
#include <linux/spi/ads7846.h>
#include <mach/pxa2xx_spi.h>

#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <plat/i2c.h>

#include <mach/aximx50.h>
#include <mach/pxa2xx-regs.h>
#include <mach/mfp-pxa27x.h>
#include <mach/pxa27x_keypad.h>
#include <mach/pxafb.h>
#include <mach/gpio.h>
#include <mach/ssp.h>
#include <mach/mmc.h>
#include <mach/udc.h>

#include "generic.h"
#include "devices.h"

/****************
 * Init Machine *
 ****************/

static unsigned long aximx50_pin_config[] __initdata = {

    /* BTUART */
    GPIO42_BTUART_RXD,
    GPIO43_BTUART_TXD,
    GPIO44_BTUART_CTS,
    GPIO45_BTUART_RTS,

    /* STUART */
    GPIO46_STUART_RXD,
    GPIO47_STUART_TXD,

    /* MCI controller */
    GPIO32_MMC_CLK,
    GPIO112_MMC_CMD,
    GPIO92_MMC_DAT_0,
    GPIO109_MMC_DAT_1,
    GPIO110_MMC_DAT_2,
    GPIO111_MMC_DAT_3,

    /* LCD */
    GPIO58_LCD_LDD_0,
    GPIO59_LCD_LDD_1,
    GPIO60_LCD_LDD_2,
    GPIO61_LCD_LDD_3,
    GPIO62_LCD_LDD_4,
    GPIO63_LCD_LDD_5,
    GPIO64_LCD_LDD_6,
    GPIO65_LCD_LDD_7,
    GPIO66_LCD_LDD_8,
    GPIO67_LCD_LDD_9,
    GPIO68_LCD_LDD_10,
    GPIO69_LCD_LDD_11,
    GPIO70_LCD_LDD_12,
    GPIO71_LCD_LDD_13,
    GPIO72_LCD_LDD_14,
    GPIO73_LCD_LDD_15,
    GPIO74_LCD_FCLK,
    GPIO75_LCD_LCLK,
    GPIO76_LCD_PCLK,
    GPIO77_LCD_BIAS,

    /* I2C */
    GPIO117_I2C_SCL,
    GPIO118_I2C_SDA,

    /* SSP1 */
    GPIO23_SSP1_SCLK,
    GPIO24_GPIO, /* GPIO_NR_X50_TSC2046_CS - SFRM as chip select */
    GPIO25_SSP1_TXD,
    GPIO26_SSP1_RXD,

    /* PC Card */
    GPIO48_nPOE,
    GPIO49_nPWE,
    GPIO50_nPIOR,
    GPIO51_nPIOW,
    GPIO85_nPCE_1,
    GPIO54_nPCE_2,
    GPIO55_nPREG,
    GPIO56_nPWAIT,
    GPIO57_nIOIS16,

    /* SDRAM and local bus */
    GPIO15_nCS_1,
    GPIO78_nCS_2,
    GPIO79_nCS_3,
    GPIO80_nCS_4,
    GPIO33_nCS_5,
    GPIO18_RDY,
    
    /* Keys */
    GPIO93_KP_DKIN_0,
    GPIO100_KP_MKIN_0,
    GPIO101_KP_MKIN_1,
    GPIO97_KP_MKIN_3,
    GPIO98_KP_MKIN_4,
    GPIO90_KP_MKIN_5,
    GPIO91_KP_MKIN_6,
    GPIO103_KP_MKOUT_0,
    GPIO105_KP_MKOUT_2,
};

/******************************************************************************
 * SD/MMC card controller
 ******************************************************************************/
static struct pxamci_platform_data aximx50_mci_platform_data = {
	.ocr_mask		= MMC_VDD_32_33 | MMC_VDD_33_34,
	.gpio_card_detect	= GPIO_NR_AXIMX50_SD_DETECT,
	.gpio_card_ro		= GPIO_NR_AXIMX50_SD_READONLY,
	.gpio_power		= GPIO_NR_AXIMX50_SD_POWER,
};

/****************************************************************
 * Keyboard
 ****************************************************************/
static unsigned int x50_key_map[] = {
    KEY(0, 0, KEY_WLAN),
    KEY(0, 2, KEY_UP),

    KEY(1, 0, KEY_RECORD),
    KEY(1, 2, KEY_DOWN),

    KEY(3, 0, 67), // calendar
    KEY(3, 2, KEY_RIGHT),

    KEY(4, 0, 68), // contacts
    KEY(4, 2, KEY_LEFT),

    KEY(5, 0, KEY_MENU), // mail
    KEY(5, 2, KEY_ENTER),

    KEY(6, 0, KEY_BACK), // home
};

static struct pxa27x_keypad_platform_data x50_kbd = {
    .matrix_key_rows = 7,
    .matrix_key_cols = 3,
    .matrix_key_map = x50_key_map,
    .matrix_key_map_size = ARRAY_SIZE(x50_key_map),

    .direct_key_num = 1,
    //.direct_key_map = { KEY_POWER, 0, 0, 0, 0, 0, 0, 0 },
    .direct_key_map = { 0, 0, 0, 0, 0, 0, 0, 0 },
};

/****************************************************************
 * USB Gadget
 ****************************************************************/

static struct pxa2xx_udc_mach_info x50_udc_info = {
    .gpio_vbus_inverted = false,
    .gpio_vbus = GPIO_NR_X50_USB_CABLE_DETECT,
//  .gpio_pullup =
};

/***************
 * Framebuffer *
 ***************/

static struct pxafb_mode_info aximx50_pxafb_modes_vga[] = {
{
	.pixclock	= 96153,
	.bpp		= 16,
	.xres		= 480,
	.yres		= 640,
	.hsync_len	= 64,
	.vsync_len	= 5,
	.left_margin	= 17,
	.upper_margin	= 1,
	.right_margin	= 87,
	.lower_margin   = 4,
},
};

static struct pxafb_mach_info aximx50_fb_info_vga = {
	.modes		= aximx50_pxafb_modes_vga,
	.num_modes	= ARRAY_SIZE(aximx50_pxafb_modes_vga),
	.lccr0		= LCCR0_ENB | LCCR0_LDM |		// 0x9
		LCCR0_SFM | LCCR0_IUM | LCCR0_EFM | LCCR0_Act |	// 0xf
		LCCR0_QDM |					// 0x8
								// 0x0
								// 0x0
		LCCR0_BM  | LCCR0_OUM | LCCR0_RDSTM |		// 0xb
		LCCR0_CMDIM					// 0x1
		,						// 0x0
		//0x01b008f9,
	.lccr3		= 0x04f00001,
};


/* ADS7846 is connected through SSP ... */
static struct pxa2xx_spi_master pxa_ssp_master_info = {
    .num_chipselect = 3, /* Matches the number of chips attached to NSSP */
//  .clock_enable = CKEN_SSP1,
    .enable_dma = 1,
};

static void ads7846_cs(u32 command)
{
    gpio_set_value(GPIO_NR_X50_TSC2046_CS, !(command == PXA2XX_CS_ASSERT));
}

static int aximx50_ads7846_pendown_state(void)
{
    return !gpio_get_value(GPIO_NR_X50_PEN_IRQ_N);
}

static struct pxa2xx_spi_chip ads_hw = {
//  .tx_threshold       = 12,
//  .rx_threshold       = 12,
//  .dma_burst_size     = 8,
    .cs_control     = ads7846_cs,
};

static const struct ads7846_platform_data aximx50_ts_info = {
    .model              = 7846,
    .vref_delay_usecs   = 100,  /* internal, no capacitor */
    .settle_delay_usecs = 150,
    .x_min              = 1,
    .x_max              = 480,
    .y_min              = 1,
    .y_max              = 640,
    .pressure_min       = 1,
    .pressure_max       = 512,
    .debounce_max       = 20,
    .debounce_tol       = 10,
    .debounce_rep       = 1,
//  .get_pendown_state  = &aximx50_ads7846_pendown_state,
    .gpio_pendown       = GPIO_NR_X50_PEN_IRQ_N,
};

static struct spi_board_info __initdata aximx50_boardinfo[] = { {
    .modalias         = "ads7846",
    .platform_data    = &aximx50_ts_info,
    .controller_data  = &ads_hw,
    .irq              = X50_IRQ(PEN_IRQ_N),
    .max_speed_hz     = 100000 /* max sample rate at 3V */
                        * 26 /* command + data + overhead */,
    .bus_num          = 1,
    .chip_select      = 0,
} };

/******************************************************/
/* EGPIOs */


/*
 * EGPIO (Xilinx CPLD)
 *
 * 7 32-bit aligned 8-bit registers: 3x output, 1x irq, 3x input
 */
static struct resource egpio_resources[] = {
	[0] = {
		.start = PXA_CS4_PHYS,
		.end   = PXA_CS4_PHYS + 0x20,
		.flags = IORESOURCE_MEM,
	},/*
	[1] = {
		.start = gpio_to_irq(GPIO13_MAGICIAN_CPLD_IRQ),
		.end   = gpio_to_irq(GPIO13_MAGICIAN_CPLD_IRQ),
		.flags = IORESOURCE_IRQ,
	},*/
};

static struct htc_egpio_chip egpio_chips[] = {
	[0] = {
		.reg_start = 0,
		.gpio_base = X50_EGPIO_BASE,
		.num_gpios = 24,
		.direction = HTC_EGPIO_OUTPUT,
		//.initial_values = 0x40, /* EGPIO_MAGICIAN_GSM_RESET */
	},/*
	[1] = {
		.reg_start = 4,
		.gpio_base = MAGICIAN_EGPIO(4, 0),
		.num_gpios = 24,
		.direction = HTC_EGPIO_INPUT,
	},*/
};

static struct htc_egpio_platform_data egpio_info = {
	.reg_width    = 8,
	.bus_width    = 32,
	/*.irq_base     = IRQ_BOARD_START,
	.num_irqs     = 4,
	.ack_register = 3,*/
	.chip         = egpio_chips,
	.num_chips    = ARRAY_SIZE(egpio_chips),
};

static struct platform_device aximx50_egpio = {
	.name          = "htc-egpio",
	.id            = -1,
	.resource      = egpio_resources,
	.num_resources = ARRAY_SIZE(egpio_resources),
	.dev = {
		.platform_data = &egpio_info,
	},
};

/******************************************************/

static struct platform_device *devices[] __initdata = {
    &aximx50_egpio,
    //&aximx50_bt,
};

static void __init aximx50_map_io(void)
{
    pxa_map_io();
}

static void __init aximx50_init_irq(void)
{
    pxa27x_init_irq();
}

static void __init aximx50_init( void )
{
    int err;
    printk(KERN_NOTICE "Dell Axim x50/x51v initialization\n");

    pxa2xx_mfp_config(ARRAY_AND_SIZE(aximx50_pin_config));
    set_pxa_fb_info(&aximx50_fb_info_vga);
    
    err = gpio_request(GPIO_NR_X50_TSC2046_CS, "ADS7846_CS");
    if (err)
        return;

    gpio_direction_output(GPIO_NR_X50_TSC2046_CS, 1);
    
    pxa_set_i2c_info(NULL);
    
    pxa2xx_set_spi_info(1, &pxa_ssp_master_info);
    spi_register_board_info(aximx50_boardinfo, ARRAY_SIZE(aximx50_boardinfo));
    
    pxa_set_mci_info(&aximx50_mci_platform_data);
    pxa_set_udc_info(&x50_udc_info);
    pxa_set_keypad_info(&x50_kbd);
    
    platform_add_devices(devices, ARRAY_SIZE(devices));
}


MACHINE_START(X50, "Dell Axim x50/x51(v)")
    .phys_io = 0x40000000,
    .io_pg_offst = (io_p2v(0x40000000) >> 18) & 0xfffc,
    .boot_params = 0xa8000100,
    .map_io = aximx50_map_io,
    .init_irq = aximx50_init_irq,
    .timer = &pxa_timer,
    .init_machine = aximx50_init,
MACHINE_END

