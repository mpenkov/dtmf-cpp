dtmf-cpp
========

C++ DTMF detector and generator classes

The original code was written by Plyashkevich Viatcheslav <plyashkevich@yandex.ru>
and is available in its original form at http://sourceforge.net/projects/dtmf/

Main features:

- Portable fixed-point implementation
- Detection of DTMF tones from 8KHz PCM8 signal

Installation
------------

    git clone https://github.com/mpenkov/dtmf-cpp.git
    cd dtmf-cpp
    make
    bin/detect-au.out test-data/Dtmf0.au