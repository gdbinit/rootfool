/*
 *                                ___        .-.                        ___
 *                               (   )      /    \                     (   )
 *  ___ .-.      .--.     .--.    | |_      | .`. ;    .--.     .--.    | |
 * (   )   \    /    \   /    \  (   __)    | |(___)  /    \   /    \   | |
 *  | ' .-. ;  |  .-. ; |  .-. ;  | |       | |_     |  .-. ; |  .-. ;  | |
 *  |  / (___) | |  | | | |  | |  | | ___  (   __)   | |  | | | |  | |  | |
 *  | |        | |  | | | |  | |  | |(   )  | |      | |  | | | |  | |  | |
 *  | |        | |  | | | |  | |  | | | |   | |      | |  | | | |  | |  | |
 *  | |        | '  | | | '  | |  | ' | |   | |      | '  | | | '  | |  | |
 *  | |        '  `-' / '  `-' /  ' `-' ;   | |      '  `-' / '  `-' /  | |
 * (___)        `.__.'   `.__.'    `.__.   (___)      `.__.'   `.__.'  (___)
 *
 * RootFool (read ROTFL)
 *
 * Created by Pedro Vilaça on 06/10/15.
 * pedro@sentinelone.com - http://www.sentinelone.com
 * reverser@put.as - https://reverse.put.as
 *
 * Copyright (c) 2015 Sentinel One. All rights reserved.
 *
 * rootfool.c
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <mach/mach_types.h>
#include "logging.h"
#include "kernel_symbols.h"
#include "kernel_control.h"

/* global variables */
/* structure to hold kernel headers info we use to solve symbols */
struct kernel_info g_kinfo;

extern const int version_major;
extern const int version_minor;

kern_return_t rootfool_start(kmod_info_t * ki, void *d);
kern_return_t rootfool_stop(kmod_info_t *ki, void *d);

kern_return_t rootfool_start(kmod_info_t * ki, void *d)
{
    if (version_major != 15)
    {
        ERROR_MSG("This kext only supports El Capitan.");
        return KERN_NOT_SUPPORTED;
    }

    /* initialize structure with kernel information to solve symbols */
    if (init_kernel_info() != KERN_SUCCESS)
    {
        /* in case of failure buffers are freed inside */
        ERROR_MSG("Failed to init kernel info structure!");
        return KERN_FAILURE;
    }
    
    SOLVE_KERNEL_SYMBOL("_csr_set_allow_all", _csr_set_allow_all)

    if (start_comms() != KERN_SUCCESS)
    {
        return KERN_FAILURE;
    }

    return KERN_SUCCESS;
}

kern_return_t rootfool_stop(kmod_info_t *ki, void *d)
{
    if (stop_comms() != KERN_SUCCESS)
    {
        return KERN_FAILURE;
    }
    
    if (cleanup_kernel_info() != KERN_SUCCESS)
    {
        return KERN_FAILURE;
    }
    
    return KERN_SUCCESS;
}
