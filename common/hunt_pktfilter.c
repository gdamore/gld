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

#include "efsys.h"
#include "efx.h"
#include "efx_types.h"
#include "efx_impl.h"

#if EFSYS_OPT_HUNTINGTON

static	__checkReturn			int
hunt_pktfilter_apply(efx_nic_t *enp)
{
	int rc;

	EFSYS_ASSERT(enp->en_family == EFX_FAMILY_HUNTINGTON);

	/* FIXME */
	_NOTE(ARGUNUSED(enp))
	if (B_FALSE) {
		rc = ENOTSUP;
		goto fail1;
	}
	/* FIXME */

	return (0);

fail1:
	EFSYS_PROBE1(fail1, int, rc);

	return (rc);
}

	__checkReturn			int
hunt_pktfilter_set(
	__in				efx_nic_t *enp,
	__in				boolean_t unicst,
	__in				boolean_t brdcst)
{
	efx_port_t *epp = &(enp->en_port);
	efx_mac_ops_t *emop = epp->ep_emop;
	boolean_t old_unicst;
	boolean_t old_brdcst;
	int rc;

	EFSYS_ASSERT3U(enp->en_magic, ==, EFX_NIC_MAGIC);
	EFSYS_ASSERT3U(enp->en_mod_flags, &, EFX_MOD_PORT);

	old_unicst = unicst;
	old_brdcst = brdcst;

	epp->ep_all_unicst = unicst;
	epp->ep_brdcst = brdcst;

	if ((rc = emop->emo_reconfigure(enp)) != 0)
		goto fail1;

	if (enp->en_mod_flags & EFX_MOD_FILTER) {
		if ((rc = hunt_pktfilter_apply(enp)) != 0)
			goto fail2;
	}

	return (0);

fail2:
	EFSYS_PROBE(fail2);
fail1:
	EFSYS_PROBE1(fail1, int, rc);

	epp->ep_all_unicst = old_unicst;
	epp->ep_brdcst = old_brdcst;

	return (rc);
}

#if EFSYS_OPT_MCAST_FILTER_LIST

	__checkReturn			int
hunt_pktfilter_mcast_set(
	__in				efx_nic_t *enp,
	__in_ecount(6*count)		uint8_t const *addrs,
	__in				int count)
{
	int rc;

	EFSYS_ASSERT(enp->en_family == EFX_FAMILY_HUNTINGTON);

	/* FIXME */
	_NOTE(ARGUNUSED(enp, addrs, count))
	if (B_FALSE) {
		rc = ENOTSUP;
		goto fail1;
	}
	/* FIXME */

	return (0);

fail1:
	EFSYS_PROBE1(fail1, int, rc);

	return (rc);
}

#endif /* EFSYS_OPT_MCAST_FILTER_LIST */

	__checkReturn			int
hunt_pktfilter_mcast_all(
	__in				efx_nic_t *enp)
{
	int rc;

	EFSYS_ASSERT(enp->en_family == EFX_FAMILY_HUNTINGTON);

	/* FIXME */
	_NOTE(ARGUNUSED(enp))
	if (B_FALSE) {
		rc = ENOTSUP;
		goto fail1;
	}
	/* FIXME */

	return (0);

fail1:
	EFSYS_PROBE1(fail1, int, rc);

	return (rc);
}

#endif /* EFSYS_OPT_HUNTINGTON */
