# create a soundfile in AU format playing a sine wave
# of a given frequency, duration and volume
# tested with Python25   by vegaseat     29jan2008
#
# from http://www.daniweb.com/software-development/python/code/217024/creating-and-playing-a-sine-wave-sound-python# 
#
from struct import pack
from math import sin, pi
SAMPLE_RATE = 8000
def au_file(name='test.au', freq=440, dur=1000, vol=0.5):
    """
    creates an AU format audio file of a sine wave
    of frequency freq (Hz)
    for duration dur (milliseconds)
    at volume vol (max is 1.0)
    """
    fout = open(name, 'wb')
    nsamples = SAMPLE_RATE*dur/1000
    # header needs size, encoding=2, sampling_rate=8000, channel=1
    fout.write('.snd' + pack('>5L', 24, nsamples, 2, SAMPLE_RATE, 1))
    factor = 2 * pi * freq/SAMPLE_RATE
    # write data
    for seg in range(nsamples):
        # sine wave calculations
        sin_seg = sin(seg * factor)
        fout.write(pack('b', vol * 127 * sin_seg))
    fout.close()

# test the module ...
if __name__ == '__main__':
    import sys
    if len(sys.argv) != 3:
        print >> sys.stderr, "usage: python %s fname.au freq" % __file__
        sys.exit(-1)
    name = sys.argv[1]
    freq = float(sys.argv[2])

    au_file(name=name, freq=freq, dur=128, vol=0.8)
    
    # if you have Windows, you can test the audio file
    # otherwise comment this code out
    #import os
    #os.startfile('sound800.au')
