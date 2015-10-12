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
 * kernel_control.c
 *
 * Functions to solve kernel symbols
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

#include "kernel_control.h"

#include <sys/systm.h>
#include <sys/conf.h>
#include <mach/mach_types.h>
#include <libkern/libkern.h>
#include <sys/proc.h>
#include <kern/locks.h>
#include <sys/kern_control.h>
#include <sys/malloc.h>

#include "logging.h"
#include "kernel_symbols.h"

#define enable_interrupts() __asm__ volatile("sti");
#define disable_interrupts() __asm__ volatile("cli");

static int ctl_connect(kern_ctl_ref ctl_ref, struct sockaddr_ctl *sac, void **unitinfo);
static errno_t ctl_disconnect(kern_ctl_ref ctl_ref, u_int32_t unit, void *unitinfo);
static int ctl_set(kern_ctl_ref ctl_ref, u_int32_t unit, void *unitinfo, int opt, void *data, size_t len);
static int process_userland_command(struct userland_event *event);

static struct kcontrol_info
{
    int max_clients;
    kern_ctl_ref ctl_ref;
    u_int32_t client_unit;
    kern_ctl_ref client_ctl_ref;
    boolean_t kern_ctl_registered;
} g_kcontrol;

int g_connection_to_userland;

// described at Network Kernel Extensions Programming Guide
static struct kern_ctl_reg g_ctl_reg = {
    BUNDLE_ID,            /* use a reverse dns name which includes a name unique to your comany */
    0,				   	  /* set to 0 for dynamically assigned control ID - CTL_FLAG_REG_ID_UNIT not set */
    0,					  /* ctl_unit - ignored when CTL_FLAG_REG_ID_UNIT not set */
    0,                    /* no privileged access required to access this filter */
    0,					  /* use default send size buffer */
    0,                    /* Override receive buffer size */
    ctl_connect,		  /* Called when a connection request is accepted */
    ctl_disconnect,		  /* called when a connection becomes disconnected */
    NULL,				  /* ctl_send_func - handles data sent from the client to kernel control - not implemented */
    ctl_set,			  /* called when the user process makes the setsockopt call */
    NULL			 	  /* called when the user process makes the getsockopt call */
};

#pragma mark -
#pragma mark Start and Stop functions

kern_return_t
start_comms(void)
{
    errno_t error = 0;
    // register the kernel control
    error = ctl_register(&g_ctl_reg, &g_kcontrol.ctl_ref);
    if (error == 0)
    {
        g_kcontrol.kern_ctl_registered = TRUE;
        return KERN_SUCCESS;
    }
    else
    {
        g_kcontrol.kern_ctl_registered = FALSE;
        ERROR_MSG("Failed to install kernel control!");
        return KERN_FAILURE;
    }
}

kern_return_t
stop_comms(void)
{
    /* can't unload kext if there are clients connected else it will lead to kernel panic */
    if (g_kcontrol.max_clients > 0)
    {
        ERROR_MSG("Clients still connected, can't remove.");
        return KERN_FAILURE;
    }
    errno_t error = 0;
    // remove kernel control
    error = ctl_deregister(g_kcontrol.ctl_ref);
    switch (error)
    {
        case 0:
        {
            return KERN_SUCCESS;
        }
        case EINVAL:
        {
            ERROR_MSG("The kernel control reference is invalid.");
            return KERN_FAILURE;
        }
        case EBUSY:
        {
            ERROR_MSG("The kernel control stil has clients attached.");
            return KERN_FAILURE;
        }
        default:
            return KERN_FAILURE;
    }
}

#pragma mark -
#pragma mark Kernel control functions

/*
 * called when a client connects to the socket
 * we need to store some info to use later
 */
static int
ctl_connect(kern_ctl_ref ctl_ref, struct sockaddr_ctl *sac, void **unitinfo)
{
    /* XXX: Client authentication should be added here :-) */
    /* this can be based on codesignature - there are some kernel functions available */
    
    /* we only accept a single client */
    if (g_kcontrol.max_clients > 0)
    {
        return EBUSY;
    }
    g_kcontrol.max_clients++;
    // store the unit id and ctl_ref of the client that connected
    // we will need these to queue data to userland
    g_kcontrol.client_unit = sac->sc_unit;
    g_kcontrol.client_ctl_ref = ctl_ref;
    g_connection_to_userland = 1;

    /*
     * send the current settings of all options we are able to modify
     */
    errno_t error = 0;
    struct userland_event event = {0};
    /* XXX: needs to be implemented, essentially find the kernel address that contains the status and retrieve it */
    event.sip_status = 1; /* XXX: return fixed 1 for now */
    error = ctl_enqueuedata(g_kcontrol.client_ctl_ref, g_kcontrol.client_unit, &event, sizeof(struct userland_event), CTL_DATA_EOR);
    if (error)
    {
        ERROR_MSG("Failed to send inactive event with error: %d.", error);
    }
    
    return 0;
}

/*
 * and when client disconnects
 */
static errno_t
ctl_disconnect(kern_ctl_ref ctl_ref, u_int32_t unit, void *unitinfo)
{
    // reset some vars
    if (g_kcontrol.max_clients > 0)
    {
        g_kcontrol.max_clients--;
    }
    g_kcontrol.client_unit = 0;
    g_kcontrol.client_ctl_ref = NULL;
    g_connection_to_userland = 0;
    return 0;
}

/*
 * receive data from userland to kernel
 */
static int
ctl_set(kern_ctl_ref ctl_ref, u_int32_t unit, void *unitinfo, int opt, void *data, size_t len)
{
    int error = 0;
    if (len == 0 || data == NULL)
    {
        ERROR_MSG("Invalid reply event?");
        return EINVAL;
    }
    
    /* XXX: add some kind of error checking to the input data? */
    switch (opt)
    {
        case 0:
        {
            struct userland_event *event = (struct userland_event*)data;
            /* act on operation and return result */
            return process_userland_command(event);
        }
        default:
            error = ENOTSUP;
            break;
    }
    return error;
}

static int
process_userland_command(struct userland_event *event)
{
    if (event == NULL)
    {
        return ENOTSUP;
    }
    
    switch (event->cmd)
    {
        case ENABLE_SIP:
        {
            disable_interrupts();
            _csr_set_allow_all(0);
            enable_interrupts();
            return 0;
        }
        case DISABLE_SIP:
        {
            disable_interrupts();
            _csr_set_allow_all(1);
            enable_interrupts();
            return 0;
        }
        default:
        {
            ERROR_MSG("Unknown command!");
            return ENOTSUP;
        }
    }
    return 0;
}
