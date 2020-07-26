/*
 * Copyright 2012 Michael Ossmann <mike@ossmann.com>
 * Copyright 2012 Benjamin Vernoux <titanmkd@gmail.com>
 * Copyright 2012 Jared Boone <jared@sharebrained.com>
 *
 * This file is part of HackRF.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __HACKRF_CORE_H
#define __HACKRF_CORE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

#include "si5351c.h"
#include "spi_ssp.h"

#include "max2837.h"
#include "max5864.h"
#include "mixer.h"
#include "w25q80bv.h"
#include "sgpio.h"
#include "rf_path.h"
#include "cpld_jtag.h"

/* hardware identification number */
#define BOARD_ID_JAWBREAKER 1
#define BOARD_ID_HACKRF_ONE 2
#define BOARD_ID_RAD1O      3

#ifdef JAWBREAKER
#define BOARD_ID BOARD_ID_JAWBREAKER
#endif

#ifdef HACKRF_ONE
#define BOARD_ID BOARD_ID_HACKRF_ONE
#endif

#ifdef RAD1O
#define BOARD_ID BOARD_ID_RAD1O
#endif

/*
 * SCU PinMux
 */

/* GPIO Output PinMux */
#define SCU_PINMUX_LED1     (P4_1)  /* GPIO2[1] on P4_1 */
#define SCU_PINMUX_LED2     (P4_2)  /* GPIO2[2] on P4_2 */
#define SCU_PINMUX_LED3     (P6_12) /* GPIO2[8] on P6_12 */
#ifdef RAD1O
#define SCU_PINMUX_LED4     (PB_6) /* GPIO5[26] on PB_6 */
#endif

#define SCU_PINMUX_EN1V8    (P6_10) /* GPIO3[6] on P6_10 */

/* GPIO Input PinMux */
#define SCU_PINMUX_BOOT0    (P1_1)  /* GPIO0[8] on P1_1 */
#define SCU_PINMUX_BOOT1    (P1_2)  /* GPIO0[9] on P1_2 */
#define SCU_PINMUX_BOOT2    (P2_8)  /* GPIO5[7] on P2_8 */
#define SCU_PINMUX_BOOT3    (P2_9)  /* GPIO1[10] on P2_9 */

/* USB peripheral */
#ifdef JAWBREAKER
#define SCU_PINMUX_USB_LED0 (P6_8)
#define SCU_PINMUX_USB_LED1 (P6_7)
#endif

/* SSP1 Peripheral PinMux */
#define SCU_SSP1_MISO       (P1_3)  /* P1_3 */
#define SCU_SSP1_MOSI       (P1_4)  /* P1_4 */
#define SCU_SSP1_SCK        (P1_19) /* P1_19 */
#define SCU_SSP1_SSEL       (P1_20) /* P1_20 */

/* CPLD JTAG interface */
#define SCU_PINMUX_CPLD_TDO (P9_5)  /* GPIO5[18] */
#define SCU_PINMUX_CPLD_TCK (P6_1)  /* GPIO3[ 0] */
#if (defined HACKRF_ONE || defined RAD1O)
#define SCU_PINMUX_CPLD_TMS (P6_5)  /* GPIO3[ 4] */
#define SCU_PINMUX_CPLD_TDI (P6_2)  /* GPIO3[ 1] */
#else
#define SCU_PINMUX_CPLD_TMS (P6_2)  /* GPIO3[ 1] */
#define SCU_PINMUX_CPLD_TDI (P6_5)  /* GPIO3[ 4] */
#endif

/* CPLD SGPIO interface */
#define SCU_PINMUX_SGPIO0   (P0_0)
#define SCU_PINMUX_SGPIO1   (P0_1)
#define SCU_PINMUX_SGPIO2   (P1_15)
#define SCU_PINMUX_SGPIO3   (P1_16)
#define SCU_PINMUX_SGPIO4   (P6_3)
#define SCU_PINMUX_SGPIO5   (P6_6)
#define SCU_PINMUX_SGPIO6   (P2_2)
#define SCU_PINMUX_SGPIO7   (P1_0)
#if (defined JAWBREAKER || defined HACKRF_ONE || defined RAD1O)
#define SCU_PINMUX_SGPIO8   (P9_6)
#endif
#define SCU_PINMUX_SGPIO9   (P4_3)
#define SCU_PINMUX_SGPIO10  (P1_14)
#define SCU_PINMUX_SGPIO11  (P1_17)
#define SCU_PINMUX_SGPIO12  (P1_18)
#define SCU_PINMUX_SGPIO13  (P4_8)
#define SCU_PINMUX_SGPIO14  (P4_9)
#define SCU_PINMUX_SGPIO15  (P4_10)

/* MAX2837 GPIO (XCVR_CTL) PinMux */
#ifdef RAD1O
#define SCU_XCVR_RXHP       (P8_1)  /* GPIO[] on P8_1 */
#define SCU_XCVR_B6         (P8_2)  /* GPIO[] on P8_2 */
#define SCU_XCVR_B7         (P9_3)  /* GPIO[] on P8_3 */
#endif

#define SCU_XCVR_ENABLE     (P4_6)  /* GPIO2[6] on P4_6 */
#define SCU_XCVR_RXENABLE   (P4_5)  /* GPIO2[5] on P4_5 */
#define SCU_XCVR_TXENABLE   (P4_4)  /* GPIO2[4] on P4_4 */
#define SCU_XCVR_CS         (P1_20) /* GPIO0[15] on P1_20 */

/* MAX5864 SPI chip select (AD_CS) GPIO PinMux */
#define SCU_AD_CS           (P5_7)  /* GPIO2[7] on P5_7 */

/* RFFC5071 GPIO serial interface PinMux */
#if (defined JAWBREAKER || defined HACKRF_ONE)
#define SCU_MIXER_ENX       (P5_4)  /* GPIO2[13] on P5_4 */
#define SCU_MIXER_SCLK      (P2_6)  /* GPIO5[6] on P2_6 */
#define SCU_MIXER_SDATA     (P6_4)  /* GPIO3[3] on P6_4 */
#define SCU_MIXER_RESETX    (P5_5)  /* GPIO2[14] on P5_5 */
#endif
#ifdef RAD1O
#define SCU_VCO_CE          (P5_4)  /* GPIO2[13] on P5_4 */
#define SCU_VCO_SCLK        (P2_6)  /* GPIO5[6] on P2_6 */
#define SCU_VCO_SDATA       (P6_4)  /* GPIO3[3] on P6_4 */
#define SCU_VCO_LE          (P5_5)  /* GPIO2[14] on P5_5 */
#define SCU_VCO_MUX         (PB_5)  /* GPIO5[25] on PB_5 */
#define SCU_MIXER_EN        (P6_8)  /* GPIO5[16] on P6_8 */
#define SCU_SYNT_RFOUT_EN   (P6_9)  /* GPIO3[5] on P6_9 */
#endif

/* RF LDO control */
#ifdef JAWBREAKER
#define SCU_RF_LDO_ENABLE   (P5_0)  /* GPIO2[9] on P5_0 */
#endif

/* RF supply (VAA) control */
#ifdef HACKRF_ONE
#define SCU_NO_VAA_ENABLE   (P5_0)  /* GPIO2[9] on P5_0 */
#endif
#ifdef RAD1O
#define SCU_VAA_ENABLE      (P5_0)  /* GPIO2[9] on P5_0 */
#endif


/* SPI flash */
#define SCU_SSP0_MISO       (P3_6)
#define SCU_SSP0_MOSI       (P3_7)
#define SCU_SSP0_SCK        (P3_3)
#define SCU_SSP0_SSEL       (P3_8) /* GPIO5[11] on P3_8 */
#define SCU_FLASH_HOLD      (P3_4) /* GPIO1[14] on P3_4 */
#define SCU_FLASH_WP        (P3_5) /* GPIO1[15] on P3_5 */

/* RF switch control */
#ifdef HACKRF_ONE
#define SCU_HP              (P4_0)  /* GPIO2[0] on P4_0 */
#define SCU_LP              (P5_1)  /* GPIO2[10] on P5_1 */
#define SCU_TX_MIX_BP       (P5_2)  /* GPIO2[11] on P5_2 */
#define SCU_NO_MIX_BYPASS   (P1_7)  /* GPIO1[0] on P1_7 */
#define SCU_RX_MIX_BP       (P5_3)  /* GPIO2[12] on P5_3 */
#define SCU_TX_AMP          (P5_6)  /* GPIO2[15] on P5_6 */
#define SCU_TX              (P6_7)  /* GPIO5[15] on P6_7 */
#define SCU_MIX_BYPASS      (P6_8)  /* GPIO5[16] on P6_8 */
#define SCU_RX              (P2_5)  /* GPIO5[5] on P2_5 */
#define SCU_NO_TX_AMP_PWR   (P6_9)  /* GPIO3[5] on P6_9 */
#define SCU_AMP_BYPASS      (P2_10) /* GPIO0[14] on P2_10 */
#define SCU_RX_AMP          (P2_11) /* GPIO1[11] on P2_11 */
#define SCU_NO_RX_AMP_PWR   (P2_12) /* GPIO1[12] on P2_12 */
#endif
#ifdef RAD1O
#define SCU_BY_AMP          (P1_7)  /* GPIO1[0] on P1_7 */
#define SCU_BY_AMP_N        (P2_5)  /* GPIO5[5] on P2_5 */
#define SCU_TX_RX           (P2_10) /* GPIO0[14] on P2_10 */
#define SCU_TX_RX_N         (P2_11) /* GPIO1[11] on P2_11 */
#define SCU_BY_MIX          (P2_12) /* GPIO1[12] on P2_12 */
#define SCU_BY_MIX_N        (P5_1) /* GPIO2[10] on P5_1 */
#define SCU_LOW_HIGH_FILT   (P5_2) /* GPIO2[11] on P5_2 */
#define SCU_LOW_HIGH_FILT_N (P5_3) /* GPIO2[12] on P5_3 */
#define SCU_TX_AMP          (P5_6) /* GPIO2[15] on P5_6 */
#define SCU_RX_LNA          (P6_7) /* GPIO5[15] on P6_7 */
#endif

/* TODO add other Pins */
#define SCU_PINMUX_GPIO3_8  (P7_0)  /* GPIO3[8] */
#define SCU_PINMUX_GPIO3_9  (P7_1)  /* GPIO3[9] */
#define SCU_PINMUX_GPIO3_10 (P7_2)  /* GPIO3[10] */
#define SCU_PINMUX_GPIO3_11 (P7_3)  /* GPIO3[11] */
#define SCU_PINMUX_GPIO3_12 (P7_4)  /* GPIO3[12] */
#define SCU_PINMUX_GPIO3_13 (P7_5)  /* GPIO3[13] */
#define SCU_PINMUX_GPIO3_14 (P7_6)  /* GPIO3[14] */
#define SCU_PINMUX_GPIO3_15 (P7_7)  /* GPIO3[15] */

#define SCU_PINMUX_SD_POW   (P1_5)  /* GPIO1[8] */
#define SCU_PINMUX_SD_CMD   (P1_6)  /* GPIO1[9] */
#define SCU_PINMUX_SD_VOLT0 (P1_8)  /* GPIO1[1] */
#define SCU_PINMUX_SD_DAT0  (P1_9)  /* GPIO1[2] */
#define SCU_PINMUX_SD_DAT1  (P1_10) /* GPIO1[3] */
#define SCU_PINMUX_SD_DAT2  (P1_11) /* GPIO1[4] */
#define SCU_PINMUX_SD_DAT3  (P1_12) /* GPIO1[5] */
#define SCU_PINMUX_SD_CD    (P1_13) /* GPIO1[6] */

#define SCU_PINMUX_U0_TXD   (P2_0)  /* GPIO5[0] */
#define SCU_PINMUX_U0_RXD   (P2_1)  /* GPIO5[1] */

#define SCU_PINMUX_ISP      (P2_7)  /* GPIO0[7] */

#define SCU_PINMUX_GP_CLKIN	(P4_7)

typedef enum {
	TRANSCEIVER_MODE_OFF = 0,
	TRANSCEIVER_MODE_RX = 1,
	TRANSCEIVER_MODE_TX = 2,
	TRANSCEIVER_MODE_SS = 3,
	TRANSCEIVER_MODE_CPLD_UPDATE = 4
} transceiver_mode_t;

typedef enum {
	HW_SYNC_MODE_OFF = 0,
	HW_SYNC_MODE_ON = 1,
} hw_sync_mode_t;

void delay(uint32_t duration);

/* TODO: Hide these configurations */
extern si5351c_driver_t clock_gen;
extern const ssp_config_t ssp_config_w25q80bv;
extern const ssp_config_t ssp_config_max2837;
extern const ssp_config_t ssp_config_max5864;

extern max2837_driver_t max2837;
extern max5864_driver_t max5864;
extern mixer_driver_t mixer;
extern w25q80bv_driver_t spi_flash;
extern sgpio_config_t sgpio_config;
extern rf_path_t rf_path;
extern jtag_t jtag_cpld;
extern i2c_bus_t i2c0;

void cpu_clock_init(void);
void cpu_clock_pll1_low_speed(void);
void cpu_clock_pll1_max_speed(void);
void ssp1_set_mode_max2837(void);
void ssp1_set_mode_max5864(void);

void pin_setup(void);

void enable_1v8_power(void);
void disable_1v8_power(void);

bool sample_rate_frac_set(uint32_t rate_num, uint32_t rate_denom);
bool sample_rate_set(const uint32_t sampling_rate_hz);
bool baseband_filter_bandwidth_set(const uint32_t bandwidth_hz);

#if (defined HACKRF_ONE || defined RAD1O)
void enable_rf_power(void);
void disable_rf_power(void);
#endif

typedef enum {
	LED1 = 0,
	LED2 = 1,
	LED3 = 2,
	LED4 = 3,
} led_t;

void led_on(const led_t led);
void led_off(const led_t led);
void led_toggle(const led_t led);

void hw_sync_enable(const hw_sync_mode_t hw_sync_mode);


#ifdef __cplusplus
}
#endif

#endif /* __HACKRF_CORE_H */
