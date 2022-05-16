/*
 * apc.c - APC (Aurora Personality Chip) driver for Sun Solaris 7.
 *
 * Copyright (C) 2022 Malte Dehling.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/systm.h>
#include <sys/modctl.h>
#include <sys/cmn_err.h>
#include <sys/ddi.h>
#include <sys/sunddi.h>


extern void (*idle_cpu)();


#define APC_IDLE_REG		0x00
#define APC_IDLE_REG_IDLE	0x01
#define APC_REG_SIZE		0x34


static int apc_attach(dev_info_t *dip, ddi_attach_cmd_t cmd);
static int apc_getinfo(dev_info_t *dip, ddi_info_cmd_t infocmd,
    void *arg, void **resultp);

static void apc_idle_cpu();


static struct dev_ops apc_ops = {
    DEVO_REV,           /* devo_rev */
    0,                  /* devo_refcnt */
    apc_getinfo,        /* devo_getinfo */
    nulldev,            /* devo_identify */
    nulldev,            /* devo_probe */
    apc_attach,         /* devo_attach */
    nodev,              /* devo_detach */
    nodev,              /* devo_reset */
    NULL,               /* devo_power */
    NULL,               /* devo_cb_ops */
    NULL                /* devo_bus_ops */
};

static struct modldrv modldrv = {
    &mod_driverops,     /* drv_modops */
    "APC driver v0.1",  /* drv_linkinfo */
    &apc_ops            /* drv_dev_ops */
};

static struct modlinkage modlinkage = {
    MODREV_1,           /* ml_rev */
    &modldrv, NULL      /* ml_linkage[] */
};


static struct {
    dev_info_t *apc_dip;
    caddr_t apc_reg_addr;
    ddi_acc_handle_t apc_reg_acc_handle;
} apc_state;

static ddi_device_acc_attr_t apc_reg_access_attr = {
    DDI_DEVICE_ATTR_V0,
    DDI_STRUCTURE_BE_ACC,
    DDI_STRICTORDER_ACC
};


int _init(void) {

#ifdef DEBUG
    cmn_err(CE_NOTE, "apc: _init()");
#endif

    return mod_install(&modlinkage);
}


int _info(struct modinfo *modinfop) {

#ifdef DEBUG
    cmn_err(CE_NOTE, "apc: _info()");
#endif

    return mod_info(&modlinkage, modinfop);
}


int _fini(void) {

#ifdef DEBUG
    cmn_err(CE_NOTE, "apc: _fini()");
#endif

    return mod_remove(&modlinkage);
}


static int apc_attach(dev_info_t *dip, ddi_attach_cmd_t cmd) {
    int ret;

    switch (cmd) {

    case DDI_ATTACH:
    
#ifdef DEBUG
        cmn_err(CE_NOTE, "apc_attach()");
#endif

        apc_state.apc_dip = dip;

        ret = ddi_regs_map_setup(apc_state.apc_dip, 0,
            &apc_state.apc_reg_addr, 0, APC_REG_SIZE,
            &apc_reg_access_attr, &apc_state.apc_reg_acc_handle);

        if (ret != DDI_SUCCESS) {
            cmn_err(CE_WARN, "apc_attach(): ddi_regs_map_setup() failed");
            ret = DDI_FAILURE;
            break;
        };

        idle_cpu = apc_idle_cpu;

        break;
    
    default:
        cmn_err(CE_WARN, "apc_attach(): cmd not implemented");
        ret = DDI_FAILURE;
    };

    return ret;
}


static int apc_getinfo(dev_info_t *dip, ddi_info_cmd_t info_cmd,
    void *arg, void **resultp)
{
    int ret;

#ifdef DEBUG
    cmn_err(CE_NOTE, "apc_getinfo()");
#endif

    switch (info_cmd) {

    case DDI_INFO_DEVT2DEVINFO:
        *resultp = apc_state.apc_dip;
        ret = DDI_SUCCESS;
        break;

    case DDI_INFO_DEVT2INSTANCE:
        *resultp = 0;
        ret = DDI_SUCCESS;
        break;

    default:
        cmn_err(CE_WARN, "apc_getinfo(): info_cmd not implemented");
        ret = DDI_FAILURE;
    };

    return ret;
}


static void apc_idle_cpu() {
    uint8_t apc_idle_reg;

    apc_idle_reg = ddi_get8(apc_state.apc_reg_acc_handle,
        (uint8_t *)apc_state.apc_reg_addr + APC_IDLE_REG);

    ddi_put8(apc_state.apc_reg_acc_handle,
        (uint8_t *)apc_state.apc_reg_addr + APC_IDLE_REG,
        apc_idle_reg | APC_IDLE_REG_IDLE);
}
