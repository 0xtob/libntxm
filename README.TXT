
              libntxm v0.2 - An XM Player Library for the Nintendo DS
                                                             by 0xtob

                                    http://code.google.com/p/libntxm/

WHAT'S THIS?
------------

  libntxm (NitroTracker XM library) is a library for playing XM music
  in Nintendo DS homebrew software. It's very CPU friendly, because
  it uses the DS sound hardware directly instead of doing software
  mixing.


WHO SHOULD USE THIS?
--------------------

  This library is intended for use in homebrew games and applications.
  As it does not support the complete XM format yet, other libraries
  may still be preferrable. So, use it only if you can live with the
  limitations listed below.


LIMITATIONS
-----------

  libntxm does not yet support all instrument settings and effects.
  In order to keep the lists short, here are the _unsupported_
  instrument settings and the _supported_ effects:

  Unsupported instrument settings:
    - Panning
    - Fade out
    - Vibrato
    - Envelope sustain point
    - Envelope loop

  Supported effects:
    - Arpeggio
    - Set loop
    - Set volume
    - Pattern break
    - Note cut
    - Volume slide
    - Set speed and tempo


INSTALLATION
------------

  libntxm was developed using devkitARM r21 and libnds 20071023. It will
  probably not work with earlier versions of these.

  - Make sure you have the above mentioned versions of devkitARM and libnds
    or higher, and make sure libfat is installed.

  - Copy the include/ntxm folder to your libnds include path, such that
    your include path has a subdirectory named ntxm that contains the
    headers.

  - Copy the contents of the lib folder (ntxm7.a and ntxm9.a) to your
    libnds library path.


USAGE
-----

  The provided example explains how to use the library. You need to
  have a custom ARM7 binary. Make sure to link to the respective
  libraries in the arm7 and arm9 makefiles. For testing the example,
  you need to have a file called test.xm on your card.


NOTES
-----

  libntxm uses some memory behind the libnds IPC struct for exchanging
  messages. Make sure that you are not already using that memory area
  for your own purposes.


DEVELOPMENT
-----------

  You want to help by adding new features to the library? Great!
  If you want to help out, please drop me a line first, so we can avoid
  collisions :)


VERSION HISTORY
---------------

 v0.2 (2008-04-13)
  - Relicensed under Noncommercial zLib License / GPLv3
  - Now using the same codebase as NitroTracker for easier integration

 v0.1 (2007-04-15)
  - Initial Release

CONTACT
-------

  e-Mail: me@nitrotracker.tobw.net
  IRC: _0xtob in #dsdev on Blitzed



Happy coding!
     - 0xtob

