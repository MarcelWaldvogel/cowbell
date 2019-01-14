# Cowbell — Ring the phone on motion

This is a variation on the [Heise project "Die Türklingel im Heimnetz"](https://www.heise.de/ct/ausgabe/2017-17-Die-Tuerklingel-im-Heimnetz-3787241.html). I would like to thank Andrijan Möcker and [c't](https://www.heise.de/ct/) for the inspiration.

The idea was
- to use it with a motion detector instead of a manuell bell button,
- to allow turning it off and on, and
- to make it simpler by not requiring an entire Asterisk server.

I utterly failed achieving the latter, assuming there would be SIP libraries
out there which would allow writing a simple call/answer/play/DTMF application
a breeze. So, I first had to write
[`flexosip`](https://github.com/MarcelWaldvogel/flexosip), which is still not
complete.

# Setup

- Create a SIP phone on your PBX. Make sure it will not answer calls from the outside.
  * On a Fritz!Box, this is Telephony→Devices→Add device→Phone→LAN/WLAN, then select one of your
    outgoing numbers (will probably not have to call out) and select the numbers it should receive
    calls on (deselect all lines).  
    If you do not deselect all incoming lines, it will answer the phone for that line and thus
    - cause confusion
    - allow remote (unauthorized) users to update the settings
- Setup a Raspberry Pi with a motion detector
  * I used a [3.3 V Passive Infrared Motion Detector](https://www.pollin.de/p/pir-bewegungsmelder-modul-daypower-hc-sr501-810590)
- Configure a Raspberry Pi to run *Cowbell*:
  * Install *Raspbian Stretch*
  * Download `cowbell` (`git clone https://github.com/MarcelWaldvogel/cowbell`)
  * `cd cowbell`
  * `git submodule init; git submodule update`
  * Install the dependencies (`apt install libexosip2-dev libc-ares-dev libortp-dev libsndfile1-dev`)
  * Download `inih` (`(cd flexosip && git clone https://github.com/benhoyt/inih)`)
  * Install `wiringPi` (`(cd wiringPi && ./build)`)
  * Copy `cowbell.ini.sample` to `/etc/` and configure it (instructions are inside)
  * Copy `cowbell.service` to `/etc/systemd/system/`, configure it, and actiate it with `systemctl daemon-reload; systemctl enable cowbell; systemctl start cowbell`
- Try it out
  * If `cowbell` is not run automatically yet, start it as `./cowbell` (after configuring GPIO; see `cowbell.ini.sample`).
  * By default, the trigger is disabled
  * Call the extension you registered for the SIP phone (on my Fritz!Box, this was `**620`)
  * When you hear the announcement, it has already been activated for
    one-shot action. But if you want, you may press one of the following buttons on your phone:
    - Press '0' to disable the trigger
    - Press '1' to activate for one-shot action
    - Press '5' to activate for the next 5 minutes
    - Press '9' to activate forever (until manually disabled, of course)
  * Trigger the motion sensor (or press *Return* on the interactively-run program)
  * It should call the designated extension and ring for the designated duration
    (answering or declining the incoming call on the ringing phone will result in termination of the call)

# Operation

* By default, the system is asleep, not watching for an event (motion detector) to fire
* Calling in
  - Plays a greeting
  - Does either
    - When the system is set to watch for an event, it plays the current status (once, 5min, forever) and accepts mode change
    - When the system is not set to watch for an event (the typical case), it enables "once" mode and accepts mode change
  - Waits for mode change via DTMF digits (repeat as often as you want). You can press the digits at any time, you do not need to wait for the end of the recording.
    - '0' disable watching
    - '1' watch once
    - '5' watch for 5 minutes
    - '9' watch until further notice (i.e., forever)
  - Waits for you to hang up
* When the alert fires
  - Calls the number and then
    - If the user accepts/declines the call, terminates the call (on most phones, it is better to decline the call right away, because otherwise, the phone will play several seconds of busy signal before returning to the main menu)
    - If the user does not do that, terminate the ringing after the number of seconds you specified
  - Returns to watching, if modes '5' or '9' were active
