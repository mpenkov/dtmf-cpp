"""Generate a sequence of DTMF tones."""

#
# http://docs.python.org/library/struct.html
#
from struct import pack
from math import sin, pi

ROW_FREQ = (697, 770, 852, 941)
COL_FREQ = (1209, 1336, 1477, 1633)

def create_parser():
    """Create an object to use for the parsing of command-line arguments."""
    from optparse import OptionParser
    usage = "usage: %s \"1 2 3 4 5 6 7 8 9\" filename.au [options]" % __file__
    parser = OptionParser(usage)
    parser.add_option(
            "--sample-rate",
            "-s",
            dest="sample_rate",
            default=8000,
            type="int",
            help="Use the specified sample rate in Hz")
    parser.add_option(
            "--volume",
            "-v",
            dest="volume",
            default=0.5,
            type="float",
            help="Specify signal volume in the range [0,1]")
    parser.add_option(
            "--duration",
            "-d",
            dest="duration",
            default=40,
            type="int",
            help="Specify the tone duration in ms")
    return parser

def tone2(f1, f2, sample_rate, dur):
    """
    Generate a tone composed of two frequencies, f1 and f2, of the
    specified sample rate (Hz), duration (ms) and volume.
    """
    #
    # Check that we're using valid DTMF frequencies.
    #
    assert f1 in ROW_FREQ
    assert f2 in COL_FREQ
    nsamples = sample_rate*dur/1000
    factor1 = 2*pi*f1/sample_rate
    factor2 = 2*pi*f2/sample_rate
    return [ (sin(x*factor1) + sin(x*factor2))/2 for x in range(nsamples) ]

def silence(sample_rate, dur):
    """Generate silence of the specified duration, in ms."""
    return [ 0 for i in range(sample_rate*dur/1000) ]

def save_au(fname, samples, sample_rate, vol):
    """Save the samples to the specified file in AU format."""
    fout = open(fname, "wb")
    nsamples = len(samples)
    # header needs size, encoding=2, sampling_rate=8000, channel=1
    fout.write(".snd" + pack(">5L", 24, nsamples, 2, sample_rate, 1))
    for y in samples:
        fout.write(pack("b", vol * 127 * y))
    fout.close()

def main():
    parser = create_parser()
    options, args = parser.parse_args()
    if len(args) != 2:
        parser.error("invalid number of arguments")

    #
    # Don't support frequencies below 4KHz
    # 
    assert options.sample_rate >= 4000

    #
    # http://en.wikipedia.org/wiki/Dual-tone_multi-frequency_signaling#Keypad
    #
    tones = { "1" : tone2(697, 1209, options.sample_rate, options.duration), 
              "2" : tone2(697, 1336, options.sample_rate, options.duration),
              "3" : tone2(697, 1477, options.sample_rate, options.duration),
              "A" : tone2(697, 1633, options.sample_rate, options.duration),
              "4" : tone2(770, 1209, options.sample_rate, options.duration),
              "5" : tone2(770, 1336, options.sample_rate, options.duration),
              "6" : tone2(770, 1477, options.sample_rate, options.duration),
              "B" : tone2(770, 1633, options.sample_rate, options.duration),
              "7" : tone2(852, 1209, options.sample_rate, options.duration),
              "8" : tone2(852, 1336, options.sample_rate, options.duration),
              "9" : tone2(852, 1477, options.sample_rate, options.duration),
              "C" : tone2(852, 1633, options.sample_rate, options.duration),
              "*" : tone2(941, 1209, options.sample_rate, options.duration),
              "0" : tone2(941, 1336, options.sample_rate, options.duration),
              "#" : tone2(941, 1477, options.sample_rate, options.duration),
              "D" : tone2(941, 1633, options.sample_rate, options.duration),
              " " : silence(options.sample_rate, options.duration) }
    #
    # for convenient command-line generation
    #
    tones["S"] = tones["*"]
    tones["H"] = tones["#"]

    samples = list()
    for ch in args[0]:
        samples += tones[ch]
    save_au(args[1], samples, options.sample_rate, options.volume)

if __name__ == "__main__":
    main()
