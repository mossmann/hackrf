/*
 * Copyright 2012 Jared Boone
 * Copyright 2013 Benjamin Vernoux
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

#include "rf_path.h"

#include <libopencm3/lpc43xx/scu.h>

#include <hackrf_core.h>

#include "hackrf-ui.h"

#include <mixer.h>
#include <max2837.h>
#include <max5864.h>
#include <sgpio.h>

#if (defined JAWBREAKER || defined HACKRF_ONE || defined RAD1O)
/*
 * RF switches on Jawbreaker are controlled by General Purpose Outputs (GPO) on
 * the RFFC5072.
 *
 * On HackRF One, the same signals are controlled by GPIO on the LPC.
 * SWITCHCTRL_NO_TX_AMP_PWR and SWITCHCTRL_NO_RX_AMP_PWR are not normally used
 * on HackRF One as the amplifier power is instead controlled only by
 * SWITCHCTRL_AMP_BYPASS.
 *
 * The rad1o also uses GPIO pins to control the different switches. The amplifiers
 * are also connected to the LPC.
 */
#define SWITCHCTRL_NO_TX_AMP_PWR (1 << 0) /* GPO1 turn off TX amp power */
#define SWITCHCTRL_AMP_BYPASS    (1 << 1) /* GPO2 bypass amp section */
#define SWITCHCTRL_TX            (1 << 2) /* GPO3 1 for TX mode, 0 for RX mode */
#define SWITCHCTRL_MIX_BYPASS    (1 << 3) /* GPO4 bypass RFFC5072 mixer section */
#define SWITCHCTRL_HP            (1 << 4) /* GPO5 1 for high-pass, 0 for low-pass */
#define SWITCHCTRL_NO_RX_AMP_PWR (1 << 5) /* GPO6 turn off RX amp power */

/*
 GPO6  GPO5  GPO4 GPO3  GPO2  GPO1
!RXAMP  HP  MIXBP  TX  AMPBP !TXAMP  Mix mode   Amp mode
   1    X     1    1     1      1    TX bypass  Bypass
   1    X     1    1     0      0    TX bypass  TX amplified
   1    1     0    1     1      1    TX high    Bypass
   1    1     0    1     0      0    TX high    TX amplified
   1    0     0    1     1      1    TX low     Bypass
   1    0     0    1     0      0    TX low     TX amplified
   1    X     1    0     1      1    RX bypass  Bypass
   0    X     1    0     0      1    RX bypass  RX amplified
   1    1     0    0     1      1    RX high    Bypass
   0    1     0    0     0      1    RX high    RX amplified
   1    0     0    0     1      1    RX low     Bypass
   0    0     0    0     0      1    RX low     RX amplified
*/

/*
 * Safe (initial) switch settings turn off both amplifiers and enable both amp
 * bypass and mixer bypass.
 */
#define SWITCHCTRL_SAFE (SWITCHCTRL_NO_TX_AMP_PWR | SWITCHCTRL_AMP_BYPASS | SWITCHCTRL_TX | SWITCHCTRL_MIX_BYPASS | SWITCHCTRL_HP | SWITCHCTRL_NO_RX_AMP_PWR)
#endif

uint8_t switchctrl = SWITCHCTRL_SAFE;

/*
 * Antenna port power on HackRF One is controlled by GPO1 on the RFFC5072.
 * This is the only thing we use RFFC5072 GPO for on HackRF One.  The value of
 * SWITCHCTRL_NO_ANT_PWR does not correspond to the GPO1 bit in the gpo
 * register.
 */
#define SWITCHCTRL_ANT_PWR (1 << 6) /* turn on antenna port power */

#ifdef HACKRF_ONE
static void switchctrl_set_hackrf_one(rf_path_t* const rf_path, uint8_t ctrl) {
	if (ctrl & SWITCHCTRL_TX) {
		gpio_set(rf_path->gpio_tx);
		gpio_clear(rf_path->gpio_rx);
	} else {
		gpio_clear(rf_path->gpio_tx);
		gpio_set(rf_path->gpio_rx);
	}

	if (ctrl & SWITCHCTRL_MIX_BYPASS) {
		gpio_set(rf_path->gpio_mix_bypass);
		gpio_clear(rf_path->gpio_no_mix_bypass);
		if (ctrl & SWITCHCTRL_TX) {
			gpio_set(rf_path->gpio_tx_mix_bp);
			gpio_clear(rf_path->gpio_rx_mix_bp);
		} else {
			gpio_clear(rf_path->gpio_tx_mix_bp);
			gpio_set(rf_path->gpio_rx_mix_bp);
		}
	} else {
		gpio_clear(rf_path->gpio_mix_bypass);
		gpio_set(rf_path->gpio_no_mix_bypass);
		gpio_clear(rf_path->gpio_tx_mix_bp);
		gpio_clear(rf_path->gpio_rx_mix_bp);
	}

	if (ctrl & SWITCHCTRL_HP) {
		gpio_set(rf_path->gpio_hp);
		gpio_clear(rf_path->gpio_lp);
	} else {
		gpio_clear(rf_path->gpio_hp);
		gpio_set(rf_path->gpio_lp);
	}

	if (ctrl & SWITCHCTRL_AMP_BYPASS) {
		gpio_set(rf_path->gpio_amp_bypass);
		gpio_clear(rf_path->gpio_tx_amp);
		gpio_set(rf_path->gpio_no_tx_amp_pwr);
		gpio_clear(rf_path->gpio_rx_amp);
		gpio_set(rf_path->gpio_no_rx_amp_pwr);
	} else if (ctrl & SWITCHCTRL_TX) {
		gpio_clear(rf_path->gpio_amp_bypass);
		gpio_set(rf_path->gpio_tx_amp);
		gpio_clear(rf_path->gpio_no_tx_amp_pwr);
		gpio_clear(rf_path->gpio_rx_amp);
		gpio_set(rf_path->gpio_no_rx_amp_pwr);
	} else {
		gpio_clear(rf_path->gpio_amp_bypass);
		gpio_clear(rf_path->gpio_tx_amp);
		gpio_set(rf_path->gpio_no_tx_amp_pwr);
		gpio_set(rf_path->gpio_rx_amp);
		gpio_clear(rf_path->gpio_no_rx_amp_pwr);
	}

	/*
	 * These normally shouldn't be set post-Jawbreaker, but they can be
	 * used to explicitly turn off power to the amplifiers while AMP_BYPASS
	 * is unset:
	 */
	if (ctrl & SWITCHCTRL_NO_TX_AMP_PWR)
		gpio_set(rf_path->gpio_no_tx_amp_pwr);
	if (ctrl & SWITCHCTRL_NO_RX_AMP_PWR)
		gpio_set(rf_path->gpio_no_rx_amp_pwr);

	if (ctrl & SWITCHCTRL_ANT_PWR) {
		mixer_set_gpo(&mixer, 0x00); /* turn on antenna power by clearing GPO1 */
	} else {
		mixer_set_gpo(&mixer, 0x01); /* turn off antenna power by setting GPO1 */
	}
}
#endif

#ifdef RAD1O
static void switchctrl_set_rad1o(rf_path_t* const rf_path, uint8_t ctrl) {
	if (ctrl & SWITCHCTRL_TX) {
		gpio_set(rf_path->gpio_tx_rx_n);
		gpio_clear(rf_path->gpio_tx_rx);
	} else {
		gpio_clear(rf_path->gpio_tx_rx_n);
		gpio_set(rf_path->gpio_tx_rx);
	}

	if (ctrl & SWITCHCTRL_MIX_BYPASS) {
		gpio_clear(rf_path->gpio_by_mix);
		gpio_set(rf_path->gpio_by_mix_n);
		gpio_clear(rf_path->gpio_mixer_en);
	} else {
		gpio_set(rf_path->gpio_by_mix);
		gpio_clear(rf_path->gpio_by_mix_n);
		gpio_set(rf_path->gpio_mixer_en);
	}

	if (ctrl & SWITCHCTRL_HP) {
		gpio_set(rf_path->gpio_low_high_filt);
		gpio_clear(rf_path->gpio_low_high_filt_n);
	} else {
		gpio_clear(rf_path->gpio_low_high_filt);
		gpio_set(rf_path->gpio_low_high_filt_n);
	}

	if (ctrl & SWITCHCTRL_AMP_BYPASS) {
		gpio_clear(rf_path->gpio_by_amp);
		gpio_set(rf_path->gpio_by_amp_n);

		gpio_clear(rf_path->gpio_tx_amp);
		gpio_clear(rf_path->gpio_rx_lna);

	} else if (ctrl & SWITCHCTRL_TX) {
		gpio_set(rf_path->gpio_by_amp);
		gpio_clear(rf_path->gpio_by_amp_n);

		gpio_set(rf_path->gpio_tx_amp);
		gpio_clear(rf_path->gpio_rx_lna);

	} else {
		gpio_set(rf_path->gpio_by_amp);
		gpio_clear(rf_path->gpio_by_amp_n);

		gpio_clear(rf_path->gpio_tx_amp);
		gpio_set(rf_path->gpio_rx_lna);
	}

	/*
	 * These normally shouldn't be set post-Jawbreaker, but they can be
	 * used to explicitly turn off power to the amplifiers while AMP_BYPASS
	 * is unset:
	 */
	if (ctrl & SWITCHCTRL_NO_TX_AMP_PWR) {
		gpio_clear(rf_path->gpio_tx_amp);
	}
	if (ctrl & SWITCHCTRL_NO_RX_AMP_PWR) {
		gpio_clear(rf_path->gpio_rx_lna);
	}
}
#endif

static void switchctrl_set(rf_path_t* const rf_path, const uint8_t gpo) {
#ifdef JAWBREAKER
	(void) rf_path; /* silence unused param warning */
	mixer_set_gpo(&mixer, gpo);
#elif HACKRF_ONE
	switchctrl_set_hackrf_one(rf_path, gpo);
#elif RAD1O
	switchctrl_set_rad1o(rf_path, gpo);
#else
	(void)gpo;
#endif
}

void rf_path_pin_setup(rf_path_t* const rf_path) {
#ifdef HACKRF_ONE
	/* Configure RF switch control signals */
	scu_pinmux(SCU_HP,             SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_LP,             SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_TX_MIX_BP,      SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_NO_MIX_BYPASS,  SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_RX_MIX_BP,      SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_TX_AMP,         SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_TX,             SCU_GPIO_FAST | SCU_CONF_FUNCTION4);
	scu_pinmux(SCU_MIX_BYPASS,     SCU_GPIO_FAST | SCU_CONF_FUNCTION4);
	scu_pinmux(SCU_RX,             SCU_GPIO_FAST | SCU_CONF_FUNCTION4);
	scu_pinmux(SCU_NO_TX_AMP_PWR,  SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_AMP_BYPASS,     SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_RX_AMP,         SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_NO_RX_AMP_PWR,  SCU_GPIO_FAST | SCU_CONF_FUNCTION0);

	/* Configure RF power supply (VAA) switch */
	scu_pinmux(SCU_NO_VAA_ENABLE,  SCU_GPIO_FAST | SCU_CONF_FUNCTION0);

	/* Configure RF switch control signals as outputs */
	gpio_output(rf_path->gpio_amp_bypass);
	gpio_output(rf_path->gpio_no_mix_bypass);
	gpio_output(rf_path->gpio_rx_amp);
	gpio_output(rf_path->gpio_no_rx_amp_pwr);
	gpio_output(rf_path->gpio_hp);
	gpio_output(rf_path->gpio_lp);
	gpio_output(rf_path->gpio_tx_mix_bp);
	gpio_output(rf_path->gpio_rx_mix_bp);
	gpio_output(rf_path->gpio_tx_amp);
	gpio_output(rf_path->gpio_no_tx_amp_pwr);
	gpio_output(rf_path->gpio_tx);
	gpio_output(rf_path->gpio_mix_bypass);
	gpio_output(rf_path->gpio_rx);

	/*
	 * Safe (initial) switch settings turn off both amplifiers and antenna port
	 * power and enable both amp bypass and mixer bypass.
	 */
	switchctrl_set(rf_path, SWITCHCTRL_AMP_BYPASS | SWITCHCTRL_MIX_BYPASS);
#elif RAD1O
	/* Configure RF switch control signals */
	scu_pinmux(SCU_BY_AMP,         SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_BY_AMP_N,       SCU_GPIO_FAST | SCU_CONF_FUNCTION4);
	scu_pinmux(SCU_TX_RX,          SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_TX_RX_N,        SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_BY_MIX,         SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_BY_MIX_N,       SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_LOW_HIGH_FILT,  SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_LOW_HIGH_FILT_N,SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_TX_AMP,         SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_RX_LNA,         SCU_GPIO_FAST | SCU_CONF_FUNCTION4);
	scu_pinmux(SCU_MIXER_EN,       SCU_GPIO_FAST | SCU_CONF_FUNCTION4);

	/* Configure RF power supply (VAA) switch */
	scu_pinmux(SCU_VAA_ENABLE,  SCU_GPIO_FAST | SCU_CONF_FUNCTION0);

	/* Configure RF switch control signals as outputs */
	gpio_output(rf_path->gpio_tx_rx_n);
	gpio_output(rf_path->gpio_tx_rx);
	gpio_output(rf_path->gpio_by_mix);
	gpio_output(rf_path->gpio_by_mix_n);
	gpio_output(rf_path->gpio_by_amp);
	gpio_output(rf_path->gpio_by_amp_n);
	gpio_output(rf_path->gpio_mixer_en);
	gpio_output(rf_path->gpio_low_high_filt);
	gpio_output(rf_path->gpio_low_high_filt_n);
	gpio_output(rf_path->gpio_tx_amp);
	gpio_output(rf_path->gpio_rx_lna);

	/*
	 * Safe (initial) switch settings turn off both amplifiers and antenna port
	 * power and enable both amp bypass and mixer bypass.
	 */
	switchctrl_set(rf_path, SWITCHCTRL_AMP_BYPASS | SWITCHCTRL_MIX_BYPASS);
#else
	(void) rf_path; /* silence unused param warning */
#endif
}

void rf_path_init(rf_path_t* const rf_path) {
	ssp1_set_mode_max5864();
	max5864_setup(&max5864);
	max5864_shutdown(&max5864);
	
	ssp1_set_mode_max2837();
	max2837_setup(&max2837);
	max2837_start(&max2837);
	
	mixer_setup(&mixer);
	switchctrl_set(rf_path, switchctrl);
}

void rf_path_set_direction(rf_path_t* const rf_path, const rf_path_direction_t direction) {
	/* Turn off TX and RX amplifiers, then enable based on direction and bypass state. */
	rf_path->switchctrl |= SWITCHCTRL_NO_TX_AMP_PWR | SWITCHCTRL_NO_RX_AMP_PWR;
	switch(direction) {
	case RF_PATH_DIRECTION_TX:
		rf_path->switchctrl |= SWITCHCTRL_TX;
		if( (rf_path->switchctrl & SWITCHCTRL_AMP_BYPASS) == 0 ) {
			/* TX amplifier is in path, be sure to enable TX amplifier. */
			rf_path->switchctrl &= ~SWITCHCTRL_NO_TX_AMP_PWR;
		}
		mixer_tx(&mixer);
		if( rf_path->switchctrl & SWITCHCTRL_MIX_BYPASS ) {
			mixer_disable(&mixer);
		} else {
			mixer_enable(&mixer);
		}
		ssp1_set_mode_max5864();
		max5864_tx(&max5864);
		ssp1_set_mode_max2837();
		max2837_tx(&max2837);
		sgpio_configure(&sgpio_config, SGPIO_DIRECTION_TX);
		break;
	
	case RF_PATH_DIRECTION_RX:
		rf_path->switchctrl &= ~SWITCHCTRL_TX;
		if( (rf_path->switchctrl & SWITCHCTRL_AMP_BYPASS) == 0 ) {
			/* RX amplifier is in path, be sure to enable RX amplifier. */
			rf_path->switchctrl &= ~SWITCHCTRL_NO_RX_AMP_PWR;
		}
		mixer_rx(&mixer);
		if( rf_path->switchctrl & SWITCHCTRL_MIX_BYPASS ) {
			mixer_disable(&mixer);
		} else {
			mixer_enable(&mixer);
		}
		ssp1_set_mode_max5864();
		max5864_rx(&max5864);
		ssp1_set_mode_max2837();
		max2837_rx(&max2837);
		sgpio_configure(&sgpio_config, SGPIO_DIRECTION_RX);
		break;
		
	case RF_PATH_DIRECTION_OFF:
	default:
#ifdef HACKRF_ONE
		rf_path_set_antenna(rf_path, 0);
#endif
		rf_path_set_lna(rf_path, 0);
		/* Set RF path to receive direction when "off" */
		rf_path->switchctrl &= ~SWITCHCTRL_TX;
		mixer_disable(&mixer);
		ssp1_set_mode_max5864();
		max5864_standby(&max5864);
		ssp1_set_mode_max2837();
		max2837_set_mode(&max2837, MAX2837_MODE_STANDBY);
		sgpio_configure(&sgpio_config, SGPIO_DIRECTION_RX);
		break;
	}

	switchctrl_set(rf_path, rf_path->switchctrl);

	hackrf_ui_setDirection(direction);
}

void rf_path_set_filter(rf_path_t* const rf_path, const rf_path_filter_t filter) {
	switch(filter) {
	default:
	case RF_PATH_FILTER_BYPASS:
		rf_path->switchctrl |= SWITCHCTRL_MIX_BYPASS;
		mixer_disable(&mixer);
		break;
		
	case RF_PATH_FILTER_LOW_PASS:
		rf_path->switchctrl &= ~(SWITCHCTRL_HP | SWITCHCTRL_MIX_BYPASS);
		mixer_enable(&mixer);
		break;
		
	case RF_PATH_FILTER_HIGH_PASS:
		rf_path->switchctrl &= ~SWITCHCTRL_MIX_BYPASS;
		rf_path->switchctrl |= SWITCHCTRL_HP;
		mixer_enable(&mixer);
		break;
	}

	switchctrl_set(rf_path, rf_path->switchctrl);
}

void rf_path_set_lna(rf_path_t* const rf_path, const uint_fast8_t enable) {
	if( enable ) {
		if( rf_path->switchctrl & SWITCHCTRL_TX ) {
			/* AMP_BYPASS=0, NO_RX_AMP_PWR=1, NO_TX_AMP_PWR=0 */
			rf_path->switchctrl |= SWITCHCTRL_NO_RX_AMP_PWR;
			rf_path->switchctrl &= ~(SWITCHCTRL_AMP_BYPASS | SWITCHCTRL_NO_TX_AMP_PWR);
		} else {
			/* AMP_BYPASS=0, NO_RX_AMP_PWR=0, NO_TX_AMP_PWR=1 */
			rf_path->switchctrl |= SWITCHCTRL_NO_TX_AMP_PWR;
			rf_path->switchctrl &= ~(SWITCHCTRL_AMP_BYPASS | SWITCHCTRL_NO_RX_AMP_PWR);
		}
	} else {
		/* AMP_BYPASS=1, NO_RX_AMP_PWR=1, NO_TX_AMP_PWR=1 */
		rf_path->switchctrl |= SWITCHCTRL_AMP_BYPASS | SWITCHCTRL_NO_TX_AMP_PWR | SWITCHCTRL_NO_RX_AMP_PWR;
	}
	
	switchctrl_set(rf_path, rf_path->switchctrl);

	hackrf_ui_setLNAPower(enable);
}

/* antenna port power control */
void rf_path_set_antenna(rf_path_t* const rf_path, const uint_fast8_t enable) {
	if (enable) {
		rf_path->switchctrl |= SWITCHCTRL_ANT_PWR;
	} else {
		rf_path->switchctrl &= ~(SWITCHCTRL_ANT_PWR);
	}

	switchctrl_set(rf_path, rf_path->switchctrl);
}
