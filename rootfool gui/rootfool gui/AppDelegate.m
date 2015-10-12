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
 * AppDelegate.m
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

#import "AppDelegate.h"
#import "shared_data.h"

@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    [NSApplication sharedApplication];
    // Insert code here to initialize your application
    _kc = [KernelControl new];
    if ( [_kc connectToKext] == 0 )
    {
        [_statusField setStringValue:@"Connected to kext!"];
        [_disconnectButton setEnabled:YES];
        [_connectButton setEnabled:NO];
        NSImage *connectedImage = [NSImage imageNamed:@"status-available.tiff"];
        [_status setImage:connectedImage];
        [self enableOptionButtons];
    }
    else
    {
        [_statusField setStringValue:@"Failed to connect to kext!"];
        [_disconnectButton setEnabled:NO];
        [_connectButton setEnabled:YES];
        [self disableOptionButtons];
    }
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    // Insert code here to tear down your application
    /* XXX: restore default? */
    [_kc disconnectFromKext];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

/* 
 * enable all option buttons
 * called when connection to kext is made
 */
-(void)enableOptionButtons
{
    [_sipButton setEnabled:YES];
}

/*
 * disable all option buttons
 * if there's no connection to kext there's no point in user be able to change options
 */
-(void)disableOptionButtons
{
    [_sipButton setEnabled:NO];
}

/*
 * update the state of the buttons when connection is made to kext
 * kext will send us the state of all settings when we connect to it
 */
-(void)updateOptionButtons:(struct userland_event)event
{
    /* process the current kernel settings and set buttons accordingly */
    if (event.sip_status == 0)
    {
        [_sipButton setState:NSOffState];
    }
    else if (event.sip_status == 1)
    {
        [_sipButton setState:NSOnState];
    }
}

- (IBAction)pressConnect:(id)sender
{
    if ( [_kc connectToKext] == 0 )
    {
        [_statusField setStringValue:@"Connected to kext!"];
        [_disconnectButton setEnabled:YES];
        [_connectButton setEnabled:NO];
        NSImage *connectedImage = [NSImage imageNamed:@"status-available.tiff"];
        [_status setImage:connectedImage];
        [self enableOptionButtons];
    }
    else
    {
        [_statusField setStringValue:@"Failed to connect to kext!"];
        [_disconnectButton setEnabled:NO];
        [self disableOptionButtons];
    }
}

- (IBAction)pressDisconnect:(id)sender
{
    [_kc disconnectFromKext];
    [_statusField setStringValue:@"Disconnected from kext!"];
    [_disconnectButton setEnabled:NO];
    [_connectButton setEnabled:YES];
    NSImage *connectedImage = [NSImage imageNamed:@"status-away.tiff"];
    [_status setImage:connectedImage];
    [self disableOptionButtons];
}

- (IBAction)takeSIP:(id)sender
{
    if ( [[sender cell] state] == NSOnState)
    {
        [_kc sendCommand:ENABLE_SIP];
        [_statusField setStringValue:@"Enabled SIP."];
    }
    else if ( [[sender cell] state] == NSOffState)
    {
        [_kc sendCommand:DISABLE_SIP];
        [_statusField setStringValue:@"Disabled SIP."];
    }
}

@end
