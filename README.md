# omsi_basic

A BASIC system for the PDP-8's OS/8 operating system written by Don Baccus
and Gerd Hoeren.  Wayne Davison wrote the floating point package.

Copyright 1972 in the published sources but we started in 1971 and gave
a talk at DECUS in Atlanta in spring of 1971. 

I hauled around a couple of old DECTapes with me through various moves,
including one with a non-release version of OMSI BASIC.

Many thanks to Josh Dersch for reading them for me.  The OMSI BASIC tape
was 40 years old when he read it!  DECTape never dies.

Unfortunately being a non-release version there were some bugs still that
caused it to crash for various reasons.  I've been slowly working through
fixing them.  I also found a bug in the Mac OS/X PDP-8/e simulator, where
some microprogrammed instructions were not working properly.  I fixed them
and they've been incorporated in the last version of the simulator.

If the EAE is available the floating point package uses it, and seems to
speed up compuation-intensive programs by about 30% when timed in "real
PDP-8 speed" mode.  It sets the EAE to Mode A as it was never extended to
use Mode B if it was available.

There appears to be no surviving OMSI BASIC documentation though I've
been slowly writing up a simple language guide and guide to the editor
(which is really quite slick, mostly written by Gerd).



