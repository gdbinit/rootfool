Rootfool (read ROTFL)

(c) fG! - 2015 - reverser@put.as - https://reverse.put.as - http://www.sentinelone.com

A small kernel extension and Cocoa GUI to dynamically disable and enable the System Integrity Protection (SIP) introduced in El Capitan (obviously this is El Capitan only).

You need to manually load and unload the kernel extension. You will also need a kernel extension code signing certificate (issued by Apple on request, if you are lucky it's approved).

No major hacks applied, it uses an internal Apple function that enables/disabled SIP. It could be achieved by manually finding the same variable, but at least for now calling that function is good enough.

Useful for developers to disable/enable SIP for testing without having to reboot and use csrutil.

Might add other dynamic features in the future (mostly developer boot arguments that could be enabled/disabled runtime without reboots).

This is not a security hole in El Capitan, if you are able to run kernel level code SIP will not protect the system. This is by design and acceptable under SIP threat model.

Another GUI should be available soon, co-worker Julien-Pierre made one, I just need to try it and integrate into the repo.

And thanks to Assaf for the rootfool name, way better than the original rootful ;-)

Enjoy,

fG!

P.S.: 10.11.4 update removed csr_set_allow_all() function used to enable/disable SIP. It means this code does not work on El Capitan 10.11.4 or newer versions when released.