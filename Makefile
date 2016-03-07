#########################################
#
# in tree GLD Driver
#
#########################################

TARGET		:= 	sfxge

SFXGE_SRCS	:= 	sfxge_err.c \
			sfxge_ev.c \
			sfxge_hash.c \
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
			mcdi_mon.c \
			siena_mac.c \
			siena_mcdi.c \
			siena_nic.c \
			siena_nvram.c \
			siena_phy.c \
			siena_sram.c \
			siena_vpd.c \
			ef10_ev.c \
			ef10_filter.c \
			ef10_intr.c \
			ef10_mac.c \
			ef10_mcdi.c \
			ef10_nic.c \
			ef10_nvram.c \
			ef10_phy.c \
			ef10_rx.c \
			ef10_tx.c \
			ef10_vpd.c \
			hunt_nic.c \
			hunt_phy.c \
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
			siena_flash.h \
			siena_impl.h \
			hunt_impl.h \
			ef10_impl.h \
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

CC=/opt/gcc-4.4.4/bin/gcc
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
