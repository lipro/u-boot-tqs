/*
 * Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
 *
 * (C) Copyright 2008-2009 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2010
 * Daniel Gericke, TQ-Systems GmBH, daniel.gericke@tqs.de
 *
 * Configuration settings for the TQMa35 board.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/mx35.h>

 /* High Level Configuration Options */
#define CONFIG_ARM1136		1	/* This is an arm1136 CPU core */
#define CONFIG_MXC		1
#define CONFIG_MX35		1	/* in a mx31 */
#define CONFIG_MX35_HCLK_FREQ	24000000	/* RedBoot says 26MHz */
#define CONFIG_MX35_CLK32	32768

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

/*
 * Disabled for now due to build problems under Debian and a significant increase
 * in the final file size: 144260 vs. 109536 Bytes.
 */
#if 0
#define CONFIG_OF_LIBFDT		1
#define CONFIG_FIT			1
#define CONFIG_FIT_VERBOSE		1
#endif

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_REVISION_TAG		1
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 512 * 1024)
#define CONFIG_SYS_GBL_DATA_SIZE	128/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */
#define CONFIG_HARD_I2C		1
#define CONFIG_I2C_MXC		1
#define CONFIG_SYS_I2C_PORT		I2C2_BASE_ADDR
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		0xfe

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_MXC_UART         1
#define CONFIG_SYS_MX35_UART3   1
#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{9600, 19200, 38400, 57600, 115200}

/***********************************************************
 * Command definition
 ***********************************************************/

#include <config_cmd_default.h>

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP

#define CONFIG_CMD_DATE
#define CONFIG_CMD_DTT
#define CONFIG_CMD_EEPROM

#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_MMC

/*
 * MMC Configs
 * */
#ifdef CONFIG_CMD_MMC
	#define CONFIG_MMC				1
	#define CONFIG_GENERIC_MMC
	#define CONFIG_IMX_MMC
	#define CONFIG_DOS_PARTITION	1
	#define CONFIG_CMD_FAT		1
#endif

#define CONFIG_BOOTDELAY	3

#define CONFIG_LOADADDR		0x80800000	/* loadaddr env var */

#define	CONFIG_EXTRA_ENV_SETTINGS					\
		"hostname=tqma35\0"					\
		"netdev=eth0\0"						\
		"console=ttymxc2\0"					\
		"uboot_addr=0xa0000000\0"				\
		"kernel_addr=0xa0080000\0"				\
		"ramdisk_addr=0xa0300000\0"				\
		"uboot=u-boot.bin\0"					\
		"kernel=uImage\0"					\
		"rootpath=/opt/ltib/rootfs\0"				\
		"nfsargs=setenv bootargs root=/dev/nfs rw "		\
			"nfsroot=${serverip}:${rootpath},v3,tcp\0"	\
		"jffsargs=setenv bootargs root=/dev/mtdblock3 rw "	\
			"rootfstype=jffs2\0"				\
		"addip=setenv bootargs ${bootargs} "			\
			"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"\
			":${hostname}:${netdev}:off panic=1\0"		\
		"addcons=setenv bootargs ${bootargs} "			\
			"console=${console},${baudrate}\0"		\
		"bootargs_base=setenv bootargs console=ttymxc2,115200\0"\
		"bootargs_nfs=setenv bootargs ${bootargs} root=/dev/nfs "\
			"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0"\
		"bootcmd=run net_nfs\0"					\
		"flash_nfs=run nfsargs addip addcons;"			\
			"bootm ${kernel_addr}\0"			\
		"net_nfs=run nfsargs addip addcons; "			\
			"tftpboot ${loadaddr} ${kernel}; bootm\0"	\
		"flash_jffs=run jffsargs addip addcons;"		\
			"bootm ${kernel_addr}\0"			\
		"prg_uboot=tftpboot ${loadaddr} ${uboot}; "		\
			"protect off ${uboot_addr} 0xa003ffff; "	\
			"erase ${uboot_addr} 0xa003ffff; "		\
			"cp.b ${loadaddr} ${uboot_addr} ${filesize}; "	\
			"setenv filesize; saveenv\0"

/*
 * add version variable to env - enables inclusion of
 * version info in flash env and reading from linux
 */
#define CONFIG_VERSION_VARIABLE

/* EEPROM */
#define CONFIG_SYS_I2C_EEPROM_ADDR      0x50            /* EEPROM AT24C256      */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 2
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS       4
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS   10      /* and takes up to 10 msec */

/* I2C RTC */
#define CONFIG_RTC_DS1337               /* Use ds1337 rtc via i2c       */
#define CONFIG_SYS_I2C_RTC_ADDR 0x68    /* at address 0x68              */

/* I2C SYSMON (LM75) */
#define CONFIG_DTT_LM75         1               /* ON Semi's LM75       */
#define CONFIG_DTT_SENSORS      {48}             /* Sensor addresses     */
#define CONFIG_SYS_DTT_MAX_TEMP 70
#define CONFIG_SYS_DTT_LOW_TEMP -30
#define CONFIG_SYS_DTT_HYSTERESIS       3

/*Support LAN9217*/
#define CONFIG_SMC911X	1
#define CONFIG_SMC911X_16_BIT 1
#define CONFIG_SMC911X_BASE CS1_BASE_ADDR

/*
 * Ethernet
 */
#define CONFIG_HAS_ETH1
#define CONFIG_NET_MULTI 1
#define CONFIG_MXC_FEC
#define CONFIG_MII
#define CONFIG_DISCOVER_PHY

#define CONFIG_FEC0_IOBASE FEC_BASE_ADDR
#define CONFIG_FEC0_PHY_ADDR	0x01

/*
 * The MX31ADS board seems to have a hardware "peculiarity" confirmed under
 * U-Boot, RedBoot and Linux: the ethernet Rx signal is reaching the CS8900A
 * controller inverted. The controller is capable of detecting and correcting
 * this, but it needs 4 network packets for that. Which means, at startup, you
 * will not receive answers to the first 4 packest, unless there have been some
 * broadcasts on the network, or your board is on a hub. Reducing the ARP
 * timeout from default 5 seconds to 200ms we speed up the initial TFTP
 * transfer, should the user wish one, significantly.
 */
#define CONFIG_ARP_TIMEOUT	200UL

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP	/* undef to save memory */
#define CONFIG_SYS_PROMPT	"MX35 U-Boot > "
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_ARCH_MMU
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE	512	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE	512 /* Boot Argument Buffer Size */

#define CONFIG_SYS_MEMTEST_START	0	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x10000

#undef	CONFIG_SYS_CLKS_IN_HZ	/* everything, incl board info, in Hz */

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_SYS_HZ		CONFIG_MX35_CLK32/* use 32kHz clock as source */

#define CONFIG_CMDLINE_EDITING	1

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128 * 1024)	/* regular stack */

/*-----------------------------------------------------------------------
 * Physical Memory Map
 * TQMa35-AA = 128 MiB mDDR (default)
 * TQMa35-AB = 256 MiB mDDR
 */
#if defined (CONFIG_TQMA35_AA)
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM_1		CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE	(128 * 1024 * 1024)
#elif defined(CONFIG_TQMA35_AB)
#define CONFIG_NR_DRAM_BANKS	2
#define PHYS_SDRAM_1		CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE	(128 * 1024 * 1024)
#define PHYS_SDRAM_2		CSD1_BASE_ADDR
#define PHYS_SDRAM_2_SIZE	(128 * 1024 * 1024)
#else
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM_1		CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE	(128 * 1024 * 1024)
#endif

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_FLASH_BASE		CS0_BASE_ADDR
#define CONFIG_SYS_MAX_FLASH_BANKS 1	/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT 1024	/* max number of sectors on one chip */
#define CONFIG_SYS_FLASH_EMPTY_INFO	/* print 'E' for empty sectors */
/* Monitor at beginning of flash */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)	/* Reserve 256 KiB */

#define	CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SECT_SIZE	(128 * 1024)
#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE

/* Address and size of Redundant Environment Sector */
#define CONFIG_ENV_OFFSET_REDUND (CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE

#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)


/*-----------------------------------------------------------------------
 * CFI FLASH driver setup
 */
#define CONFIG_SYS_FLASH_CFI		1/* Flash memory is CFI compliant */
#define CONFIG_FLASH_CFI_DRIVER		1/* Use drivers/cfi_flash.c */
#define CONFIG_SYS_FLASH_CFI_AMD_RESET  /* Use AMD (Spansion) reset cmd */
#define CONFIG_SYS_CFI_FLASH_STATUS_POLL /* use status poll method      */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1/* Use buffered writes (~10x faster) */
#define CONFIG_SYS_FLASH_PROTECTION	1/* Use hardware sector protection */

/*
 * JFFS2 partitions
 */
#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_DEV	"nor0"

/* mtdparts command line support */
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define MTDIDS_DEFAULT		"nor0=nor0"
#define CONFIG_FLASH_CFI_MTD
#define MTDPARTS_DEFAULT	"mtdparts=nor0:256k(u-boot),256k(env),2560k(kernel),-(jffs2)"


#endif				/* __CONFIG_H */
