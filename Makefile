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

DEPEND		:= -N misc/mac -N drv/ip
CPPFLAGS	+= -D_KERNEL
CPPFLAGS	+= -D_USE_GLD_V3
CPPFLAGS	+= -D_USE_GLD_V3_SOL10
CPPFLAGS	+= -D_USE_GLD_V3_SOL11
CPPFLAGS	+= -D_USE_MAC_PRIV_PROP
CPPFLAGS	+= -D_USE_GLD_V3_PROPS
CPPFLAGS	+= -U_USE_MTU_UPDATE
CPPFLAGS	+= -D_USE_NDD_PROPS

CC=/opt/gcc/4.4.4/bin/gcc
#CFLAGS += -m64 -xmodel=kernel -xstrconst -xdebugformat=stabs -v
CFLAGS += -ffreestanding -mcmodel=kernel -mno-red-zone
CFLAGS += -m64 -finline -nodefaultlibs -fno-builtin -mno-mmx -mno-sse -msave-args -Wall
CPPFLAGS	+= -I. -Icommon -I../../../../src/include -I/usr/include -include "compat.h"

# we need to tweak this
CTFMERGE	= : /opt/onbld/bin/`uname -p`/ctfmerge

all:	$(TARGET)

clean:
	@$(MakeClean)
	@rm -f *.c

%_h.c: %.h
	@cp $< $@

%.o: common/%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

$(TARGET): $(COMMON_HOBJS) $(SFXGE_HOBJS) $(COMMON_OBJS) $(SFXGE_OBJS)
	$(LD) -r -dy $(DEPEND) $(COMMON_OBJS) $(SFXGE_OBJS) -o $@
	$(CTFMERGE) -l v5 -o $@ $(COMMON_OBJS) $(SFXGE_OBJS)
	$(call DO_COPY_TARGET,$@)
