/*
 * Copyright (c) 2008-2015 Solarflare Communications Inc.
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
#include "efx_regs.h"
#include "efx_regs_mcdi.h"
#include "efx_impl.h"

#if EFSYS_OPT_MCDI


#if EFSYS_OPT_SIENA

static efx_mcdi_ops_t	__efx_mcdi_siena_ops = {
	siena_mcdi_init,		/* emco_init */
	siena_mcdi_request_copyin,	/* emco_request_copyin */
	siena_mcdi_request_poll,	/* emco_request_poll */
	siena_mcdi_request_copyout,	/* emco_request_copyout */
	siena_mcdi_poll_reboot,		/* emco_poll_reboot */
	siena_mcdi_fini,		/* emco_fini */
	siena_mcdi_fw_update_supported,	/* emco_fw_update_supported */
	siena_mcdi_macaddr_change_supported,
				/* emco_macaddr_change_supported */
	siena_mcdi_link_control_supported,
				/* emco_link_control_supported */
};

#endif	/* EFSYS_OPT_SIENA */

#if EFSYS_OPT_HUNTINGTON

static efx_mcdi_ops_t	__efx_mcdi_hunt_ops = {
	hunt_mcdi_init,			/* emco_init */
	hunt_mcdi_request_copyin,	/* emco_request_copyin */
	hunt_mcdi_request_poll,		/* emco_request_poll */
	hunt_mcdi_request_copyout,	/* emco_request_copyout */
	hunt_mcdi_poll_reboot,		/* emco_poll_reboot */
	hunt_mcdi_fini,			/* emco_fini */
	hunt_mcdi_fw_update_supported,	/* emco_fw_update_supported */
	hunt_mcdi_macaddr_change_supported,
				/* emco_macaddr_change_supported */
	hunt_mcdi_link_control_supported,
				/* emco_link_control_supported */
};

#endif	/* EFSYS_OPT_HUNTINGTON */



	__checkReturn	efx_rc_t
efx_mcdi_init(
	__in		efx_nic_t *enp,
	__in		const efx_mcdi_transport_t *emtp)
{
	efx_mcdi_ops_t *emcop;
	efx_rc_t rc;

	EFSYS_ASSERT3U(enp->en_magic, ==, EFX_NIC_MAGIC);
	EFSYS_ASSERT3U(enp->en_mod_flags, ==, 0);

	switch (enp->en_family) {
#if EFSYS_OPT_FALCON
	case EFX_FAMILY_FALCON:
		emcop = NULL;
		emtp = NULL;
		break;
#endif	/* EFSYS_OPT_FALCON */

#if EFSYS_OPT_SIENA
	case EFX_FAMILY_SIENA:
		emcop = (efx_mcdi_ops_t *)&__efx_mcdi_siena_ops;
		break;
#endif	/* EFSYS_OPT_SIENA */

#if EFSYS_OPT_HUNTINGTON
	case EFX_FAMILY_HUNTINGTON:
		emcop = (efx_mcdi_ops_t *)&__efx_mcdi_hunt_ops;
		break;
#endif	/* EFSYS_OPT_HUNTINGTON */

	default:
		EFSYS_ASSERT(0);
		rc = ENOTSUP;
		goto fail1;
	}

	if (enp->en_features & EFX_FEATURE_MCDI_DMA) {
		/* MCDI requires a DMA buffer in host memory */
		if ((emtp == NULL) || (emtp->emt_dma_mem) == NULL) {
			rc = EINVAL;
			goto fail2;
		}
	}
	enp->en_mcdi.em_emtp = emtp;

	if (emcop != NULL && emcop->emco_init != NULL) {
		if ((rc = emcop->emco_init(enp, emtp)) != 0)
			goto fail3;
	}

	enp->en_mcdi.em_emcop = emcop;
	enp->en_mod_flags |= EFX_MOD_MCDI;

	return (0);

fail3:
	EFSYS_PROBE(fail3);
fail2:
	EFSYS_PROBE(fail2);
fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	enp->en_mcdi.em_emcop = NULL;
	enp->en_mcdi.em_emtp = NULL;
	enp->en_mod_flags &= ~EFX_MOD_MCDI;

	return (rc);
}

			void
efx_mcdi_fini(
	__in		efx_nic_t *enp)
{
	efx_mcdi_iface_t *emip = &(enp->en_mcdi.em_emip);
	efx_mcdi_ops_t *emcop = enp->en_mcdi.em_emcop;

	EFSYS_ASSERT3U(enp->en_magic, ==, EFX_NIC_MAGIC);
	EFSYS_ASSERT3U(enp->en_mod_flags, ==, EFX_MOD_MCDI);

	if (emcop != NULL && emcop->emco_fini != NULL)
		emcop->emco_fini(enp);

	emip->emi_port = 0;
	emip->emi_aborted = 0;

	enp->en_mcdi.em_emcop = NULL;
	enp->en_mod_flags &= ~EFX_MOD_MCDI;
}

			void
efx_mcdi_new_epoch(
	__in		efx_nic_t *enp)
{
	efx_mcdi_iface_t *emip = &(enp->en_mcdi.em_emip);
	int state;

	/* Start a new epoch (allow fresh MCDI requests to succeed) */
	EFSYS_LOCK(enp->en_eslp, state);
	emip->emi_new_epoch = B_TRUE;
	EFSYS_UNLOCK(enp->en_eslp, state);
}


			void
efx_mcdi_request_start(
	__in		efx_nic_t *enp,
	__in		efx_mcdi_req_t *emrp,
	__in		boolean_t ev_cpl)
{
	efx_mcdi_iface_t *emip = &(enp->en_mcdi.em_emip);
	efx_mcdi_ops_t *emcop = enp->en_mcdi.em_emcop;
	unsigned int seq;
	boolean_t new_epoch;
	int state;

	EFSYS_ASSERT3U(enp->en_magic, ==, EFX_NIC_MAGIC);
	EFSYS_ASSERT3U(enp->en_mod_flags, &, EFX_MOD_MCDI);
	EFSYS_ASSERT3U(enp->en_features, &, EFX_FEATURE_MCDI);

	if (emcop == NULL || emcop->emco_request_copyin == NULL)
		return;

	/*
	 * efx_mcdi_request_start() is naturally serialised against both
	 * efx_mcdi_request_poll() and efx_mcdi_ev_cpl()/efx_mcdi_ev_death(),
	 * by virtue of there only being one outstanding MCDI request.
	 * Unfortunately, upper layers may also call efx_mcdi_request_abort()
	 * at any time, to timeout a pending mcdi request, That request may
	 * then subsequently complete, meaning efx_mcdi_ev_cpl() or
	 * efx_mcdi_ev_death() may end up running in parallel with
	 * efx_mcdi_request_start(). This race is handled by ensuring that
	 * %emi_pending_req, %emi_ev_cpl and %emi_seq are protected by the
	 * en_eslp lock.
	 */
	EFSYS_LOCK(enp->en_eslp, state);
	EFSYS_ASSERT(emip->emi_pending_req == NULL);
	emip->emi_pending_req = emrp;
	emip->emi_ev_cpl = ev_cpl;
	emip->emi_poll_cnt = 0;
	seq = emip->emi_seq++ & EFX_MASK32(MCDI_HEADER_SEQ);
	new_epoch = emip->emi_new_epoch;
	EFSYS_UNLOCK(enp->en_eslp, state);

	emcop->emco_request_copyin(enp, emrp, seq, ev_cpl, new_epoch);
}

	__checkReturn	boolean_t
efx_mcdi_request_poll(
	__in		efx_nic_t *enp)
{
	efx_mcdi_ops_t *emcop = enp->en_mcdi.em_emcop;
	boolean_t completed;

	EFSYS_ASSERT3U(enp->en_magic, ==, EFX_NIC_MAGIC);
	EFSYS_ASSERT3U(enp->en_mod_flags, &, EFX_MOD_MCDI);
	EFSYS_ASSERT3U(enp->en_features, &, EFX_FEATURE_MCDI);

	completed = B_FALSE;

	if (emcop != NULL && emcop->emco_request_poll != NULL)
		completed = emcop->emco_request_poll(enp);

	return (completed);
}

	__checkReturn	boolean_t
efx_mcdi_request_abort(
	__in		efx_nic_t *enp)
{
	efx_mcdi_iface_t *emip = &(enp->en_mcdi.em_emip);
	efx_mcdi_req_t *emrp;
	boolean_t aborted;
	int state;

	EFSYS_ASSERT3U(enp->en_magic, ==, EFX_NIC_MAGIC);
	EFSYS_ASSERT3U(enp->en_mod_flags, &, EFX_MOD_MCDI);
	EFSYS_ASSERT3U(enp->en_features, &, EFX_FEATURE_MCDI);

	/*
	 * efx_mcdi_ev_* may have already completed this event, and be
	 * spinning/blocked on the upper layer lock. So it *is* legitimate
	 * to for emi_pending_req to be NULL. If there is a pending event
	 * completed request, then provide a "credit" to allow
	 * efx_mcdi_ev_cpl() to accept a single spurious completion.
	 */
	EFSYS_LOCK(enp->en_eslp, state);
	emrp = emip->emi_pending_req;
	aborted = (emrp != NULL);
	if (aborted) {
		emip->emi_pending_req = NULL;

		/* Error the request */
		emrp->emr_out_length_used = 0;
		emrp->emr_rc = ETIMEDOUT;

		/* Provide a credit for seqno/emr_pending_req mismatches */
		if (emip->emi_ev_cpl)
			++emip->emi_aborted;

		/*
		 * The upper layer has called us, so we don't
		 * need to complete the request.
		 */
	}
	EFSYS_UNLOCK(enp->en_eslp, state);

	return (aborted);
}

	__checkReturn	efx_rc_t
efx_mcdi_request_errcode(
	__in		unsigned int err)
{

	switch (err) {
		/* MCDI v1 */
	case MC_CMD_ERR_EPERM:
		return (EACCES);
	case MC_CMD_ERR_ENOENT:
		return (ENOENT);
	case MC_CMD_ERR_EINTR:
		return (EINTR);
	case MC_CMD_ERR_EACCES:
		return (EACCES);
	case MC_CMD_ERR_EBUSY:
		return (EBUSY);
	case MC_CMD_ERR_EINVAL:
		return (EINVAL);
	case MC_CMD_ERR_EDEADLK:
		return (EDEADLK);
	case MC_CMD_ERR_ENOSYS:
		return (ENOTSUP);
	case MC_CMD_ERR_ETIME:
		return (ETIMEDOUT);
	case MC_CMD_ERR_ENOTSUP:
		return (ENOTSUP);
	case MC_CMD_ERR_EALREADY:
		return (EALREADY);

		/* MCDI v2 */
#ifdef MC_CMD_ERR_EAGAIN
	case MC_CMD_ERR_EAGAIN:
		return (EAGAIN);
#endif
#ifdef MC_CMD_ERR_ENOSPC
	case MC_CMD_ERR_ENOSPC:
		return (ENOSPC);
#endif

	case MC_CMD_ERR_ALLOC_FAIL:
		return (ENOMEM);
	case MC_CMD_ERR_NO_VADAPTOR:
		return (ENOENT);
	case MC_CMD_ERR_NO_EVB_PORT:
		return (ENOENT);
	case MC_CMD_ERR_NO_VSWITCH:
		return (ENODEV);
	case MC_CMD_ERR_VLAN_LIMIT:
		return (EINVAL);
	case MC_CMD_ERR_BAD_PCI_FUNC:
		return (ENODEV);
	case MC_CMD_ERR_BAD_VLAN_MODE:
		return (EINVAL);
	case MC_CMD_ERR_BAD_VSWITCH_TYPE:
		return (EINVAL);
	case MC_CMD_ERR_BAD_VPORT_TYPE:
		return (EINVAL);
	case MC_CMD_ERR_MAC_EXIST:
		return (EEXIST);

	default:
		EFSYS_PROBE1(mc_pcol_error, int, err);
		return (EIO);
	}
}

			void
efx_mcdi_raise_exception(
	__in		efx_nic_t *enp,
	__in_opt	efx_mcdi_req_t *emrp,
	__in		int rc)
{
	const efx_mcdi_transport_t *emtp = enp->en_mcdi.em_emtp;
	efx_mcdi_exception_t exception;

	/* Reboot or Assertion failure only */
	EFSYS_ASSERT(rc == EIO || rc == EINTR);

	/*
	 * If MC_CMD_REBOOT causes a reboot (dependent on parameters),
	 * then the EIO is not worthy of an exception.
	 */
	if (emrp != NULL && emrp->emr_cmd == MC_CMD_REBOOT && rc == EIO)
		return;

	exception = (rc == EIO)
		? EFX_MCDI_EXCEPTION_MC_REBOOT
		: EFX_MCDI_EXCEPTION_MC_BADASSERT;

	emtp->emt_exception(emtp->emt_context, exception);
}

static			efx_rc_t
efx_mcdi_poll_reboot(
	__in		efx_nic_t *enp)
{
	efx_mcdi_ops_t *emcop = enp->en_mcdi.em_emcop;

	return (emcop->emco_poll_reboot(enp));
}


			void
efx_mcdi_execute(
	__in		efx_nic_t *enp,
	__inout		efx_mcdi_req_t *emrp)
{
	const efx_mcdi_transport_t *emtp = enp->en_mcdi.em_emtp;

	EFSYS_ASSERT3U(enp->en_mod_flags, &, EFX_MOD_MCDI);
	EFSYS_ASSERT3U(enp->en_features, &, EFX_FEATURE_MCDI);

	emrp->emr_quiet = B_FALSE;
	emtp->emt_execute(emtp->emt_context, emrp);
}

			void
efx_mcdi_execute_quiet(
	__in		efx_nic_t *enp,
	__inout		efx_mcdi_req_t *emrp)
{
	const efx_mcdi_transport_t *emtp = enp->en_mcdi.em_emtp;

	EFSYS_ASSERT3U(enp->en_mod_flags, &, EFX_MOD_MCDI);
	EFSYS_ASSERT3U(enp->en_features, &, EFX_FEATURE_MCDI);

	emrp->emr_quiet = B_TRUE;
	emtp->emt_execute(emtp->emt_context, emrp);
}

			void
efx_mcdi_ev_cpl(
	__in		efx_nic_t *enp,
	__in		unsigned int seq,
	__in		unsigned int outlen,
	__in		int errcode)
{
	efx_mcdi_iface_t *emip = &(enp->en_mcdi.em_emip);
	const efx_mcdi_transport_t *emtp = enp->en_mcdi.em_emtp;
	efx_mcdi_ops_t *emcop = enp->en_mcdi.em_emcop;
	efx_mcdi_req_t *emrp;
	int state;

	EFSYS_ASSERT3U(enp->en_mod_flags, &, EFX_MOD_MCDI);
	EFSYS_ASSERT3U(enp->en_features, &, EFX_FEATURE_MCDI);

	/*
	 * Serialise against efx_mcdi_request_poll()/efx_mcdi_request_start()
	 * when we're completing an aborted request.
	 */
	EFSYS_LOCK(enp->en_eslp, state);
	if (emip->emi_pending_req == NULL || !emip->emi_ev_cpl ||
	    (seq != ((emip->emi_seq - 1) & EFX_MASK32(MCDI_HEADER_SEQ)))) {
		EFSYS_ASSERT(emip->emi_aborted > 0);
		if (emip->emi_aborted > 0)
			--emip->emi_aborted;
		EFSYS_UNLOCK(enp->en_eslp, state);
		return;
	}

	emrp = emip->emi_pending_req;
	emip->emi_pending_req = NULL;
	EFSYS_UNLOCK(enp->en_eslp, state);

	/*
	 * Fill out the remaining hdr fields, and copyout the payload
	 * if the user supplied an output buffer.
	 */
	if (errcode != 0) {
		if (!emrp->emr_quiet) {
			EFSYS_PROBE2(mcdi_err, int, emrp->emr_cmd,
			    int, errcode);
		}
		emrp->emr_out_length_used = 0;
		emrp->emr_rc = efx_mcdi_request_errcode(errcode);
	} else {
		emrp->emr_out_length_used = outlen;
		emrp->emr_rc = 0;
	}
	emcop->emco_request_copyout(enp, emrp);

	emtp->emt_ev_cpl(emtp->emt_context);
}

			void
efx_mcdi_ev_death(
	__in		efx_nic_t *enp,
	__in		int rc)
{
	efx_mcdi_iface_t *emip = &(enp->en_mcdi.em_emip);
	const efx_mcdi_transport_t *emtp = enp->en_mcdi.em_emtp;
	efx_mcdi_req_t *emrp = NULL;
	boolean_t ev_cpl;
	int state;

	/*
	 * The MCDI request (if there is one) has been terminated, either
	 * by a BADASSERT or REBOOT event.
	 *
	 * If there is an outstanding event-completed MCDI operation, then we
	 * will never receive the completion event (because both MCDI
	 * completions and BADASSERT events are sent to the same evq). So
	 * complete this MCDI op.
	 *
	 * This function might run in parallel with efx_mcdi_request_poll()
	 * for poll completed mcdi requests, and also with
	 * efx_mcdi_request_start() for post-watchdog completions.
	 */
	EFSYS_LOCK(enp->en_eslp, state);
	emrp = emip->emi_pending_req;
	ev_cpl = emip->emi_ev_cpl;
	if (emrp != NULL && emip->emi_ev_cpl) {
		emip->emi_pending_req = NULL;

		emrp->emr_out_length_used = 0;
		emrp->emr_rc = rc;
		++emip->emi_aborted;
	}

	/*
	 * Since we're running in parallel with a request, consume the
	 * status word before dropping the lock.
	 */
	if (rc == EIO || rc == EINTR) {
		EFSYS_SPIN(EFX_MCDI_STATUS_SLEEP_US);
		(void) efx_mcdi_poll_reboot(enp);
		emip->emi_new_epoch = B_TRUE;
	}

	EFSYS_UNLOCK(enp->en_eslp, state);

	efx_mcdi_raise_exception(enp, emrp, rc);

	if (emrp != NULL && ev_cpl)
		emtp->emt_ev_cpl(emtp->emt_context);
}

	__checkReturn		efx_rc_t
efx_mcdi_version(
	__in			efx_nic_t *enp,
	__out_ecount_opt(4)	uint16_t versionp[4],
	__out_opt		uint32_t *buildp,
	__out_opt		efx_mcdi_boot_t *statusp)
{
	efx_mcdi_req_t req;
	uint8_t payload[MAX(MAX(MC_CMD_GET_VERSION_IN_LEN,
				MC_CMD_GET_VERSION_OUT_LEN),
			    MAX(MC_CMD_GET_BOOT_STATUS_IN_LEN,
				MC_CMD_GET_BOOT_STATUS_OUT_LEN))];
	efx_word_t *ver_words;
	uint16_t version[4];
	uint32_t build;
	efx_mcdi_boot_t status;
	efx_rc_t rc;

	EFSYS_ASSERT3U(enp->en_features, &, EFX_FEATURE_MCDI);

	(void) memset(payload, 0, sizeof (payload));
	req.emr_cmd = MC_CMD_GET_VERSION;
	req.emr_in_buf = payload;
	req.emr_in_length = MC_CMD_GET_VERSION_IN_LEN;
	req.emr_out_buf = payload;
	req.emr_out_length = MC_CMD_GET_VERSION_OUT_LEN;

	efx_mcdi_execute(enp, &req);

	if (req.emr_rc != 0) {
		rc = req.emr_rc;
		goto fail1;
	}

	/* bootrom support */
	if (req.emr_out_length_used == MC_CMD_GET_VERSION_V0_OUT_LEN) {
		version[0] = version[1] = version[2] = version[3] = 0;
		build = MCDI_OUT_DWORD(req, GET_VERSION_OUT_FIRMWARE);

		goto version;
	}

	if (req.emr_out_length_used < MC_CMD_GET_VERSION_OUT_LEN) {
		rc = EMSGSIZE;
		goto fail2;
	}

	ver_words = MCDI_OUT2(req, efx_word_t, GET_VERSION_OUT_VERSION);
	version[0] = EFX_WORD_FIELD(ver_words[0], EFX_WORD_0);
	version[1] = EFX_WORD_FIELD(ver_words[1], EFX_WORD_0);
	version[2] = EFX_WORD_FIELD(ver_words[2], EFX_WORD_0);
	version[3] = EFX_WORD_FIELD(ver_words[3], EFX_WORD_0);
	build = MCDI_OUT_DWORD(req, GET_VERSION_OUT_FIRMWARE);

version:
	/* The bootrom doesn't understand BOOT_STATUS */
	if (MC_FW_VERSION_IS_BOOTLOADER(build)) {
		status = EFX_MCDI_BOOT_ROM;
		goto out;
	}

	(void) memset(payload, 0, sizeof (payload));
	req.emr_cmd = MC_CMD_GET_BOOT_STATUS;
	req.emr_in_buf = payload;
	req.emr_in_length = MC_CMD_GET_BOOT_STATUS_IN_LEN;
	req.emr_out_buf = payload;
	req.emr_out_length = MC_CMD_GET_BOOT_STATUS_OUT_LEN;

	efx_mcdi_execute_quiet(enp, &req);

	if (req.emr_rc == EACCES) {
		/* Unprivileged functions cannot access BOOT_STATUS */
		status = EFX_MCDI_BOOT_PRIMARY;
		version[0] = version[1] = version[2] = version[3] = 0;
		build = 0;
		goto out;
	}

	if (req.emr_rc != 0) {
		rc = req.emr_rc;
		goto fail3;
	}

	if (req.emr_out_length_used < MC_CMD_GET_BOOT_STATUS_OUT_LEN) {
		rc = EMSGSIZE;
		goto fail4;
	}

	if (MCDI_OUT_DWORD_FIELD(req, GET_BOOT_STATUS_OUT_FLAGS,
	    GET_BOOT_STATUS_OUT_FLAGS_PRIMARY))
		status = EFX_MCDI_BOOT_PRIMARY;
	else
		status = EFX_MCDI_BOOT_SECONDARY;

out:
	if (versionp != NULL)
		memcpy(versionp, version, sizeof (version));
	if (buildp != NULL)
		*buildp = build;
	if (statusp != NULL)
		*statusp = status;

	return (0);

fail4:
	EFSYS_PROBE(fail4);
fail3:
	EFSYS_PROBE(fail3);
fail2:
	EFSYS_PROBE(fail2);
fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

static	__checkReturn	efx_rc_t
efx_mcdi_do_reboot(
	__in		efx_nic_t *enp,
	__in		boolean_t after_assertion)
{
	uint8_t payload[MAX(MC_CMD_REBOOT_IN_LEN, MC_CMD_REBOOT_OUT_LEN)];
	efx_mcdi_req_t req;
	efx_rc_t rc;

	/*
	 * We could require the caller to have caused en_mod_flags=0 to
	 * call this function. This doesn't help the other port though,
	 * who's about to get the MC ripped out from underneath them.
	 * Since they have to cope with the subsequent fallout of MCDI
	 * failures, we should as well.
	 */
	EFSYS_ASSERT3U(enp->en_magic, ==, EFX_NIC_MAGIC);

	(void) memset(payload, 0, sizeof (payload));
	req.emr_cmd = MC_CMD_REBOOT;
	req.emr_in_buf = payload;
	req.emr_in_length = MC_CMD_REBOOT_IN_LEN;
	req.emr_out_buf = payload;
	req.emr_out_length = MC_CMD_REBOOT_OUT_LEN;

	MCDI_IN_SET_DWORD(req, REBOOT_IN_FLAGS,
	    (after_assertion ? MC_CMD_REBOOT_FLAGS_AFTER_ASSERTION : 0));

	efx_mcdi_execute_quiet(enp, &req);

	if (req.emr_rc == EACCES) {
		/* Unprivileged functions cannot reboot the MC. */
		goto out;
	}

	/* A successful reboot request returns EIO. */
	if (req.emr_rc != 0 && req.emr_rc != EIO) {
		rc = req.emr_rc;
		goto fail1;
	}

out:
	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

	__checkReturn	efx_rc_t
efx_mcdi_reboot(
	__in		efx_nic_t *enp)
{
	return (efx_mcdi_do_reboot(enp, B_FALSE));
}

	__checkReturn	efx_rc_t
efx_mcdi_exit_assertion_handler(
	__in		efx_nic_t *enp)
{
	return (efx_mcdi_do_reboot(enp, B_TRUE));
}

	__checkReturn	efx_rc_t
efx_mcdi_read_assertion(
	__in		efx_nic_t *enp)
{
	efx_mcdi_req_t req;
	uint8_t payload[MAX(MC_CMD_GET_ASSERTS_IN_LEN,
			    MC_CMD_GET_ASSERTS_OUT_LEN)];
	const char *reason;
	unsigned int flags;
	unsigned int index;
	unsigned int ofst;
	int retry;
	efx_rc_t rc;

	/*
	 * Before we attempt to chat to the MC, we should verify that the MC
	 * isn't in it's assertion handler, either due to a previous reboot,
	 * or because we're reinitializing due to an eec_exception().
	 *
	 * Use GET_ASSERTS to read any assertion state that may be present.
	 * Retry this command twice. Once because a boot-time assertion failure
	 * might cause the 1st MCDI request to fail. And once again because
	 * we might race with efx_mcdi_exit_assertion_handler() running on
	 * partner port(s) on the same NIC.
	 */
	retry = 2;
	do {
		(void) memset(payload, 0, sizeof (payload));
		req.emr_cmd = MC_CMD_GET_ASSERTS;
		req.emr_in_buf = payload;
		req.emr_in_length = MC_CMD_GET_ASSERTS_IN_LEN;
		req.emr_out_buf = payload;
		req.emr_out_length = MC_CMD_GET_ASSERTS_OUT_LEN;

		MCDI_IN_SET_DWORD(req, GET_ASSERTS_IN_CLEAR, 1);
		efx_mcdi_execute_quiet(enp, &req);

	} while ((req.emr_rc == EINTR || req.emr_rc == EIO) && retry-- > 0);

	if (req.emr_rc != 0) {
		if (req.emr_rc == EACCES) {
			/* Unprivileged functions cannot clear assertions. */
			goto out;
		}
		rc = req.emr_rc;
		goto fail1;
	}

	if (req.emr_out_length_used < MC_CMD_GET_ASSERTS_OUT_LEN) {
		rc = EMSGSIZE;
		goto fail2;
	}

	/* Print out any assertion state recorded */
	flags = MCDI_OUT_DWORD(req, GET_ASSERTS_OUT_GLOBAL_FLAGS);
	if (flags == MC_CMD_GET_ASSERTS_FLAGS_NO_FAILS)
		return (0);

	reason = (flags == MC_CMD_GET_ASSERTS_FLAGS_SYS_FAIL)
		? "system-level assertion"
		: (flags == MC_CMD_GET_ASSERTS_FLAGS_THR_FAIL)
		? "thread-level assertion"
		: (flags == MC_CMD_GET_ASSERTS_FLAGS_WDOG_FIRED)
		? "watchdog reset"
		: (flags == MC_CMD_GET_ASSERTS_FLAGS_ADDR_TRAP)
		? "illegal address trap"
		: "unknown assertion";
	EFSYS_PROBE3(mcpu_assertion,
	    const char *, reason, unsigned int,
	    MCDI_OUT_DWORD(req, GET_ASSERTS_OUT_SAVED_PC_OFFS),
	    unsigned int,
	    MCDI_OUT_DWORD(req, GET_ASSERTS_OUT_THREAD_OFFS));

	/* Print out the registers (r1 ... r31) */
	ofst = MC_CMD_GET_ASSERTS_OUT_GP_REGS_OFFS_OFST;
	for (index = 1;
		index < 1 + MC_CMD_GET_ASSERTS_OUT_GP_REGS_OFFS_NUM;
		index++) {
		EFSYS_PROBE2(mcpu_register, unsigned int, index, unsigned int,
			    EFX_DWORD_FIELD(*MCDI_OUT(req, efx_dword_t, ofst),
					    EFX_DWORD_0));
		ofst += sizeof (efx_dword_t);
	}
	EFSYS_ASSERT(ofst <= MC_CMD_GET_ASSERTS_OUT_LEN);

out:
	return (0);

fail2:
	EFSYS_PROBE(fail2);
fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}


/*
 * Internal routines for for specific MCDI requests.
 */

	__checkReturn	efx_rc_t
efx_mcdi_drv_attach(
	__in		efx_nic_t *enp,
	__in		boolean_t attach)
{
	efx_nic_cfg_t *encp = &(enp->en_nic_cfg);
	efx_mcdi_req_t req;
	uint8_t payload[MAX(MC_CMD_DRV_ATTACH_IN_LEN,
			    MC_CMD_DRV_ATTACH_EXT_OUT_LEN)];
	uint32_t flags;
	efx_rc_t rc;

	(void) memset(payload, 0, sizeof (payload));
	req.emr_cmd = MC_CMD_DRV_ATTACH;
	req.emr_in_buf = payload;
	req.emr_in_length = MC_CMD_DRV_ATTACH_IN_LEN;
	req.emr_out_buf = payload;
	req.emr_out_length = MC_CMD_DRV_ATTACH_EXT_OUT_LEN;

	/*
	 * Use DONT_CARE for the datapath firmware type to ensure that the
	 * driver can attach to an unprivileged function. The datapath firmware
	 * type to use is controlled by the 'sfboot' utility.
	 */
	MCDI_IN_SET_DWORD(req, DRV_ATTACH_IN_NEW_STATE, attach ? 1 : 0);
	MCDI_IN_SET_DWORD(req, DRV_ATTACH_IN_UPDATE, 1);
	MCDI_IN_SET_DWORD(req, DRV_ATTACH_IN_FIRMWARE_ID, MC_CMD_FW_DONT_CARE);

	efx_mcdi_execute(enp, &req);

	if (req.emr_rc != 0) {
		rc = req.emr_rc;
		goto fail1;
	}

	if (req.emr_out_length_used < MC_CMD_DRV_ATTACH_OUT_LEN) {
		rc = EMSGSIZE;
		goto fail2;
	}

	if (attach == B_FALSE) {
		flags = 0;
	} else if (enp->en_family == EFX_FAMILY_SIENA) {
		efx_mcdi_iface_t *emip = &(enp->en_mcdi.em_emip);

		/* Create synthetic privileges for Siena functions */
		flags = EFX_NIC_FUNC_LINKCTRL | EFX_NIC_FUNC_TRUSTED;
		if (emip->emi_port == 1)
			flags |= EFX_NIC_FUNC_PRIMARY;
	} else {
		EFX_STATIC_ASSERT(EFX_NIC_FUNC_PRIMARY ==
		    (1u << MC_CMD_DRV_ATTACH_EXT_OUT_FLAG_PRIMARY));
		EFX_STATIC_ASSERT(EFX_NIC_FUNC_LINKCTRL ==
		    (1u << MC_CMD_DRV_ATTACH_EXT_OUT_FLAG_LINKCTRL));
		EFX_STATIC_ASSERT(EFX_NIC_FUNC_TRUSTED ==
		    (1u << MC_CMD_DRV_ATTACH_EXT_OUT_FLAG_TRUSTED));

		/* Save function privilege flags (EF10 and later) */
		if (req.emr_out_length_used < MC_CMD_DRV_ATTACH_EXT_OUT_LEN) {
			rc = EMSGSIZE;
			goto fail3;
		}
		flags = MCDI_OUT_DWORD(req, DRV_ATTACH_EXT_OUT_FUNC_FLAGS);
	}
	encp->enc_func_flags = flags;

	return (0);

fail3:
	EFSYS_PROBE(fail3);
fail2:
	EFSYS_PROBE(fail2);
fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

	__checkReturn		efx_rc_t
efx_mcdi_get_board_cfg(
	__in			efx_nic_t *enp,
	__out_opt		uint32_t *board_typep,
	__out_opt		efx_dword_t *capabilitiesp,
	__out_ecount_opt(6)	uint8_t mac_addrp[6])
{
	efx_mcdi_iface_t *emip = &(enp->en_mcdi.em_emip);
	efx_mcdi_req_t req;
	uint8_t payload[MAX(MC_CMD_GET_BOARD_CFG_IN_LEN,
			    MC_CMD_GET_BOARD_CFG_OUT_LENMIN)];
	efx_rc_t rc;

	(void) memset(payload, 0, sizeof (payload));
	req.emr_cmd = MC_CMD_GET_BOARD_CFG;
	req.emr_in_buf = payload;
	req.emr_in_length = MC_CMD_GET_BOARD_CFG_IN_LEN;
	req.emr_out_buf = payload;
	req.emr_out_length = MC_CMD_GET_BOARD_CFG_OUT_LENMIN;

	efx_mcdi_execute(enp, &req);

	if (req.emr_rc != 0) {
		rc = req.emr_rc;
		goto fail1;
	}

	if (req.emr_out_length_used < MC_CMD_GET_BOARD_CFG_OUT_LENMIN) {
		rc = EMSGSIZE;
		goto fail2;
	}

	if (mac_addrp != NULL) {
		uint8_t *addrp;

		if (emip->emi_port == 1) {
			addrp = MCDI_OUT2(req, uint8_t,
			    GET_BOARD_CFG_OUT_MAC_ADDR_BASE_PORT0);
		} else if (emip->emi_port == 2) {
			addrp = MCDI_OUT2(req, uint8_t,
			    GET_BOARD_CFG_OUT_MAC_ADDR_BASE_PORT1);
		} else {
			rc = EINVAL;
			goto fail3;
		}

		EFX_MAC_ADDR_COPY(mac_addrp, addrp);
	}

	if (capabilitiesp != NULL) {
		if (emip->emi_port == 1) {
			*capabilitiesp = *MCDI_OUT2(req, efx_dword_t,
			    GET_BOARD_CFG_OUT_CAPABILITIES_PORT0);
		} else if (emip->emi_port == 2) {
			*capabilitiesp = *MCDI_OUT2(req, efx_dword_t,
			    GET_BOARD_CFG_OUT_CAPABILITIES_PORT1);
		} else {
			rc = EINVAL;
			goto fail4;
		}
	}

	if (board_typep != NULL) {
		*board_typep = MCDI_OUT_DWORD(req,
		    GET_BOARD_CFG_OUT_BOARD_TYPE);
	}

	return (0);

fail4:
	EFSYS_PROBE(fail4);
fail3:
	EFSYS_PROBE(fail3);
fail2:
	EFSYS_PROBE(fail2);
fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

	__checkReturn	efx_rc_t
efx_mcdi_get_resource_limits(
	__in		efx_nic_t *enp,
	__out_opt	uint32_t *nevqp,
	__out_opt	uint32_t *nrxqp,
	__out_opt	uint32_t *ntxqp)
{
	efx_mcdi_req_t req;
	uint8_t payload[MAX(MC_CMD_GET_RESOURCE_LIMITS_IN_LEN,
			    MC_CMD_GET_RESOURCE_LIMITS_OUT_LEN)];
	efx_rc_t rc;

	(void) memset(payload, 0, sizeof (payload));
	req.emr_cmd = MC_CMD_GET_RESOURCE_LIMITS;
	req.emr_in_buf = payload;
	req.emr_in_length = MC_CMD_GET_RESOURCE_LIMITS_IN_LEN;
	req.emr_out_buf = payload;
	req.emr_out_length = MC_CMD_GET_RESOURCE_LIMITS_OUT_LEN;

	efx_mcdi_execute(enp, &req);

	if (req.emr_rc != 0) {
		rc = req.emr_rc;
		goto fail1;
	}

	if (req.emr_out_length_used < MC_CMD_GET_RESOURCE_LIMITS_OUT_LEN) {
		rc = EMSGSIZE;
		goto fail2;
	}

	if (nevqp != NULL)
		*nevqp = MCDI_OUT_DWORD(req, GET_RESOURCE_LIMITS_OUT_EVQ);
	if (nrxqp != NULL)
		*nrxqp = MCDI_OUT_DWORD(req, GET_RESOURCE_LIMITS_OUT_RXQ);
	if (ntxqp != NULL)
		*ntxqp = MCDI_OUT_DWORD(req, GET_RESOURCE_LIMITS_OUT_TXQ);

	return (0);

fail2:
	EFSYS_PROBE(fail2);
fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

	__checkReturn	efx_rc_t
efx_mcdi_get_phy_cfg(
	__in		efx_nic_t *enp)
{
	efx_port_t *epp = &(enp->en_port);
	efx_nic_cfg_t *encp = &(enp->en_nic_cfg);
	efx_mcdi_req_t req;
	uint8_t payload[MAX(MC_CMD_GET_PHY_CFG_IN_LEN,
			    MC_CMD_GET_PHY_CFG_OUT_LEN)];
	efx_rc_t rc;

	(void) memset(payload, 0, sizeof (payload));
	req.emr_cmd = MC_CMD_GET_PHY_CFG;
	req.emr_in_buf = payload;
	req.emr_in_length = MC_CMD_GET_PHY_CFG_IN_LEN;
	req.emr_out_buf = payload;
	req.emr_out_length = MC_CMD_GET_PHY_CFG_OUT_LEN;

	efx_mcdi_execute(enp, &req);

	if (req.emr_rc != 0) {
		rc = req.emr_rc;
		goto fail1;
	}

	if (req.emr_out_length_used < MC_CMD_GET_PHY_CFG_OUT_LEN) {
		rc = EMSGSIZE;
		goto fail2;
	}

	encp->enc_phy_type = MCDI_OUT_DWORD(req, GET_PHY_CFG_OUT_TYPE);
#if EFSYS_OPT_NAMES
	(void) strncpy(encp->enc_phy_name,
		MCDI_OUT2(req, char, GET_PHY_CFG_OUT_NAME),
		MIN(sizeof (encp->enc_phy_name) - 1,
		    MC_CMD_GET_PHY_CFG_OUT_NAME_LEN));
#endif	/* EFSYS_OPT_NAMES */
	(void) memset(encp->enc_phy_revision, 0,
	    sizeof (encp->enc_phy_revision));
	memcpy(encp->enc_phy_revision,
		MCDI_OUT2(req, char, GET_PHY_CFG_OUT_REVISION),
		MIN(sizeof (encp->enc_phy_revision) - 1,
		    MC_CMD_GET_PHY_CFG_OUT_REVISION_LEN));
#if EFSYS_OPT_PHY_LED_CONTROL
	encp->enc_led_mask = ((1 << EFX_PHY_LED_DEFAULT) |
			    (1 << EFX_PHY_LED_OFF) |
			    (1 << EFX_PHY_LED_ON));
#endif	/* EFSYS_OPT_PHY_LED_CONTROL */

#if EFSYS_OPT_PHY_PROPS
	encp->enc_phy_nprops  = 0;
#endif	/* EFSYS_OPT_PHY_PROPS */

	/* Get the media type of the fixed port, if recognised. */
	EFX_STATIC_ASSERT(MC_CMD_MEDIA_XAUI == EFX_PHY_MEDIA_XAUI);
	EFX_STATIC_ASSERT(MC_CMD_MEDIA_CX4 == EFX_PHY_MEDIA_CX4);
	EFX_STATIC_ASSERT(MC_CMD_MEDIA_KX4 == EFX_PHY_MEDIA_KX4);
	EFX_STATIC_ASSERT(MC_CMD_MEDIA_XFP == EFX_PHY_MEDIA_XFP);
	EFX_STATIC_ASSERT(MC_CMD_MEDIA_SFP_PLUS == EFX_PHY_MEDIA_SFP_PLUS);
	EFX_STATIC_ASSERT(MC_CMD_MEDIA_BASE_T == EFX_PHY_MEDIA_BASE_T);
	EFX_STATIC_ASSERT(MC_CMD_MEDIA_QSFP_PLUS == EFX_PHY_MEDIA_QSFP_PLUS);
	epp->ep_fixed_port_type =
		MCDI_OUT_DWORD(req, GET_PHY_CFG_OUT_MEDIA_TYPE);
	if (epp->ep_fixed_port_type >= EFX_PHY_MEDIA_NTYPES)
		epp->ep_fixed_port_type = EFX_PHY_MEDIA_INVALID;

	epp->ep_phy_cap_mask =
		MCDI_OUT_DWORD(req, GET_PHY_CFG_OUT_SUPPORTED_CAP);
#if EFSYS_OPT_PHY_FLAGS
	encp->enc_phy_flags_mask = MCDI_OUT_DWORD(req, GET_PHY_CFG_OUT_FLAGS);
#endif	/* EFSYS_OPT_PHY_FLAGS */

	encp->enc_port = (uint8_t)MCDI_OUT_DWORD(req, GET_PHY_CFG_OUT_PRT);

	/* Populate internal state */
	encp->enc_mcdi_mdio_channel =
		(uint8_t)MCDI_OUT_DWORD(req, GET_PHY_CFG_OUT_CHANNEL);

#if EFSYS_OPT_PHY_STATS
	encp->enc_mcdi_phy_stat_mask =
		MCDI_OUT_DWORD(req, GET_PHY_CFG_OUT_STATS_MASK);
#endif	/* EFSYS_OPT_PHY_STATS */

#if EFSYS_OPT_BIST
	encp->enc_bist_mask = 0;
	if (MCDI_OUT_DWORD_FIELD(req, GET_PHY_CFG_OUT_FLAGS,
	    GET_PHY_CFG_OUT_BIST_CABLE_SHORT))
		encp->enc_bist_mask |= (1 << EFX_BIST_TYPE_PHY_CABLE_SHORT);
	if (MCDI_OUT_DWORD_FIELD(req, GET_PHY_CFG_OUT_FLAGS,
	    GET_PHY_CFG_OUT_BIST_CABLE_LONG))
		encp->enc_bist_mask |= (1 << EFX_BIST_TYPE_PHY_CABLE_LONG);
	if (MCDI_OUT_DWORD_FIELD(req, GET_PHY_CFG_OUT_FLAGS,
	    GET_PHY_CFG_OUT_BIST))
		encp->enc_bist_mask |= (1 << EFX_BIST_TYPE_PHY_NORMAL);
#endif  /* EFSYS_OPT_BIST */

	return (0);

fail2:
	EFSYS_PROBE(fail2);
fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}


	__checkReturn		efx_rc_t
efx_mcdi_firmware_update_supported(
	__in			efx_nic_t *enp,
	__out			boolean_t *supportedp)
{
	efx_mcdi_ops_t *emcop = enp->en_mcdi.em_emcop;
	efx_rc_t rc;

	if (emcop != NULL && emcop->emco_fw_update_supported != NULL) {
		if ((rc = emcop->emco_fw_update_supported(enp, supportedp))
		    != 0)
			goto fail1;
	} else {
		/* Earlier devices always supported updates */
		*supportedp = B_TRUE;
	}

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

	__checkReturn		efx_rc_t
efx_mcdi_macaddr_change_supported(
	__in			efx_nic_t *enp,
	__out			boolean_t *supportedp)
{
	efx_mcdi_ops_t *emcop = enp->en_mcdi.em_emcop;
	efx_rc_t rc;

	if (emcop != NULL && emcop->emco_macaddr_change_supported != NULL) {
		if ((rc = emcop->emco_macaddr_change_supported(enp, supportedp))
		    != 0)
			goto fail1;
	} else {
		/* Earlier devices always supported MAC changes */
		*supportedp = B_TRUE;
	}

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

	__checkReturn		efx_rc_t
efx_mcdi_link_control_supported(
	__in			efx_nic_t *enp,
	__out			boolean_t *supportedp)
{
	efx_mcdi_ops_t *emcop = enp->en_mcdi.em_emcop;
	efx_rc_t rc;

	if (emcop != NULL && emcop->emco_link_control_supported != NULL) {
		if ((rc = emcop->emco_link_control_supported(enp, supportedp))
		    != 0)
			goto fail1;
	} else {
		/* Earlier devices always supported link control */
		*supportedp = B_TRUE;
	}

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

#if EFSYS_OPT_BIST

#if EFSYS_OPT_HUNTINGTON
/*
 * Enter bist offline mode. This is a fw mode which puts the NIC into a state
 * where memory BIST tests can be run and not much else can interfere or happen.
 * A reboot is required to exit this mode.
 */
	__checkReturn		efx_rc_t
efx_mcdi_bist_enable_offline(
	__in			efx_nic_t *enp)
{
	efx_mcdi_req_t req;
	efx_rc_t rc;

	EFX_STATIC_ASSERT(MC_CMD_ENABLE_OFFLINE_BIST_IN_LEN == 0);
	EFX_STATIC_ASSERT(MC_CMD_ENABLE_OFFLINE_BIST_OUT_LEN == 0);

	req.emr_cmd = MC_CMD_ENABLE_OFFLINE_BIST;
	req.emr_in_buf = NULL;
	req.emr_in_length = 0;
	req.emr_out_buf = NULL;
	req.emr_out_length = 0;

	efx_mcdi_execute(enp, &req);

	if (req.emr_rc != 0) {
		rc = req.emr_rc;
		goto fail1;
	}

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}
#endif /* EFSYS_OPT_HUNTINGTON */

	__checkReturn		efx_rc_t
efx_mcdi_bist_start(
	__in			efx_nic_t *enp,
	__in			efx_bist_type_t type)
{
	efx_mcdi_req_t req;
	uint8_t payload[MAX(MC_CMD_START_BIST_IN_LEN,
			    MC_CMD_START_BIST_OUT_LEN)];
	efx_rc_t rc;

	(void) memset(payload, 0, sizeof (payload));
	req.emr_cmd = MC_CMD_START_BIST;
	req.emr_in_buf = payload;
	req.emr_in_length = MC_CMD_START_BIST_IN_LEN;
	req.emr_out_buf = payload;
	req.emr_out_length = MC_CMD_START_BIST_OUT_LEN;

	switch (type) {
	case EFX_BIST_TYPE_PHY_NORMAL:
		MCDI_IN_SET_DWORD(req, START_BIST_IN_TYPE, MC_CMD_PHY_BIST);
		break;
	case EFX_BIST_TYPE_PHY_CABLE_SHORT:
		MCDI_IN_SET_DWORD(req, START_BIST_IN_TYPE,
		    MC_CMD_PHY_BIST_CABLE_SHORT);
		break;
	case EFX_BIST_TYPE_PHY_CABLE_LONG:
		MCDI_IN_SET_DWORD(req, START_BIST_IN_TYPE,
		    MC_CMD_PHY_BIST_CABLE_LONG);
		break;
	case EFX_BIST_TYPE_MC_MEM:
		MCDI_IN_SET_DWORD(req, START_BIST_IN_TYPE,
		    MC_CMD_MC_MEM_BIST);
		break;
	case EFX_BIST_TYPE_SAT_MEM:
		MCDI_IN_SET_DWORD(req, START_BIST_IN_TYPE,
		    MC_CMD_PORT_MEM_BIST);
		break;
	case EFX_BIST_TYPE_REG:
		MCDI_IN_SET_DWORD(req, START_BIST_IN_TYPE,
		    MC_CMD_REG_BIST);
		break;
	default:
		EFSYS_ASSERT(0);
	}

	efx_mcdi_execute(enp, &req);

	if (req.emr_rc != 0) {
		rc = req.emr_rc;
		goto fail1;
	}

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

#endif /* EFSYS_OPT_BIST */


/* Enable logging of some events (e.g. link state changes) */
	__checkReturn	efx_rc_t
efx_mcdi_log_ctrl(
	__in		efx_nic_t *enp)
{
	efx_mcdi_req_t req;
	uint8_t payload[MAX(MC_CMD_LOG_CTRL_IN_LEN,
			    MC_CMD_LOG_CTRL_OUT_LEN)];
	efx_rc_t rc;

	(void) memset(payload, 0, sizeof (payload));
	req.emr_cmd = MC_CMD_LOG_CTRL;
	req.emr_in_buf = payload;
	req.emr_in_length = MC_CMD_LOG_CTRL_IN_LEN;
	req.emr_out_buf = payload;
	req.emr_out_length = MC_CMD_LOG_CTRL_OUT_LEN;

	MCDI_IN_SET_DWORD(req, LOG_CTRL_IN_LOG_DEST,
		    MC_CMD_LOG_CTRL_IN_LOG_DEST_EVQ);
	MCDI_IN_SET_DWORD(req, LOG_CTRL_IN_LOG_DEST_EVQ, 0);

	efx_mcdi_execute(enp, &req);

	if (req.emr_rc != 0) {
		rc = req.emr_rc;
		goto fail1;
	}

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}


#if EFSYS_OPT_MAC_STATS

typedef enum efx_stats_action_e
{
	EFX_STATS_CLEAR,
	EFX_STATS_UPLOAD,
	EFX_STATS_ENABLE_NOEVENTS,
	EFX_STATS_ENABLE_EVENTS,
	EFX_STATS_DISABLE,
} efx_stats_action_t;

static	__checkReturn	efx_rc_t
efx_mcdi_mac_stats(
	__in		efx_nic_t *enp,
	__in_opt	efsys_mem_t *esmp,
	__in		efx_stats_action_t action)
{
	efx_mcdi_req_t req;
	uint8_t payload[MAX(MC_CMD_MAC_STATS_IN_LEN,
			    MC_CMD_MAC_STATS_OUT_DMA_LEN)];
	int clear = (action == EFX_STATS_CLEAR);
	int upload = (action == EFX_STATS_UPLOAD);
	int enable = (action == EFX_STATS_ENABLE_NOEVENTS);
	int events = (action == EFX_STATS_ENABLE_EVENTS);
	int disable = (action == EFX_STATS_DISABLE);
	efx_rc_t rc;

	(void) memset(payload, 0, sizeof (payload));
	req.emr_cmd = MC_CMD_MAC_STATS;
	req.emr_in_buf = payload;
	req.emr_in_length = MC_CMD_MAC_STATS_IN_LEN;
	req.emr_out_buf = payload;
	req.emr_out_length = MC_CMD_MAC_STATS_OUT_DMA_LEN;

	MCDI_IN_POPULATE_DWORD_6(req, MAC_STATS_IN_CMD,
	    MAC_STATS_IN_DMA, upload,
	    MAC_STATS_IN_CLEAR, clear,
	    MAC_STATS_IN_PERIODIC_CHANGE, enable | events | disable,
	    MAC_STATS_IN_PERIODIC_ENABLE, enable | events,
	    MAC_STATS_IN_PERIODIC_NOEVENT, !events,
	    MAC_STATS_IN_PERIOD_MS, (enable | events) ? 1000: 0);

	if (esmp != NULL) {
		int bytes = MC_CMD_MAC_NSTATS * sizeof (uint64_t);

		EFX_STATIC_ASSERT(MC_CMD_MAC_NSTATS * sizeof (uint64_t) <=
		    EFX_MAC_STATS_SIZE);

		MCDI_IN_SET_DWORD(req, MAC_STATS_IN_DMA_ADDR_LO,
			    EFSYS_MEM_ADDR(esmp) & 0xffffffff);
		MCDI_IN_SET_DWORD(req, MAC_STATS_IN_DMA_ADDR_HI,
			    EFSYS_MEM_ADDR(esmp) >> 32);
		MCDI_IN_SET_DWORD(req, MAC_STATS_IN_DMA_LEN, bytes);
	} else {
		EFSYS_ASSERT(!upload && !enable && !events);
	}

	/*
	 * NOTE: Do not use EVB_PORT_ID_ASSIGNED when disabling periodic stats,
	 *	 as this may fail (and leave periodic DMA enabled) if the
	 *	 vadapter has already been deleted.
	 */
	MCDI_IN_SET_DWORD(req, MAC_STATS_IN_PORT_ID,
	    (disable ? EVB_PORT_ID_NULL : enp->en_vport_id));

	efx_mcdi_execute(enp, &req);

	if (req.emr_rc != 0) {
		/* EF10: Expect ENOENT if no DMA queues are initialised */
		if ((req.emr_rc != ENOENT) ||
		    (enp->en_rx_qcount + enp->en_tx_qcount != 0)) {
			rc = req.emr_rc;
			goto fail1;
		}
	}

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

	__checkReturn	efx_rc_t
efx_mcdi_mac_stats_clear(
	__in		efx_nic_t *enp)
{
	efx_rc_t rc;

	if ((rc = efx_mcdi_mac_stats(enp, NULL, EFX_STATS_CLEAR)) != 0)
		goto fail1;

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

	__checkReturn	efx_rc_t
efx_mcdi_mac_stats_upload(
	__in		efx_nic_t *enp,
	__in		efsys_mem_t *esmp)
{
	efx_rc_t rc;

	/*
	 * The MC DMAs aggregate statistics for our convenience, so we can
	 * avoid having to pull the statistics buffer into the cache to
	 * maintain cumulative statistics.
	 */
	if ((rc = efx_mcdi_mac_stats(enp, esmp, EFX_STATS_UPLOAD)) != 0)
		goto fail1;

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

	__checkReturn	efx_rc_t
efx_mcdi_mac_stats_periodic(
	__in		efx_nic_t *enp,
	__in		efsys_mem_t *esmp,
	__in		uint16_t period,
	__in		boolean_t events)
{
	efx_rc_t rc;

	/*
	 * The MC DMAs aggregate statistics for our convenience, so we can
	 * avoid having to pull the statistics buffer into the cache to
	 * maintain cumulative statistics.
	 * Huntington uses a fixed 1sec period, so use that on Siena too.
	 */
	if (period == 0)
		rc = efx_mcdi_mac_stats(enp, NULL, EFX_STATS_DISABLE);
	else if (events)
		rc = efx_mcdi_mac_stats(enp, esmp, EFX_STATS_ENABLE_EVENTS);
	else
		rc = efx_mcdi_mac_stats(enp, esmp, EFX_STATS_ENABLE_NOEVENTS);

	if (rc != 0)
		goto fail1;

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

#endif	/* EFSYS_OPT_MAC_STATS */

#if EFSYS_OPT_HUNTINGTON

/*
 * This function returns the pf and vf number of a function.  If it is a pf the
 * vf number is 0xffff.  The vf number is the index of the vf on that
 * function. So if you have 3 vfs on pf 0 the 3 vfs will return (pf=0,vf=0),
 * (pf=0,vf=1), (pf=0,vf=2) aand the pf will return (pf=0, vf=0xffff).
 */
	__checkReturn		efx_rc_t
efx_mcdi_get_function_info(
	__in			efx_nic_t *enp,
	__out			uint32_t *pfp,
	__out_opt		uint32_t *vfp)
{
	efx_mcdi_req_t req;
	uint8_t payload[MAX(MC_CMD_GET_FUNCTION_INFO_IN_LEN,
			    MC_CMD_GET_FUNCTION_INFO_OUT_LEN)];
	efx_rc_t rc;

	(void) memset(payload, 0, sizeof (payload));
	req.emr_cmd = MC_CMD_GET_FUNCTION_INFO;
	req.emr_in_buf = payload;
	req.emr_in_length = MC_CMD_GET_FUNCTION_INFO_IN_LEN;
	req.emr_out_buf = payload;
	req.emr_out_length = MC_CMD_GET_FUNCTION_INFO_OUT_LEN;

	efx_mcdi_execute(enp, &req);

	if (req.emr_rc != 0) {
		rc = req.emr_rc;
		goto fail1;
	}

	if (req.emr_out_length_used < MC_CMD_GET_FUNCTION_INFO_OUT_LEN) {
		rc = EMSGSIZE;
		goto fail2;
	}

	*pfp = MCDI_OUT_DWORD(req, GET_FUNCTION_INFO_OUT_PF);
	if (vfp != NULL)
		*vfp = MCDI_OUT_DWORD(req, GET_FUNCTION_INFO_OUT_VF);

	return (0);

fail2:
	EFSYS_PROBE(fail2);
fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

	__checkReturn		efx_rc_t
efx_mcdi_privilege_mask(
	__in			efx_nic_t *enp,
	__in			uint32_t pf,
	__in			uint32_t vf,
	__out			uint32_t *maskp)
{
	efx_mcdi_req_t req;
	uint8_t payload[MAX(MC_CMD_PRIVILEGE_MASK_IN_LEN,
			    MC_CMD_PRIVILEGE_MASK_OUT_LEN)];
	efx_rc_t rc;

	(void) memset(payload, 0, sizeof (payload));
	req.emr_cmd = MC_CMD_PRIVILEGE_MASK;
	req.emr_in_buf = payload;
	req.emr_in_length = MC_CMD_PRIVILEGE_MASK_IN_LEN;
	req.emr_out_buf = payload;
	req.emr_out_length = MC_CMD_PRIVILEGE_MASK_OUT_LEN;

	MCDI_IN_POPULATE_DWORD_2(req, PRIVILEGE_MASK_IN_FUNCTION,
	    PRIVILEGE_MASK_IN_FUNCTION_PF, pf,
	    PRIVILEGE_MASK_IN_FUNCTION_VF, vf);

	efx_mcdi_execute(enp, &req);

	if (req.emr_rc != 0) {
		rc = req.emr_rc;
		goto fail1;
	}

	if (req.emr_out_length_used < MC_CMD_PRIVILEGE_MASK_OUT_LEN) {
		rc = EMSGSIZE;
		goto fail2;
	}

	*maskp = MCDI_OUT_DWORD(req, PRIVILEGE_MASK_OUT_OLD_MASK);

	return (0);

fail2:
	EFSYS_PROBE(fail2);
fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}

#endif /* EFSYS_OPT_HUNTINGTON */

	__checkReturn		efx_rc_t
efx_mcdi_set_workaround(
	__in			efx_nic_t *enp,
	__in			uint32_t type,
	__in			boolean_t enabled,
	__out_opt		uint32_t *flagsp)
{
	efx_mcdi_req_t req;
	uint8_t payload[MAX(MC_CMD_WORKAROUND_IN_LEN,
			    MC_CMD_WORKAROUND_EXT_OUT_LEN)];
	efx_rc_t rc;

	(void) memset(payload, 0, sizeof (payload));
	req.emr_cmd = MC_CMD_WORKAROUND;
	req.emr_in_buf = payload;
	req.emr_in_length = MC_CMD_WORKAROUND_IN_LEN;
	req.emr_out_buf = payload;
	req.emr_out_length = MC_CMD_WORKAROUND_OUT_LEN;

	MCDI_IN_SET_DWORD(req, WORKAROUND_IN_TYPE, type);
	MCDI_IN_SET_DWORD(req, WORKAROUND_IN_ENABLED, enabled ? 1 : 0);

	efx_mcdi_execute_quiet(enp, &req);

	if (req.emr_rc != 0) {
		rc = req.emr_rc;
		goto fail1;
	}

	if (flagsp != NULL) {
		if (req.emr_out_length_used >= MC_CMD_WORKAROUND_EXT_OUT_LEN)
			*flagsp = MCDI_OUT_DWORD(req, WORKAROUND_EXT_OUT_FLAGS);
		else
			*flagsp = 0;
	}

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}


	__checkReturn		efx_rc_t
efx_mcdi_get_workarounds(
	__in			efx_nic_t *enp,
	__out_opt		uint32_t *implementedp,
	__out_opt		uint32_t *enabledp)
{
	efx_mcdi_req_t req;
	uint8_t payload[MC_CMD_GET_WORKAROUNDS_OUT_LEN];
	efx_rc_t rc;

	(void) memset(payload, 0, sizeof (payload));
	req.emr_cmd = MC_CMD_GET_WORKAROUNDS;
	req.emr_in_buf = NULL;
	req.emr_in_length = 0;
	req.emr_out_buf = payload;
	req.emr_out_length = MC_CMD_GET_WORKAROUNDS_OUT_LEN;

	efx_mcdi_execute(enp, &req);

	if (req.emr_rc != 0) {
		rc = req.emr_rc;
		goto fail1;
	}

	if (implementedp != NULL) {
		*implementedp =
		    MCDI_OUT_DWORD(req, GET_WORKAROUNDS_OUT_IMPLEMENTED);
	}

	if (enabledp != NULL) {
		*enabledp = MCDI_OUT_DWORD(req, GET_WORKAROUNDS_OUT_ENABLED);
	}

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}


#endif	/* EFSYS_OPT_MCDI */