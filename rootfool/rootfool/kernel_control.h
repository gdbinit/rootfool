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
 * kernel_control.h
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

#ifndef __rootfool__kernel_control__
#define __rootfool__kernel_control__

#include <mach/mach_types.h>
#include <libkern/libkern.h>
#include <sys/vnode.h>

#include "shared_data.h"

kern_return_t start_comms(void);
kern_return_t stop_comms(void);

#endif /* defined(__rootfool__kernel_control__) */
