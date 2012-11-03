"""
Implementation of the Goertzel algorithm.

Based on notes from:

https://sites.google.com/site/hobbydebraj/home/goertzel-algorithm-dtmf-detection
"""
from math import pi, cos

from plot_au import read_au
from tonegen import ROW_FREQ, COL_FREQ

#
# Mapping of dual tones (lo, hi) to their symbols.
#
FREQ_TO_TONE = dict()
FREQ_TO_TONE[ (697, 1209) ] = "1"
FREQ_TO_TONE[ (697, 1336) ] = "2"
FREQ_TO_TONE[ (697, 1477) ] = "3"
FREQ_TO_TONE[ (697, 1633) ] = "A"
FREQ_TO_TONE[ (770, 1209) ] = "4"
FREQ_TO_TONE[ (770, 1336) ] = "5"
FREQ_TO_TONE[ (770, 1477) ] = "6"
FREQ_TO_TONE[ (770, 1633) ] = "B"
FREQ_TO_TONE[ (852, 1209) ] = "7"
FREQ_TO_TONE[ (852, 1336) ] = "8"
FREQ_TO_TONE[ (852, 1477) ] = "9"
FREQ_TO_TONE[ (852, 1633) ] = "C"
FREQ_TO_TONE[ (941, 1209) ] = "*"
FREQ_TO_TONE[ (941, 1336) ] = "0"
FREQ_TO_TONE[ (941, 1477) ] = "#"
FREQ_TO_TONE[ (941, 1633) ] = "D"

def create_parser():
    """Create an object to use for the parsing of command-line arguments."""
    from optparse import OptionParser
    usage = "usage: %s filename.au [options]" % __file__
    parser = OptionParser(usage)
    parser.add_option(
            "--threshold",
            "-t",
            dest="threshold",
            default=1000,
            type="int",
            help="Specify the cutoff threshold")
    parser.add_option(
            "--buflen",
            "-b",
            dest="buflen",
            default=120,
            type="int",
            help="Specify the detection buffer length")
    return parser

class Goertzel:
    """This class implements DTMF detection by using the Goertzel algorithm."""
    def __init__(self, sample_rate, threshold):
        self.sample_rate = sample_rate
        self.threshold = threshold
        self.coeff = dict()
        
        for f in ROW_FREQ + COL_FREQ:
            theta = 2*pi/sample_rate * f
            self.coeff[f] = 2*cos(theta)
            #print f, theta, self.coeff[f]

    def process(self, samples):
        """Process a block of samples."""
        #
        # Calculate the Z-transforms for each frequency.
        #
        prev_prev = dict()
        prev = dict()
        for i,x in enumerate(samples):
            mag = dict()
            for f in ROW_FREQ + COL_FREQ:
                try:
                    p = prev[f]
                except KeyError:
                    p = 0
                try:
                    pp = prev_prev[f]
                except KeyError:
                    pp = 0

                output = x + self.coeff[f]*p - pp
                prev_prev[f] = p
                prev[f] = output

        for f in ROW_FREQ + COL_FREQ:
            mag[f] = prev_prev[f]**2 + prev[f]**2 - self.coeff[f]*prev_prev[f]*prev[f]

        #
        # Get the most likely tone candidate.
        #
        strong = sorted(mag, key=lambda f: mag[f], reverse=True)
        try:
            tone = FREQ_TO_TONE[tuple(sorted(strong[:2]))]
        except KeyError:
            tone = "."

        #
        # Reject the candidate if the magnitudes aren't strong enough.
        #
        mag1, mag2 = mag[strong[0]], mag[strong[1]]
        if not (mag1 > self.threshold and mag2 > self.threshold):
            tone = "."

        return tone, mag1, mag2

def main():
    parser = create_parser()
    options, args = parser.parse_args()
    if len(args) != 1:
        parser.error("invalid number of arguments")
    sample_rate, samples = read_au(args[0])
    print "%s: %dHz" % (args[0], sample_rate)
    goertzel = Goertzel(sample_rate, options.threshold)
    for i in range(0, len(samples) - options.buflen, options.buflen): 
        tone, mag1, mag2 = goertzel.process(samples[i:i+options.buflen])
        print "%8d %s %.2f %.2f" % (i, tone, mag1, mag2)

if __name__ == "__main__":
    import sys
    sys.exit(main())
