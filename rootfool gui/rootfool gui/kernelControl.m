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
 * RootFool GUI (read ROTFL)
 *
 * Created by Pedro Vilaça on 06/10/15.
 * pedro@sentinelone.com - http://www.sentinelone.com
 * reverser@put.as - https://reverse.put.as
 *
 * Copyright (c) 2015 Sentinel One. All rights reserved.
 *
 * kernelControl.m
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

#import <Cocoa/Cocoa.h>
#import "AppDelegate.h"

#import "kernelControl.h"
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/kern_control.h>
#include <sys/kern_event.h>
#include <sys/sys_domain.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <mach/mach.h>
#include <mach/mach_types.h>
#include <mach/i386/thread_status.h>
#include <mach/mach_vm.h>

#include "shared_data.h"

#define ERROR_MSG(fmt, ...) NSLog(@"[ERROR] " fmt "", ## __VA_ARGS__)
#define DEBUG_MSG(fmt, ...) NSLog(@"[DEBUG] " fmt "", ##  __VA_ARGS__)

@interface KernelControl()
{
    dispatch_queue_t _queue;
    dispatch_source_t _readSource;
}
@end

@implementation KernelControl

-(int)connectToKext
{
    _kSock = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
    if (_kSock < 0)
    {
        ERROR_MSG("Failed to create kernel control socket. Error %d (%s).", errno, strerror(errno));
        return -1;
    }
    
    /* the control ID is dynamically generated so we must obtain sc_id using ioctl */
    memset(&_ctl_info, 0, sizeof(_ctl_info));
    strncpy(_ctl_info.ctl_name, BUNDLE_ID, MAX_KCTL_NAME);
    _ctl_info.ctl_name[MAX_KCTL_NAME-1] = '\0';
    if ( ioctl(_kSock, CTLIOCGINFO, &_ctl_info) == -1 )
    {
        ERROR_MSG("Failed to retrieve bundle information! Error %d (%s).", errno, strerror(errno));
        /* if we can't get the bundle info it means the kernel socket is not up, */
        /* so close the socket else check above will be always true */
        close(_kSock);
        _kSock = -1;
        return -1;
    }
    bzero(&_sc, sizeof(struct sockaddr_ctl));
    _sc.sc_len = sizeof(struct sockaddr_ctl);
    _sc.sc_family = AF_SYSTEM;
    _sc.ss_sysaddr = AF_SYS_CONTROL;
    _sc.sc_id = _ctl_info.ctl_id;
    _sc.sc_unit = 0;
    if ( connect(_kSock, (struct sockaddr*)&_sc, sizeof(_sc)) != 0 )
    {
        _kSock = -1;
        ERROR_MSG("Failed to connect to kernel control socket! Error %d (%s).", errno, strerror(errno));
        return -1;
    }
    
    DEBUG_MSG("Connected to kernel socket...");
    
    /* create the handler to act when data is received on the socket */
    _queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    _readSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, _kSock, 0, _queue);
    dispatch_source_set_event_handler(_readSource, ^{
        [self kernelNotifications];
    });
    dispatch_resume(_readSource);
    
    return 0;
}

-(int)disconnectFromKext
{
    if (_kSock != -1)
    {
        DEBUG_MSG("Disconnecting from kernel socket...");
        close(_kSock);
        _kSock = -1;
        dispatch_source_cancel(_readSource);
    }
    return 0;
}

/* the function responsible for reading events from the kernel and replying */
-(void)kernelNotifications
{
    ssize_t n = 0;
    struct userland_event data = {0};
    /* we managed to read something, check what it is */
    n = recv(_kSock, &data, sizeof(struct userland_event), 0);
    if (n < 0)
    {
        ERROR_MSG("Recv error.");
    }
    else if (n < sizeof(struct userland_event))
    {
        ERROR_MSG("Received smaller buffer than expected from kernel.");
    }
    else
    {
        /* process the current settings and set options accordingly */
        DEBUG_MSG("Received connect kernel event");
        /* get the app delegate so we can update the buttons */
        AppDelegate *delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
        data.sip_status = 1;
        [delegate updateOptionButtons:data];
    }
}

-(int)sendCommand:(enum commands)command
{
    if (_kSock == -1)
    {
        NSLog(@"Can't send command, not connected!");
        return -1;
    }
    struct userland_event event = {0};
    event.cmd = command;
    
    int ret = setsockopt(_kSock, SYSPROTO_CONTROL, 0, (void*)&event, (socklen_t)sizeof(struct userland_event));
    if (ret)
    {
        NSLog(@"[ERROR] Kernel command execution failed!");
        return -1;
    }
    return 0;
}

@end
