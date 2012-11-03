Utility Scripts
===============

These scripts are used for testing and debugging.  For full options, run:

    python scriptname.py --help

DTMF Tone Sequence Generator
----------------------------

To generate a sequence of DTMF tones:

    python tonegen.py "1 2 3 4 5 6 7 8 9 0 A B C D * #" test.au

You can then play back the generated file in any media player.

DTMF Detector
-------------

To detect DTMF tones in an audio file:

    python goertzel.py test.au

You will see output like:

    test.au: 8000Hz
       0 1 3439407.55 3377800.10
     120 1 3797085.89 3783684.74
     240 1 1528250.62 1507809.07
     360 . 0.00 0.00
     480 . 0.00 0.00
     600 2 1623529.03 1600404.12
     720 2 3504532.93 3498308.97
     840 2 3502300.73 3478325.32
    ...

The left-most column indicates the sample number of the first frame that got processed.
The second column shows the detected DTMF tone ("." means nothing was detected).
Currently, this simple detector doesn't apply any logic to the tones it detects -- it merely indicates
the tone detected at each frame.

Waveform Plotter
----------------

To quickly plot the waveform stored in an AU file:

    python plot_au.py test.au

This requires matplotlib.  Alternatively, use an editor like http://audacity.sourceforge.net/

Result:
![Alt text](https://raw.github.com/mpenkov/dtmf-cpp/master/scripts/plot_au.png)

Goertzel Output Plotter
-----------------------

If you build the detector in debug mode:

    cd ..
    make clean
    make debug

Then you can visualize the strength of each magnitude at each frame:

    ../bin/detect-au.out test.au | python plot_T.py

Result:
![Alt text](https://raw.github.com/mpenkov/dtmf-cpp/master/scripts/plot_T.png)
