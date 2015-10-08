/*
 * Copyright (c) 2013-2015 Solarflare Communications Inc.
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

#if EFSYS_OPT_FALCON || EFSYS_OPT_SIENA
extern	__checkReturn			efx_rc_t
falconsiena_pktfilter_set(
	__in				efx_nic_t *enp,
	__in				boolean_t unicst,
	__in				boolean_t brdcst);
#if EFSYS_OPT_MCAST_FILTER_LIST
extern	__checkReturn			efx_rc_t
falconsiena_pktfilter_mcast_list_set(
	__in				efx_nic_t *enp,
	__in				uint8_t const *addrs,
	__in				int count);
#endif /* EFSYS_OPT_MCAST_FILTER_LIST */
extern	__checkReturn			efx_rc_t
falconsiena_pktfilter_mcast_all(
	__in				efx_nic_t *enp);
#endif /* EFSYS_OPT_FALCON || EFSYS_OPT_SIENA */

#if EFSYS_OPT_FALCON
static efx_pktfilter_ops_t	__efx_pktfilter_falcon_ops = {
	falconsiena_pktfilter_set,		/* epfo_set */
#if EFSYS_OPT_MCAST_FILTER_LIST
	falconsiena_pktfilter_mcast_list_set,	/* epfo_mcast_list_set */
#endif /* EFSYS_OPT_MCAST_FILTER_LIST */
	falconsiena_pktfilter_mcast_all,	/* epfo_mcast_all */
};
#endif /* EFSYS_OPT_FALCON */

#if EFSYS_OPT_SIENA
static efx_pktfilter_ops_t	__efx_pktfilter_siena_ops = {
	falconsiena_pktfilter_set,		/* epfo_set */
#if EFSYS_OPT_MCAST_FILTER_LIST
	falconsiena_pktfilter_mcast_list_set,	/* epfo_mcast_list_set */
#endif /* EFSYS_OPT_MCAST_FILTER_LIST */
	falconsiena_pktfilter_mcast_all,	/* epfo_mcast_all */
};
#endif /* EFSYS_OPT_SIENA */

#if EFSYS_OPT_HUNTINGTON
static efx_pktfilter_ops_t	__efx_pktfilter_hunt_ops = {
	hunt_pktfilter_set,		/* epfo_set */
#if EFSYS_OPT_MCAST_FILTER_LIST
	hunt_pktfilter_mcast_set,	/* epfo_mcast_set */
#endif /* EFSYS_OPT_MCAST_FILTER_LIST */
	hunt_pktfilter_mcast_all	/* epfo_mcast_all */
};
#endif /* EFSYS_OPT_HUNTINGTON */

	__checkReturn			efx_rc_t
efx_pktfilter_init(
	__in				efx_nic_t *enp)
{
	efx_rc_t rc;

	EFSYS_ASSERT3U(enp->en_magic, ==, EFX_NIC_MAGIC);
	EFSYS_ASSERT3U(enp->en_mod_flags, &, EFX_MOD_PROBE);
	EFSYS_ASSERT(!(enp->en_mod_flags & EFX_MOD_PKTFILTER));

	enp->en_mod_flags |= EFX_MOD_PKTFILTER;

	switch (enp->en_family) {
#if EFSYS_OPT_FALCON
	case EFX_FAMILY_FALCON:
		enp->en_epfop =
		    (efx_pktfilter_ops_t *)&__efx_pktfilter_falcon_ops;
		break;
#endif /* EFSYS_OPT_FALCON */

#if EFSYS_OPT_SIENA
	case EFX_FAMILY_SIENA:
		enp->en_epfop =
		    (efx_pktfilter_ops_t *)&__efx_pktfilter_siena_ops;
		break;
#endif /* EFSYS_OPT_SIENA */

#if EFSYS_OPT_HUNTINGTON
	case EFX_FAMILY_HUNTINGTON:
		enp->en_epfop =
		    (efx_pktfilter_ops_t *)&__efx_pktfilter_hunt_ops;
		break;
#endif /* EFSYS_OPT_HUNTINGTON */

	default:
		EFSYS_ASSERT(0);
		rc = ENOTSUP;
		goto fail1;
	}

	return (0);
fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	enp->en_epfop = NULL;
	enp->en_mod_flags &= ~EFX_MOD_PKTFILTER;
	return (rc);
}

					void
efx_pktfilter_fini(
	__in				efx_nic_t *enp)
{
	enp->en_epfop = NULL;
	enp->en_mod_flags &= ~EFX_MOD_PKTFILTER;
}

	__checkReturn			efx_rc_t
efx_pktfilter_set(
	__in				efx_nic_t *enp,
	__in				boolean_t unicst,
	__in				boolean_t brdcst)
{
	efx_pktfilter_ops_t *epfop = enp->en_epfop;
	efx_rc_t rc;

	EFSYS_ASSERT3U(enp->en_mod_flags, &, EFX_MOD_PKTFILTER);

	if ((rc = epfop->epfo_set(enp, unicst, brdcst)) != 0)
		goto fail1;

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

#if EFSYS_OPT_MCAST_FILTER_LIST

	__checkReturn			efx_rc_t
efx_pktfilter_mcast_list_set(
	__in				efx_nic_t *enp,
	__in_ecount(6*count)		uint8_t const *addrs,
	__in				int count)
{
	efx_pktfilter_ops_t *epfop = enp->en_epfop;
	efx_rc_t rc;

	EFSYS_ASSERT3U(enp->en_mod_flags, &, EFX_MOD_PKTFILTER);

	if ((rc = epfop->epfo_mcast_list_set(enp, addrs, count)) != 0)
		goto fail1;

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

#endif /* EFSYS_OPT_MCAST_FILTER_LIST */

	__checkReturn			efx_rc_t
efx_pktfilter_mcast_all(
	__in				efx_nic_t *enp)
{
	efx_pktfilter_ops_t *epfop = enp->en_epfop;
	efx_rc_t rc;

	EFSYS_ASSERT3U(enp->en_mod_flags, &, EFX_MOD_PKTFILTER);

	if ((rc = epfop->epfo_mcast_all(enp)) != 0)
		goto fail1;

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

#if EFSYS_OPT_FALCON || EFSYS_OPT_SIENA

	__checkReturn			efx_rc_t
falconsiena_pktfilter_set(
	__in				efx_nic_t *enp,
	__in				boolean_t unicst,
	__in				boolean_t brdcst)
{
	efx_port_t *epp = &(enp->en_port);
	efx_mac_ops_t *emop = epp->ep_emop;
	boolean_t old_unicst;
	boolean_t old_brdcst;
	efx_rc_t rc;

	EFSYS_ASSERT3U(enp->en_magic, ==, EFX_NIC_MAGIC);
	EFSYS_ASSERT3U(enp->en_mod_flags, &, EFX_MOD_PORT);

	old_unicst = unicst;
	old_brdcst = brdcst;

	epp->ep_all_unicst = unicst;
	epp->ep_brdcst = brdcst;

	if ((rc = emop->emo_reconfigure(enp)) != 0)
		goto fail1;

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	epp->ep_all_unicst = old_unicst;
	epp->ep_brdcst = old_brdcst;

	return (rc);
}

#if EFSYS_OPT_MCAST_FILTER_LIST

	__checkReturn			efx_rc_t
falconsiena_pktfilter_mcast_list_set(
	__in				efx_nic_t *enp,
	__in_ecount(6*count)		uint8_t const *addrs,
	__in				int count)
{
	unsigned int bucket[EFX_MAC_HASH_BITS];
	int i;
	efx_rc_t rc;

	for (i = 0; i < count; i++) {
		/* Calculate hash bucket (IEEE 802.3 CRC32 of the MAC addr) */
		uint32_t crc = efx_crc32_calculate(0xffffffff, addrs, 6);
		bucket[crc & 0xff] = 1;
		addrs += 6;
	}

	if ((rc = efx_mac_hash_set(enp, bucket)) != 0)
		goto fail1;

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

#endif /* EFSYS_OPT_MCAST_FILTER_LIST */

	__checkReturn			efx_rc_t
falconsiena_pktfilter_mcast_all(
	__in				efx_nic_t *enp)
{
	unsigned int bucket[EFX_MAC_HASH_BITS];
	int index;
	efx_rc_t rc;

	for (index = 0; index < EFX_MAC_HASH_BITS; index++) {
		bucket[index] = 1;
	}

	if ((rc = efx_mac_hash_set(enp, bucket)) != 0)
		goto fail1;

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

#endif /* EFSYS_OPT_FALCON || EFSYS_OPT_SIENA */
