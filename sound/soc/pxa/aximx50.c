/*
 * ALSA SoC for Dell Axim x50/x51v
 *
 * Copyright (c) 2009 Ertan Deniz
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>
#include <mach/aximx50.h>
//#include <mach/pxa-regs.h>
#include <mach/hardware.h>
#include <mach/audio.h>

#include "../codecs/wm8750.h"
#include "pxa2xx-pcm.h"
#include "pxa2xx-i2s.h"

#define aximx50_HP        1
#define aximx50_HP_OFF    0
#define aximx50_SPK_ON    1
#define aximx50_SPK_OFF   0

 /* audio clock in Hz - rounded from 12.235MHz */
#define aximx50_AUDIO_CLOCK 12288000

static int aximx50_jack_func;
static int aximx50_spk_func;

static void aximx50_ext_control(struct snd_soc_codec *codec)
{
    /* set up jack connection */
    if (aximx50_jack_func == aximx50_HP) {
        /* set = unmute headphone */
		gpio_set_value(GPIO_NR_X50_MUTE_L, 1);
		gpio_set_value(GPIO_NR_X50_MUTE_R, 1);
        snd_soc_dapm_enable_pin(codec, "Headphone Jack");
    } else {
		gpio_set_value(GPIO_NR_X50_MUTE_L, 0);
		gpio_set_value(GPIO_NR_X50_MUTE_R, 0);
        snd_soc_dapm_disable_pin(codec, "Headphone Jack");
    }

    /* set the enpoints to their new connetion states */
    if (aximx50_spk_func == aximx50_SPK_ON)
        snd_soc_dapm_enable_pin(codec, "Ext Spk");
    else
        snd_soc_dapm_disable_pin(codec, "Ext Spk");

    /* signal a DAPM event */
    snd_soc_dapm_sync(codec);
}

static int aximx50_startup(struct snd_pcm_substream *substream)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_codec *codec = rtd->socdev->card->codec;

    /* check the jack status at stream startup */
    aximx50_ext_control(codec);
    return 0;
}

/* we need to unmute the HP at shutdown as the mute burns power on poodle */
static void aximx50_shutdown(struct snd_pcm_substream *substream)
{
    /* set = unmute headphone */
	//gpio_set_value(GPIO_NR_X50_MUTE_L, 1);
	//gpio_set_value(GPIO_NR_X50_MUTE_R, 1);
}

static int aximx50_hw_params(struct snd_pcm_substream *substream,
    struct snd_pcm_hw_params *params)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
    struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
    unsigned int clk = 0;
    int ret = 0;

    switch (params_rate(params)) {
    case 8000:
    case 16000:
    case 48000:
    case 96000:
        clk = 12288000;
        break;
    case 11025:
    case 22050:
    case 44100:
        clk = 11289600;
        break;
    }

    /* set codec DAI configuration */
    ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S |
        SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
    if (ret < 0)
        return ret;

    /* set cpu DAI configuration */
    ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S |
        SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
    if (ret < 0)
        return ret;

    /* set the codec system clock for DAC and ADC */
    ret = snd_soc_dai_set_sysclk(codec_dai, WM8750_SYSCLK, clk,
        SND_SOC_CLOCK_IN);
    if (ret < 0)
        return ret;

    /* set the I2S system clock as input (unused) */
    ret = snd_soc_dai_set_sysclk(cpu_dai, PXA2XX_I2S_SYSCLK, clk,
        SND_SOC_CLOCK_OUT);
    if (ret < 0)
        return ret;

    return 0;
}

static struct snd_soc_ops aximx50_ops = {
    .startup = aximx50_startup,
    .hw_params = aximx50_hw_params,
    .shutdown = aximx50_shutdown,
};

static int aximx50_get_jack(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    ucontrol->value.integer.value[0] = aximx50_jack_func;
    return 0;
}

static int aximx50_set_jack(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    struct snd_soc_codec *codec =  snd_kcontrol_chip(kcontrol);

    if (aximx50_jack_func == ucontrol->value.integer.value[0])
        return 0;

    aximx50_jack_func = ucontrol->value.integer.value[0];
    aximx50_ext_control(codec);
    return 1;
}

static int aximx50_get_spk(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    ucontrol->value.integer.value[0] = aximx50_spk_func;
    return 0;
}

static int aximx50_set_spk(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    struct snd_soc_codec *codec =  snd_kcontrol_chip(kcontrol);

    if (aximx50_spk_func == ucontrol->value.integer.value[0])
        return 0;

    aximx50_spk_func = ucontrol->value.integer.value[0];
    aximx50_ext_control(codec);
    return 1;
}

static int aximx50_amp_event(struct snd_soc_dapm_widget *w,
    struct snd_kcontrol *k, int event)
{/*
    if (SND_SOC_DAPM_EVENT_ON(event))
        locomo_gpio_write(&aximx50_locomo_device.dev,
            aximx50_LOCOMO_GPIO_AMP_ON, 0);
    else
        locomo_gpio_write(&aximx50_locomo_device.dev,
            aximx50_LOCOMO_GPIO_AMP_ON, 1);
*/
    return 0;
}

/* poodle machine dapm widgets */
static const struct snd_soc_dapm_widget wm8750_dapm_widgets[] = {
SND_SOC_DAPM_HP("Headphone Jack", NULL),
SND_SOC_DAPM_SPK("Ext Spk", aximx50_amp_event),
};

/* Corgi machine connections to the codec pins */
static const struct snd_soc_dapm_route audio_map[] = {

    /* headphone connected to LOUT1, ROUT1 */
    {"Headphone Jack", NULL, "LOUT1"},
    {"Headphone Jack", NULL, "ROUT1"},

    /* ext speaker connected to LOUT2, ROUT2  */
    {"Ext Spk", NULL , "ROUT2"},
    {"Ext Spk", NULL , "LOUT2"},
};

static const char *jack_function[] = {"Off", "Headphone"};
static const char *spk_function[] = {"Off", "On"};
static const struct soc_enum aximx50_enum[] = {
    SOC_ENUM_SINGLE_EXT(2, jack_function),
    SOC_ENUM_SINGLE_EXT(2, spk_function),
};

static const struct snd_kcontrol_new wm8750_aximx50_controls[] = {
    SOC_ENUM_EXT("Jack Function", aximx50_enum[0], aximx50_get_jack,
        aximx50_set_jack),
    SOC_ENUM_EXT("Speaker Function", aximx50_enum[1], aximx50_get_spk,
        aximx50_set_spk),
};

/*
 * Logic for a wm8750 as connected on a Sharp SL-C7x0 Device
 */
static int aximx50_wm8750_init(struct snd_soc_codec *codec)
{
    int i, err;

    snd_soc_dapm_disable_pin(codec, "RINPUT1");
    snd_soc_dapm_disable_pin(codec, "LINPUT2");
    snd_soc_dapm_disable_pin(codec, "RINPUT2");
    snd_soc_dapm_disable_pin(codec, "LINPUT3");
    snd_soc_dapm_disable_pin(codec, "RINPUT3");
    snd_soc_dapm_disable_pin(codec, "OUT3");
    snd_soc_dapm_disable_pin(codec, "MONO1");

    /* Add axim specific controls */
    for (i = 0; i < ARRAY_SIZE(wm8750_aximx50_controls); i++) {
        err = snd_ctl_add(codec->card,
            snd_soc_cnew(&wm8750_aximx50_controls[i], codec, NULL));
        if (err < 0)
            return err;
    }

    /* Add axim specific widgets */
    snd_soc_dapm_new_controls(codec, wm8750_dapm_widgets,
                  ARRAY_SIZE(wm8750_dapm_widgets));

    /* Set up axim specific audio path audio_map */
    snd_soc_dapm_add_routes(codec, audio_map, ARRAY_SIZE(audio_map));

    snd_soc_dapm_sync(codec);
    return 0;
}

/* aximx50 digital audio interface glue - connects codec <--> CPU */
static struct snd_soc_dai_link aximx50_dai = {
    .name = "wm8750",
    .stream_name = "WM8750",
    .cpu_dai = &pxa_i2s_dai,
    .codec_dai = &wm8750_dai,
    .init = aximx50_wm8750_init,
    .ops = &aximx50_ops,
};

/* aximx50 audio machine driver */
static struct snd_soc_card snd_soc_aximx50 = {
    .name = "Aximx50",
	.platform = &pxa2xx_soc_platform,
    .dai_link = &aximx50_dai,
    .num_links = 1,
};

/* aximx50 audio private data */
static struct wm8750_setup_data aximx50_wm8750_setup = {
	.i2c_bus = 0,
    .i2c_address = 0x1a,
};

/* aximx50 audio subsystem */
static struct snd_soc_device aximx50_snd_devdata = {
    .card = &snd_soc_aximx50,
    .codec_dev = &soc_codec_dev_wm8750,
    .codec_data = &aximx50_wm8750_setup,
};

static struct platform_device *aximx50_snd_device;

static int __init aximx50_init(void)
{
    int err;
    
    aximx50_snd_device = platform_device_alloc("soc-audio", -1);
    if (!aximx50_snd_device)
        return -ENOMEM;

	err = gpio_request(GPIO_NR_AXIMX50_AUDIO_PWR, "AXIMX50_AUDIO_PWR");
	if (err) {
		printk(KERN_ERR "aximx50: Unable to claim audio power GPIO\n");
		return err;
	}
	err = gpio_direction_output(GPIO_NR_AXIMX50_AUDIO_PWR, 1);
	if (err) {
		printk(KERN_ERR "aximx50: Unable to set audio power GPIO to output\n");
		return err;
	}

    platform_set_drvdata(aximx50_snd_device, &aximx50_snd_devdata);
    aximx50_snd_devdata.dev = &aximx50_snd_device->dev;
    err = platform_device_add(aximx50_snd_device);

    if (err)
        platform_device_put(aximx50_snd_device);

    return err;
}

static void __exit aximx50_exit(void)
{
    platform_device_unregister(aximx50_snd_device);
}

module_init(aximx50_init);
module_exit(aximx50_exit);

/* Module information */
MODULE_AUTHOR("Ertan Deniz");
MODULE_DESCRIPTION("ALSA SoC for Dell Axim x50/51v");
MODULE_LICENSE("GPL");

