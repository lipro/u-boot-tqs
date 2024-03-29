/*
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 *
 * (C) Copyright 2008-2009 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __BOARD_TQMA35_H
#define __BOARD_TQMA35_H

#define UNALIGNED_ACCESS_ENABLE
#define LOW_INT_LATENCY_ENABLE
#define BRANCH_PREDICTION_ENABLE

#define L2CC_AUX_CTL_CONFIG	0x00030024

#define AIPS_MPR_CONFIG		0x77777777
#define AIPS_OPACR_CONFIG	0x00000000

/* MPR - priority is M4 > M2 > M3 > M5 > M0 > M1 */
#define MAX_MPR_CONFIG		0x00302154
/* SGPCR - always park on last master */
#define MAX_SGPCR_CONFIG	0x00000010
/* MGPCR - restore default values */
#define MAX_MGPCR_CONFIG	0x00000000

/*
 * M3IF Control Register (M3IFCTL)
 * MRRP[0] = L2CC0 not on priority list (0 << 0) = 0x00000000
 * MRRP[1] = L2CC1 not on priority list (0 << 0) = 0x00000000
 * MRRP[2] = MBX not on priority list (0 << 0)   = 0x00000000
 * MRRP[3] = MAX1 not on priority list (0 << 0)  = 0x00000000
 * MRRP[4] = SDMA not on priority list (0 << 0)  = 0x00000000
 * MRRP[5] = MPEG4 not on priority list (0 << 0) = 0x00000000
 * MRRP[6] = IPU1 on priority list (1 << 6)      = 0x00000040
 * MRRP[7] = IPU2 not on priority list (0 << 0)  = 0x00000000
 *                                               ------------
 *                                                 0x00000040
 */
#define M3IF_CONFIG	0x00000040

#define DBG_BASE_ADDR           WEIM_CTRL_CS1
#define DBG_CSCR_U_CONFIG       0x0000D843
#define DBG_CSCR_L_CONFIG       0x22252521
#define DBG_CSCR_A_CONFIG       0x22220A00

#define WEIM_WCR_00000200	0x00000200

#define CCM_CCMR_CONFIG		0x203F4208
#define CCM_PDR0_CONFIG		0x00001000

#define PLL_BRM_OFFSET	31
#define PLL_PD_OFFSET	26
#define PLL_MFD_OFFSET	16
#define PLL_MFI_OFFSET	10

#define _PLL_BRM(x)	((x) << PLL_BRM_OFFSET)
#define _PLL_PD(x)	(((x) - 1) << PLL_PD_OFFSET)
#define _PLL_MFD(x)	(((x) - 1) << PLL_MFD_OFFSET)
#define _PLL_MFI(x)	((x) << PLL_MFI_OFFSET)
#define _PLL_MFN(x)	(x)
#define _PLL_SETTING(brm, pd, mfd, mfi, mfn) \
	(_PLL_BRM(brm) | _PLL_PD(pd) | _PLL_MFD(mfd) | _PLL_MFI(mfi) |\
	 _PLL_MFN(mfn))

#define CCM_MPLL_532_HZ	_PLL_SETTING(1, 1, 12, 11, 1)
#define CCM_MPLL_399_HZ _PLL_SETTING(0, 1, 16, 8, 5)
#define CCM_PPLL_300_HZ _PLL_SETTING(0, 1, 4, 6, 1)

/*MEMORY SETING*/
#define MEMORY_MDDR_ENABLE

#define ESDCTL_0x92220000	0x92220000
#define ESDCTL_0xA2220000	0xA2220000
#define ESDCTL_0xB2220000	0xB2220000
#define ESDCTL_0x82228080 	0x82228080

#define ESDCTL_PRECHARGE	0x00000400

#define ESDCTL_MDDR_CONFIG 	0x005AE829
#define ESDCTL_MDDR_MR		0x00000033
#define ESDCTL_MDDR_EMR		0x02000020

#define ESDCTL_DDR2_CONFIG 	0x007FFC3F
#define ESDCTL_DDR2_EMR2	0x04000000
#define ESDCTL_DDR2_EMR3	0x06000000
#define ESDCTL_DDR2_EN_DLL	0x02000400
#define ESDCTL_DDR2_RESET_DLL	0x00000333
#define ESDCTL_DDR2_MR		0x00000233
#define ESDCTL_DDR2_OCD_DEFAULT 0x02000780

#define ESDCTL_DELAY_LINE5	0x00F49F00
#endif				/* __BOARD_TQMA35_H */
