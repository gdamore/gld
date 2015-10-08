/*
 * Copyright (c) 2007-2015 Solarflare Communications Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of the FreeBSD Project.
 */

#ifndef	_SYS_SFX7101_IMPL_H
#define	_SYS_SFX7101_IMPL_H

#ifdef	__cplusplus
extern "C" {
#endif

#if EFSYS_OPT_PHY_SFX7101

#define	SFX7101_MMD_MASK						\
	    ((1 << PMA_PMD_MMD) |					\
	    (1 << PCS_MMD) |						\
	    (1 << PHY_XS_MMD) |						\
	    (1 << AN_MMD))

/* IO expender */
#define	PCA9539 0x74

#define	P0_IN 0x00
#define	P0_OUT 0x02
#define	P0_INVERT 0x04
#define	P0_CONFIG 0x06

#define	P0_EN_1V0X_LBN 0
#define	P0_EN_1V0X_WIDTH 1
#define	P0_EN_1V2_LBN 1
#define	P0_EN_1V2_WIDTH 1
#define	P0_EN_2V5_LBN 2
#define	P0_EN_2V5_WIDTH 1
#define	P0_FLASH_CFG_EN_LBN 3
#define	P0_FLASH_CFG_EN_WIDTH 1
#define	P0_EN_5V_LBN 4
#define	P0_EN_5V_WIDTH 1
#define	P0_SHORTEN_JTAG_LBN 5
#define	P0_SHORTEN_JTAG_WIDTH 1
#define	P0_X_TRST_LBN 6
#define	P0_X_TRST_WIDTH 1
#define	P0_DSP_RESET_LBN 7
#define	P0_DSP_RESET_WIDTH 1

#define	P1_IN 0x01
#define	P1_OUT 0x03
#define	P1_INVERT 0x05
#define	P1_CONFIG 0x07

#define	P1_AFE_PWD_LBN 0
#define	P1_AFE_PWD_WIDTH 1
#define	P1_DSP_PWD25_LBN 1
#define	P1_DSP_PWD25_WIDTH 1
#define	P1_RESERVED_LBN 2
#define	P1_RESERVED_WIDTH 2
#define	P1_SPARE_LBN 4
#define	P1_SPARE_WIDTH 4

#define	PMA_PMD_XCONTROL_REG 0xc000
#define	SSR_LBN 15
#define	SSR_WIDTH 1
#define	LNPGA_POWERDOWN_LBN 8
#define	LNPGA_POWERDOWN_WIDTH 1
#define	AFE_POWERDOWN_LBN 9
#define	AFE_POWERDOWN_WIDTH 1
#define	DSP_POWERDOWN_LBN 10
#define	DSP_POWERDOWN_WIDTH 1
#define	PHY_POWERDOWN_LBN 11
#define	PHY_POWERDOWN_WIDTH 1

#define	PMA_PMD_XSTATUS_REG 0xc001
#define	FLP_RCVD_LBN 12
#define	FLP_RCVD_WIDTH 1

#define	PMA_PMD_LED_CONTROL_REG 0xc007
#define	LED_ACTIVITY_EN_LBN 3
#define	LED_ACTIVITY_EN_WIDTH 1
#define	LED_FLASH_PERIOD_LBN 0
#define	LED_FLASH_PERIOD_WIDTH 3

#define	PMA_PMD_LED_OVERRIDE_REG 0xc009
#define	LED_LINK_LBN 0 /* Green */
#define	LED_LINK_WIDTH 2
#define	LED_TX_LBN 4 /* Amber */
#define	LED_TX_WIDTH 2
#define	LED_RX_LBN 6 /* Red */
#define	LED_RX_WIDTH 2
#define	LED_NORMAL_DECODE 0x0
#define	LED_ON_DECODE 0x1
#define	LED_OFF_DECODE 0x2
#define	LED_FLASH_DECODE 0x3

#define	PMA_PMD_FW_REV0_REG 0xc026
#define	PMA_PMD_FW_REV1_REG 0xc027

#define	PCS_BOOT_STATUS_REG 0xd000	/* PRM 10.4.1 */
#define	RESET_CAUSE_LBN 8
#define	RESET_CAUSE_WIDTH 2
#define	HW_RESET_DECODE 0x0
#define	SW_RESET_DECODE 0x1
#define	WD_RESET_DECODE 0x2
#define	SW_WD_RESET_DECODE 0x3
#define	UPLOAD_PROGRESS_LBN 7
#define	UPLOAD_PROGRESS_WIDTH 1
#define	CODE_DOWNLOAD_LBN 6
#define	CODE_DOWNLOAD_WIDTH 1
#define	CKSUM_OK_LBN 5
#define	CKSUM_OK_WIDTH 1
#define	CODE_STARTED_LBN 4
#define	CODE_STARTED_WIDTH 1
#define	BOOT_STATUS_LBN 3
#define	BOOT_STATUS_WIDTH 1
#define	BOOT_PROGRESS_LBN 1
#define	BOOT_PROGRESS_WIDTH 2
#define	INIT_DECODE 0x0
#define	MDIO_WAIT_DECODE 0x1
#define	CKSUM_START_DECODE 0x2
#define	APP_JMP_DECODE 0x3
#define	FATAL_ERR_LBN 0
#define	FATAL_ERR_WIDTH 1

#define	PCS_LM_RAM_LS_ADDR_REG 0xd004
#define	LM_RAM_LS_ADDR_LBN 0
#define	LM_RAM_LS_ADDR_WIDTH 16

#define	PCS_LM_RAM_MS_ADDR_REG 0xd005
#define	LM_RAM_MS_ADDR_LBN 0
#define	LM_RAM_MS_ADDR_WIDTH 3
#define	BYTE_ACCESS_LBN 15
#define	BYTE_ACCESS_WIDTH 1

#define	PCS_LM_RAM_DATA_REG 0xd006
#define	LM_RAM_DATA_LBN 0
#define	LM_RAM_DATA_WIDTH 16

#define	PCS_TEST_SELECT_REG 0xd807	/* PRM 10.5.8 */
#define	CLK312_EN_LBN 3
#define	CLK312_EN_WIDTH 1

#define	PCS_CLOCK_CTRL_REG 0xd801
#define	PLL312_RST_N_LBN 2
#define	PLL312_RST_N_WIDTH 1

#define	PCS_SOFT_RST2_REG 0xd806
#define	SERDES_RST_N_LBN 13
#define	SERDES_RST_N_WIDTH 1
#define	XGXS_RST_N_LBN 12
#define	XGXS_RST_N_WIDTH 1

#define	PHY_XS_XGXS_TEST_REG 0xc00a
#define	SERDES_LOOPBACK_LBN 9
#define	SERDES_LOOPBACK_WIDTH 1
#define	NE_LOOPBACK_LBN 8
#define	NE_LOOPBACK_WIDTH 1

#define	LOADER_MMD 1
#define	LOADER_MAX_BUFF_SZ_REG 49192
#define	LOADER_ACTUAL_BUFF_SZ_REG 49193
#define	LOADER_CMD_RESPONSE_REG 49194
#define	LOADER_CMD_ERASE_FLASH 0x0001
#define	LOADER_CMD_FILL_BUFFER 0x0002
#define	LOADER_CMD_PROGRAM_FLASH 0x0003
#define	LOADER_CMD_READ_FLASH 0x0004
#define	LOADER_RESPONSE_OK 0x0100
#define	LOADER_RESPONSE_ERROR 0x0200
#define	LOADER_RESPONSE_BUSY 0x0300
#define	LOADER_WORDS_WRITTEN_REG 49195
#define	LOADER_WORDS_READ_REG 49195
#define	LOADER_FLASH_ADDR_LOW_REG 49196
#define	LOADER_FLASH_ADDR_HI_REG  49197
#define	LOADER_DATA_REG 49198

#define	FIRMWARE_BLOCK_SIZE 0x4000
#define	FIRMWARE_MAX_SIZE 0x30000

#endif	/* EFSYS_OPT_PHY_SFX7101 */

#ifdef	__cplusplus
}
#endif

#endif	/* _SYS_SFX7101_IMPL_H */
