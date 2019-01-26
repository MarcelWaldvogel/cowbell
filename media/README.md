# Image files

- *[bell-256.png](./bell-256.png)* and *[bell-256.jpeg](./bell-256.jpeg)*:
  A bell in 256x256, suitable for a Fritz!Fon image, to be associated with
  the Cowbell phone number in the Fritz!Box address book.  
  Please note that due to Fritz!Fon (or Fritz!Box?) weirdness (bug?), that
  images on internal phone numbers will only be shown when dialing out, not
  when a call is received. This mostly defeats the purpose of having that
  image. Maybe someone has an idea how to fix this?

# Audio ringtone

- *[dingdong.mp3](./dingdong.mp3)*: A sample ringtone, to be associated with
  the internal phone number used, so that the Cowbell will ring nicely.  
  I was unable to associate a ringtone with just a single internal number
  in the Fritz!Box/Fritz!Fon combo, so I use it for all the internal calls
  (in my setting, the Fritz!Fon and the Cowbell are the only two internal
  numbers).

# Audio voice prompts

All audio files should be created with 8 or (preferably) 16 kHz sampling rate.

- *[hello.ogg](./hello.ogg)*: The welcome message that will be played after
  answering the phone. All the other messages will be played afterwards or
  in response to the appropriate DTMF input.
- *[on.ogg](./on.ogg)*: Played if this call has resulted in switching from
  off to one-shot mode.
- *[1x.ogg](./1x.ogg)*: Played when one-shot mode was already active when
  calling or when activated by the `1` key.
- *[5min.ogg](./5min.ogg)*: Played when 5-minute mode was already active when
  calling or when activated by the `5` key.
- *[perm.ogg](./perm.ogg)*: Played when permanent mode was already active when
  calling or when activated by the `9` key.
- *[off.ogg](./off.ogg)*: Played when off mode was activated by the `0` key.
