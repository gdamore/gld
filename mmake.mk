#########################################
#
# in tree GLD Driver
#
#########################################

TARGET		:= 	sfxge

SFXGE_SRCS	:= 	sfxge_err.c \
			sfxge_ev.c \
			sfxge_intr.c \
			sfxge_mac.c \
			sfxge_gld_ndd.c \
			sfxge_gld_v3.c \
			sfxge_mon.c \
			sfxge_phy.c \
			sfxge_sram.c \
			sfxge_bar.c \
			sfxge_pci.c \
			sfxge_nvram.c \
			sfxge_rx.c \
			sfxge_tcp.c \
			sfxge_tx.c \
			sfxge_mcdi.c \
			sfxge_vpd.c \
			sfxge.c \
			sfxge_dma.c
SFXGE_OBJS	:=	$(SFXGE_SRCS:%.c=%.o)

SFXGE_HDRS	:= 	sfxge.h \
			sfxge_debug.h \
			sfxge_ioc.h \
			efsys.h
SFXGE_HSRCS	:=	$(SFXGE_HDRS:%.h=%_h.c)
SFXGE_HOBJS	:=	$(SFXGE_HDRS:%.h=%_h.o)

COMMON_SRCS	:= 	efx_bootcfg.c \
			efx_crc32.c \
			efx_ev.c \
			efx_filter.c \
			efx_hash.c \
			efx_intr.c \
			efx_mac.c \
			efx_mcdi.c \
			efx_mon.c \
			efx_nic.c \
			efx_nvram.c \
			efx_phy.c \
			efx_port.c \
			efx_rx.c \
			efx_sram.c \
			efx_tx.c \
			efx_vpd.c \
			efx_wol.c \
			falcon_gmac.c \
			falcon_i2c.c \
			falcon_mac.c \
			falcon_mdio.c \
			falcon_nic.c \
			falcon_nvram.c \
			falcon_spi.c \
			falcon_sram.c \
			falcon_vpd.c \
			falcon_xmac.c \
			lm87.c \
			max6647.c \
			nullmon.c \
			nullphy.c \
			qt2022c2.c \
			qt2025c.c \
			sft9001.c \
			sfx7101.c \
			txc43128.c \
			xphy.c \
			siena_mac.c \
			siena_mcdi.c \
			mcdi_mon.c \
			siena_nic.c \
			siena_nvram.c \
			siena_phy.c \
			siena_sram.c \
			siena_vpd.c \
			hunt_ev.c \
			hunt_filter.c \
			hunt_intr.c \
			hunt_mac.c \
			hunt_mcdi.c \
			hunt_nic.c \
			hunt_nvram.c \
			hunt_phy.c \
			hunt_rx.c \
			hunt_sram.c \
			hunt_tx.c \
			hunt_vpd.c
COMMON_OBJS	:=	$(COMMON_SRCS:%.c=%.o)

COMMON_HDRS	:= 	efx.h \
			efx_impl.h \
			efx_mcdi.h \
			efx_pcie.h \
			efx_regs.h \
			efx_regs_ef10.h \
			efx_regs_mcdi.h \
			efx_regs_pci.h \
			efx_types.h \
			falcon_gmac.h \
			falcon_impl.h \
			falcon_nvram.h \
			falcon_stats.h \
			falcon_xmac.h \
			lm87.h \
			lm87_impl.h \
			max6647.h \
			max6647_impl.h \
			nullmon.h \
			nullphy.h \
			nullphy_impl.h \
			qt2022c2.h \
			qt2022c2_impl.h \
			qt2025c.h \
			qt2025c_impl.h \
			sft9001.h \
			sft9001_impl.h \
			sfx7101.h \
			sfx7101_impl.h \
			txc43128.h \
			txc43128_impl.h \
			xphy.h \
			siena_flash.h \
			siena_impl.h \
			hunt_impl.h \
			efx_phy_ids.h \
			mcdi_mon.h

IMPORT		:= 	$(COMMON_SRCS:%=../common/%) $(COMMON_HDRS:%=../common/%)

ifndef MMAKE_NO_RULES
MMAKE_INCLUDE	:=

# Solaris 10 9/10 (U9) is required to build this driver, as earlier Solaris
# releases do not ship with the required header files for GLDv3.
# The driver will run on Solaris 10 10/09 (U8) and later versions.
OS_SUPPORTED:=$(shell ggrep -Eq  'Solaris 11|s10x_u(9|10)|OmniOS v11' /etc/release && echo YES)
ifeq ($(OS_SUPPORTED),)
      $(error This driver needs to be built on sol10u9 or later. \
	To build a driver to run on sol10u8 use sol10u9)
endif

ifeq ($(GLDV),3_sol11)
DEPEND		:= -N misc/mac -N drv/ip
MMAKE_CFLAGS    += -D_USE_GLD_V3
MMAKE_CFLAGS    += -D_USE_GLD_V3_SOL10
MMAKE_CFLAGS    += -D_USE_GLD_V3_SOL11
MMAKE_CFLAGS	+= -D_USE_MAC_PRIV_PROP
MMAKE_CFLAGS    += -D_USE_GLD_V3_PROPS
MMAKE_CFLAGS    += -U_USE_MTU_UPDATE
MMAKE_CFLAGS    += -D_USE_NDD_PROPS
else


ifeq ($(GLDV),3_sol10)
DEPEND		:= -N misc/mac -N drv/ip
MMAKE_CFLAGS    += -D_USE_GLD_V3
MMAKE_CFLAGS    += -D_USE_GLD_V3_SOL10
MMAKE_CFLAGS	+= -U_USE_MAC_PRIV_PROP
MMAKE_CFLAGS    += -U_USE_GLD_V3_PROPS
MMAKE_CFLAGS    += -U_USE_MTU_UPDATE
MMAKE_CFLAGS    += -D_USE_NDD_PROPS
# http://intranet.solarflarecom.com/staffwiki/SolarisDevel
# There is no CTF data that is correct for Solaris 10
export CTFCONVERT_PERMISSIVE=1
else

$(error "Please set GLDV=3_sol10|3_sol11")
endif
endif

MMAKE_INCLUDE	+= -I../../../../src/include -I/usr/include -include "compat.h"

all:	$(TARGET)

clean:
	@$(MakeClean)
	@rm -f *.c

%_h.c: %.h
	@cp $< $@

$(TARGET): $(COMMON_HOBJS) $(SFXGE_HOBJS) $(COMMON_OBJS) $(SFXGE_OBJS)
	$(LD) -r -dy $(DEPEND) $(COMMON_OBJS) $(SFXGE_OBJS) -o $@
	onbld ctfmerge -l v5 -o $@ $(COMMON_OBJS) $(SFXGE_OBJS)
	$(call DO_COPY_TARGET,$@)

endif
