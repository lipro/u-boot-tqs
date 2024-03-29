/*
 * Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
 *
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
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

#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/mx35.h>
#include <asm/arch/mx35_pins.h>
#include <asm/arch/iomux.h>
#include <i2c.h>
#include <netdev.h>

#ifdef CONFIG_ARCH_MMU
#include <asm/mmu.h>
#include <asm/arch/mmu.h>
#endif

#ifdef CONFIG_CMD_MMC
#include <mmc.h>
#include <fsl_esdhc.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

static u32 system_rev;

u32 get_board_rev(void)
{
	return system_rev;
}

static inline void setup_soc_rev(void)
{
	int reg;
	reg = __REG(IIM_BASE_ADDR + IIM_SREV);
	if (!reg) {
		reg = __REG(ROMPATCH_REV);
		reg <<= 4;
	} else
		reg += CHIP_REV_1_0;
	system_rev = 0x35000 + (reg & 0xFF);
}

static inline void set_board_rev(int rev)
{
	system_rev =  (system_rev & ~(0xF << 8)) | (rev & 0xF) << 8;
}

int is_soc_rev(int rev)
{
	return (system_rev & 0xFF) - rev;
}

#ifdef CONFIG_ARCH_MMU
void board_mmu_init(void)
{
	unsigned long ttb_base = PHYS_SDRAM_1 + 0x40000;
	unsigned long i;

	/*
	 * Set the TTB register
	 */
	asm volatile ("mcr  p15,0,%0,c2,c0,0" : : "r"(ttb_base) /*:*/);

	/*
	 * Set the Domain Access Control Register
	 */
	i = ARM_ACCESS_DACR_DEFAULT;
	asm volatile ("mcr  p15,0,%0,c3,c0,0" : : "r"(i) /*:*/);

	/*
	 * First clear all TT entries - ie Set them to Faulting
	 */
	memset((void *)ttb_base, 0, ARM_FIRST_LEVEL_PAGE_TABLE_SIZE);
	/* Actual   Virtual  Size   Attributes   	Function */
	/* Base     Base     MB     cached? buffered?  access permissions */
	/* xxx00000 xxx00000 */
	X_ARM_MMU_SECTION(0x000, 0xF00, 0x1,
			  ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			  ARM_ACCESS_PERM_RW_RW); /* ROM */
	X_ARM_MMU_SECTION(0x100, 0x100, 0x1,
			  ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			  ARM_ACCESS_PERM_RW_RW); /* iRAM */
	X_ARM_MMU_SECTION(0x300, 0x300, 0x1,
			  ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			  ARM_ACCESS_PERM_RW_RW); /* L2CC */
	/* Internal Regsisters upto SDRAM*/
	X_ARM_MMU_SECTION(0x400, 0x400, 0x400,
			  ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			  ARM_ACCESS_PERM_RW_RW);
	X_ARM_MMU_SECTION(0x800, 0x000, 0x80,
			  ARM_CACHEABLE, ARM_BUFFERABLE,
			  ARM_ACCESS_PERM_RW_RW); /* SDRAM 0:128M*/
	X_ARM_MMU_SECTION(0x800, 0x800, 0x80,
			  ARM_CACHEABLE, ARM_BUFFERABLE,
			  ARM_ACCESS_PERM_RW_RW); /* SDRAM 0:128M*/
	X_ARM_MMU_SECTION(0x800, 0x880, 0x80,
			  ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			  ARM_ACCESS_PERM_RW_RW); /* SDRAM 0:128M*/
	X_ARM_MMU_SECTION(0x900, 0x900, 0x80,
			  ARM_CACHEABLE, ARM_BUFFERABLE,
			  ARM_ACCESS_PERM_RW_RW); /* SDRAM 1:128M*/
	X_ARM_MMU_SECTION(0xA00, 0xA00, 0x40,
			  ARM_CACHEABLE, ARM_BUFFERABLE,
			  ARM_ACCESS_PERM_RW_RW); /* Flash */
	X_ARM_MMU_SECTION(0xB00, 0xB00, 0x20,
			  ARM_CACHEABLE, ARM_BUFFERABLE,
			  ARM_ACCESS_PERM_RW_RW); /* PSRAM */
	/* ESDCTL, WEIM, M3IF, EMI, NFC, External I/O */
	X_ARM_MMU_SECTION(0xB20, 0xB20, 0x1E0,
			  ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			  ARM_ACCESS_PERM_RW_RW);
}
#endif

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

#if CONFIG_NR_DRAM_BANKS > 1
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
#endif

	return 0;
}

#ifdef CONFIG_CMD_MMC

#define EMMC_PAD_CTL1 (PAD_CTL_47K_PU | PAD_CTL_PUE_PUD | \
			PAD_CTL_HYS_SCHMITZ | PAD_CTL_DRV_HIGH | \
			PAD_CTL_PKE_ENABLE | PAD_CTL_SRE_FAST)

u32 *imx_esdhc_base_addr;

static int esdhc_gpio_init(u32  interface_esdhc)
{
	switch (interface_esdhc) {
	case 0:
		imx_esdhc_base_addr = (u32 *)MMC_SDHC1_BASE_ADDR;

		/* Pins */
		mxc_request_iomux(MX35_PIN_SD1_CMD,
			MUX_CONFIG_FUNC | MUX_CONFIG_SION);

		/* D0 ..D3 */
		mxc_request_iomux(MX35_PIN_SD1_CLK,
			MUX_CONFIG_FUNC | MUX_CONFIG_SION);

		mxc_request_iomux(MX35_PIN_SD1_DATA0,
			MUX_CONFIG_FUNC);

		mxc_request_iomux(MX35_PIN_SD1_DATA1,
			MUX_CONFIG_FUNC);

		mxc_request_iomux(MX35_PIN_SD1_DATA2,
			MUX_CONFIG_FUNC);

		mxc_request_iomux(MX35_PIN_SD1_DATA3,
			MUX_CONFIG_FUNC);

		/* Pads */
		mxc_iomux_set_pad(MX35_PIN_SD1_CMD, EMMC_PAD_CTL1);
		mxc_iomux_set_pad(MX35_PIN_SD1_CLK, EMMC_PAD_CTL1);
		mxc_iomux_set_pad(MX35_PIN_SD1_DATA0, EMMC_PAD_CTL1);
		mxc_iomux_set_pad(MX35_PIN_SD1_DATA1, EMMC_PAD_CTL1);
		mxc_iomux_set_pad(MX35_PIN_SD1_DATA2, EMMC_PAD_CTL1);
		mxc_iomux_set_pad(MX35_PIN_SD1_DATA3, EMMC_PAD_CTL1);

		break;
	case 1:
		imx_esdhc_base_addr = (u32 *)MMC_SDHC3_BASE_ADDR;

			/* SD3_CMD */
			mxc_request_iomux(MX35_PIN_ATA_DATA4,
				  MUX_CONFIG_ALT1 | MUX_CONFIG_SION);
			/* SD3_CLK */
			mxc_request_iomux(MX35_PIN_ATA_DATA3,
				  MUX_CONFIG_ALT1 | MUX_CONFIG_SION);
			/* SD3_DAT0 */
			mxc_request_iomux(MX35_PIN_ATA_DIOR, MUX_CONFIG_ALT1);
			/* SD3_DATA1 */
			mxc_request_iomux(MX35_PIN_ATA_DIOW, MUX_CONFIG_ALT1);
			/* SD3_DATA2 */
			mxc_request_iomux(MX35_PIN_ATA_DMACK, MUX_CONFIG_ALT1);
			/* SD3_DATA3 */
			mxc_request_iomux(MX35_PIN_ATA_RESET_B,
				MUX_CONFIG_ALT1);

			/* SD3_DATA4 */
			mxc_request_iomux(MX35_PIN_ATA_IORDY,
				MUX_CONFIG_ALT1);
			/* SD3_DATA5 */
			mxc_request_iomux(MX35_PIN_ATA_DATA0,
				MUX_CONFIG_ALT1);
			/* SD3_DATA6 */
			mxc_request_iomux(MX35_PIN_ATA_DATA1,
				MUX_CONFIG_ALT1);
			/* SD3_DATA7 */
			mxc_request_iomux(MX35_PIN_ATA_DATA2,
				MUX_CONFIG_ALT1);

			mxc_iomux_set_pad(MX35_PIN_ATA_DATA4, EMMC_PAD_CTL1);
			mxc_iomux_set_pad(MX35_PIN_ATA_DATA3, EMMC_PAD_CTL1);
			mxc_iomux_set_pad(MX35_PIN_ATA_DIOR, EMMC_PAD_CTL1);
			mxc_iomux_set_pad(MX35_PIN_ATA_DIOW, EMMC_PAD_CTL1);
			mxc_iomux_set_pad(MX35_PIN_ATA_DMACK, EMMC_PAD_CTL1);
			mxc_iomux_set_pad(MX35_PIN_ATA_RESET_B, EMMC_PAD_CTL1);

			mxc_iomux_set_pad(MX35_PIN_ATA_IORDY, EMMC_PAD_CTL1);
			mxc_iomux_set_pad(MX35_PIN_ATA_DATA0, EMMC_PAD_CTL1);
			mxc_iomux_set_pad(MX35_PIN_ATA_DATA1, EMMC_PAD_CTL1);
			mxc_iomux_set_pad(MX35_PIN_ATA_DATA2, EMMC_PAD_CTL1);

			mxc_iomux_set_input(MUX_IN_ESDHC3_CARD_CLK_IN, INPUT_CTL_PATH1);
			mxc_iomux_set_input(MUX_IN_ESDHC3_CMD_IN, INPUT_CTL_PATH1);
			mxc_iomux_set_input(MUX_IN_ESDHC3_DAT0, INPUT_CTL_PATH1);
			mxc_iomux_set_input(MUX_IN_ESDHC3_DAT1, INPUT_CTL_PATH1);
			mxc_iomux_set_input(MUX_IN_ESDHC3_DAT2, INPUT_CTL_PATH1);
			mxc_iomux_set_input(MUX_IN_ESDHC3_DAT3, INPUT_CTL_PATH1);

		break;

	default:
		return 1;
		break;
	}
	return 0;
}

int board_mmc_init(void)
{
	esdhc_gpio_init(0);
	fsl_esdhc_mmc_init(gd->bd);

#if defined(CONFIG_TQMA35_MBA35CA)
/* do not init eMMC since 24V outputs on SD3_BUS on Rev.0200 */
#
#else
	esdhc_gpio_init(1);
	fsl_esdhc_mmc_init(gd->bd);
#endif
	return 0;
}
#endif

int board_init(void)
{
	int pad;

	setup_soc_rev();

	/* optimize CS0 for NOR-Flash */
	__REG(CSCR_L(0)) &= ~0x00000001;
	__REG(CSCR_U(0)) = 0x0000cf03;
	__REG(CSCR_A(0)) = 0x00220800;
	__REG(CSCR_L(0)) = 0xa0330d01;

	/* enable clocks */
	__REG(CCM_BASE_ADDR + CLKCTL_CGR0) |= 0x003F0000;
	__REG(CCM_BASE_ADDR + CLKCTL_CGR1) |= 0x00030FFF;

#if defined(CONFIG_TQMA35_MBA35CA)
#if 0
	/* REVISION_DETECT */
	pad = PAD_CTL_100K_PU | PAD_CTL_PKE_ENABLE | PAD_CTL_SRE_FAST |
		PAD_CTL_PUE_KEEPER | PAD_CTL_HYS_SCHMITZ;

	mxc_request_iomux(MX35_PIN_FSR, MUX_CONFIG_GPIO);
	mxc_iomux_set_pad(MX35_PIN_FSR, pad);
	mxc_iomux_set_input(MUX_IN_GPIO1_IN_4, INPUT_CTL_PATH1);
	mx35_gpio_direction(4, MX35_GPIO_DIRECTION_IN);
	mba35ca_rev = (mx35_gpio_get(4)) ? 200 : 300;
#endif
	/* LCD_RST */
	mxc_request_iomux(MX35_PIN_MLB_SIG, MUX_CONFIG_GPIO);
	mxc_iomux_set_input(MUX_IN_GPIO3_IN_5, INPUT_CTL_PATH1);
	mx35_gpio_set(69, 0);
	mx35_gpio_direction(69, MX35_GPIO_DIRECTION_OUT);
#endif
	/* setup pins for I2C1 */
	mxc_request_iomux(MX35_PIN_I2C1_CLK, MUX_CONFIG_SION);
	mxc_request_iomux(MX35_PIN_I2C1_DAT, MUX_CONFIG_SION);

	pad = (PAD_CTL_HYS_SCHMITZ | PAD_CTL_PKE_ENABLE \
			| PAD_CTL_PUE_PUD | PAD_CTL_ODE_OpenDrain);

	mxc_iomux_set_pad(MX35_PIN_I2C1_CLK, pad);
	mxc_iomux_set_pad(MX35_PIN_I2C1_DAT, pad);

	/* setup pins for LCD */
	pad = (PAD_CTL_HYS_CMOS | PAD_CTL_DRV_NORMAL | PAD_CTL_PKE_NONE \
		| PAD_CTL_100K_PD | PAD_CTL_ODE_CMOS | PAD_CTL_DRV_3_3V \
		| PAD_CTL_PUE_PUD | PAD_CTL_SRE_FAST);

	mxc_request_iomux(MX35_PIN_LD0, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_LD1, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_LD2, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_LD3, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_LD4, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_LD5, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_LD6, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_LD7, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_LD8, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_LD9, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_LD10, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_LD11, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_LD12, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_LD13, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_LD14, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_LD15, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_LD16, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_LD17, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_D3_VSYNC, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_D3_HSYNC, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_D3_FPSHIFT, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_D3_DRDY, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_D3_REV, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_CONTRAST, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_D3_SPL, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_D3_CLS, MUX_CONFIG_FUNC);

	mxc_iomux_set_pad(MX35_PIN_LD0, pad);
	mxc_iomux_set_pad(MX35_PIN_LD1, pad);
	mxc_iomux_set_pad(MX35_PIN_LD2, pad);
	mxc_iomux_set_pad(MX35_PIN_LD3, pad);
	mxc_iomux_set_pad(MX35_PIN_LD4, pad);
	mxc_iomux_set_pad(MX35_PIN_LD5, pad);
	mxc_iomux_set_pad(MX35_PIN_LD6, pad);
	mxc_iomux_set_pad(MX35_PIN_LD7, pad);
	mxc_iomux_set_pad(MX35_PIN_LD8, pad);
	mxc_iomux_set_pad(MX35_PIN_LD9, pad);
	mxc_iomux_set_pad(MX35_PIN_LD10, pad);
	mxc_iomux_set_pad(MX35_PIN_LD11, pad);
	mxc_iomux_set_pad(MX35_PIN_LD12, pad);
	mxc_iomux_set_pad(MX35_PIN_LD13, pad);
	mxc_iomux_set_pad(MX35_PIN_LD14, pad);
	mxc_iomux_set_pad(MX35_PIN_LD15, pad);
	mxc_iomux_set_pad(MX35_PIN_LD16, pad);
	mxc_iomux_set_pad(MX35_PIN_LD17, pad);
	mxc_iomux_set_pad(MX35_PIN_D3_VSYNC, pad);
	mxc_iomux_set_pad(MX35_PIN_D3_HSYNC, pad);
	mxc_iomux_set_pad(MX35_PIN_D3_FPSHIFT, pad);
	mxc_iomux_set_pad(MX35_PIN_D3_DRDY, pad);
	mxc_iomux_set_pad(MX35_PIN_D3_REV, pad);
	mxc_iomux_set_pad(MX35_PIN_CONTRAST, pad);
	mxc_iomux_set_pad(MX35_PIN_D3_SPL, pad);
	mxc_iomux_set_pad(MX35_PIN_D3_CLS, pad);

	/* setup pins for FEC */
	mxc_request_iomux(MX35_PIN_FEC_TX_CLK, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RX_CLK, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RX_DV, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_COL, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RDATA0, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_TDATA0, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_TX_EN, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_MDC, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_MDIO, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_TX_ERR, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RX_ERR, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_CRS, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RDATA1, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_TDATA1, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RDATA2, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_TDATA2, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_RDATA3, MUX_CONFIG_FUNC);
	mxc_request_iomux(MX35_PIN_FEC_TDATA3, MUX_CONFIG_FUNC);

	pad = (PAD_CTL_DRV_3_3V | PAD_CTL_PUE_PUD | PAD_CTL_ODE_CMOS | \
			PAD_CTL_DRV_NORMAL | PAD_CTL_SRE_SLOW);

	mxc_iomux_set_pad(MX35_PIN_FEC_TX_CLK, pad | PAD_CTL_HYS_SCHMITZ | \
			PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_RX_CLK, pad | PAD_CTL_HYS_SCHMITZ | \
			PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_RX_DV, pad | PAD_CTL_HYS_SCHMITZ | \
			 PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_COL, pad | PAD_CTL_HYS_SCHMITZ | \
			  PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_RDATA0, pad | PAD_CTL_HYS_SCHMITZ | \
			  PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_TDATA0, pad | PAD_CTL_HYS_CMOS | \
			  PAD_CTL_PKE_NONE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_TX_EN, pad | PAD_CTL_HYS_CMOS | \
			  PAD_CTL_PKE_NONE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_MDC, pad | PAD_CTL_HYS_CMOS | \
			  PAD_CTL_PKE_NONE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_MDIO, pad | PAD_CTL_HYS_SCHMITZ | \
			  PAD_CTL_PKE_ENABLE | PAD_CTL_22K_PU);
	mxc_iomux_set_pad(MX35_PIN_FEC_TX_ERR, pad | PAD_CTL_HYS_CMOS | \
			  PAD_CTL_PKE_NONE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_RX_ERR, pad | PAD_CTL_HYS_SCHMITZ | \
			  PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_CRS, pad | PAD_CTL_HYS_SCHMITZ | \
			  PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_RDATA1, pad | PAD_CTL_HYS_SCHMITZ | \
			  PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_TDATA1, pad | PAD_CTL_HYS_CMOS | \
			  PAD_CTL_PKE_NONE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_RDATA2, pad | PAD_CTL_HYS_SCHMITZ | \
			  PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_TDATA2, pad | PAD_CTL_HYS_CMOS | \
			  PAD_CTL_PKE_NONE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_RDATA3, pad | PAD_CTL_HYS_SCHMITZ | \
			  PAD_CTL_PKE_ENABLE | PAD_CTL_100K_PD);
	mxc_iomux_set_pad(MX35_PIN_FEC_TDATA3, pad | PAD_CTL_HYS_CMOS | \
			  PAD_CTL_PKE_NONE | PAD_CTL_100K_PD);

	/* setup pins for UART3 */
	mxc_request_iomux(MX35_PIN_ATA_DATA10, MUX_CONFIG_ALT1); // rxd
	mxc_request_iomux(MX35_PIN_ATA_DATA11, MUX_CONFIG_ALT1); // txd

	mxc_iomux_set_input(MUX_IN_UART3_UART_RXD_MUX, INPUT_CTL_PATH2);

	gd->bd->bi_arch_number = MACH_TYPE_TQMA35;	/* board id for linux */
	gd->bd->bi_boot_params = 0x80000100;	/* address of boot parameters */

#if defined(CONFIG_TQMA35_MBA35CA)
	/* LCD_RST */
	mx35_gpio_set(69, 1);
#endif
	return 0;
}

#ifdef BOARD_LATE_INIT
static inline int board_detect(void)
{
	u8 buf[4];
	int id;

	if (i2c_read(0x08, 0x7, 1, buf, 3) < 0) {
		printf("board_late_init: read PMIC@0x08:0x7 fail\n");
		return 0;
	}
	id = (buf[0] << 16) + (buf[1] << 8) + buf[2];
	printf("PMIC@0x08:0x7 is %x\n", id);
	id = (id >> 6) & 0x7;
	if (id == 0x7) {
		set_board_rev(1);
		return 1;
	}
	set_board_rev(0);
	return 0;
}

int board_late_init(void)
{
	u8 reg[3];
	int i;

	if (board_detect()) {
		mxc_request_iomux(MX35_PIN_WATCHDOG_RST, MUX_CONFIG_SION |
					MUX_CONFIG_ALT1);
		printf("i.MX35 CPU board version 2.0\n");
		if (i2c_read(0x08, 0x1E, 1, reg, 3)) {
			printf("board_late_init: read PMIC@0x08:0x1E fail\n");
			return 0;
		}
		reg[2] |= 0x3;
		if (i2c_write(0x08, 0x1E, 1, reg, 3)) {
			printf("board_late_init: write PMIC@0x08:0x1E fail\n");
			return 0;
		}
		if (i2c_read(0x08, 0x20, 1, reg, 3)) {
			printf("board_late_init: read PMIC@0x08:0x20 fail\n");
			return 0;
		}
		reg[2] |= 0x1;
		if (i2c_write(0x08, 0x20, 1, reg, 3)) {
			printf("board_late_init: write PMIC@0x08:0x20 fail\n");
			return 0;
		}
		mxc_request_iomux(MX35_PIN_COMPARE, MUX_CONFIG_GPIO);
		mxc_iomux_set_input(MUX_IN_GPIO1_IN_5, INPUT_CTL_PATH0);
		__REG(GPIO1_BASE_ADDR + 0x04) |= 1 << 5;
		__REG(GPIO1_BASE_ADDR) |= 1 << 5;
	} else
		printf("i.MX35 CPU board version 1.0\n");

	if (i2c_read(0x69, 0x20, 1, reg, 1) < 0) {
		printf("board_late_init: read PMIC@0x69:0x20 fail\n");
		return 0;
	}

	reg[0] |= 0x4;
	if (i2c_write(0x69, 0x20, 1, reg, 1) < 0) {
		printf("board_late_init: write back PMIC@0x69:0x20 fail\n");
		return 0;
	}

	for (i = 0; i < 1000; i++)
		udelay(200);

	if (i2c_read(0x69, 0x1A, 1, reg, 1) < 0) {
		printf("board_late_init: read PMIC@0x69:0x1A fail\n");
		return 0;
	}

	reg[0] &= 0x7F;
	if (i2c_write(0x69, 0x1A, 1, reg, 1) < 0) {
		printf("board_late_init: write back PMIC@0x69:0x1A fail\n");
		return 0;
	}
	for (i = 0; i < 1000; i++)
		udelay(200);

	reg[0] |= 0x80;
	if (i2c_write(0x69, 0x1A, 1, reg, 1) < 0) {
		printf("board_late_init: 2st write back PMIC@0x69:0x1A fail\n");
		return 0;
	}

	return 0;
}
#endif

int checkboard(void)
{
#if defined(CONFIG_TQMA35_MBA35CA)
	printf("Board: TQMa35 / MBa35-CA ");
#else
	printf("Board: TQMa35 [");
#endif

	if (system_rev & CHIP_REV_2_0)
		printf("2.0 [");
	else
		printf("1.0 [");

	switch (__REG(CCM_BASE_ADDR + CLKCTL_RCSR) & 0x0F) {
	case 0x0000:
		printf("POR");
		break;
	case 0x0002:
		printf("JTAG");
		break;
	case 0x0004:
		printf("RST");
		break;
	case 0x0008:
		printf("WDT");
		break;
	default:
		printf("unknown");
	}
	printf("]\n");
	return 0;
}

#if defined(CONFIG_SMC911X)
extern int smc911x_initialize(u8 dev_num, int base_addr);
#endif

int board_eth_init(bd_t *bis)
{
	int rc = -ENODEV;
	rc = cpu_eth_init(bis);

#if defined(CONFIG_SMC911X)
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif

	return rc;
}
