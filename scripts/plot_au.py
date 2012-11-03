"""Plot the waveform of the signal in the specified AU file."""

import sys
from struct import unpack, calcsize

def read_au(fname):
    """Read the AU file.  Return the sampling rate and samples in a list."""
    fin = open(fname, "rb")
    payload = fin.read()

    assert payload[:4] == ".snd"
    header_len = calcsize(">5L")
    _, nsamples, encoding, sample_rate, _ = unpack(">5L", payload[4:4+header_len])
    samples = unpack("%db" % nsamples, payload[4+header_len:])
    return sample_rate, samples

if __name__ == "__main__":
    import matplotlib.pyplot as plt
    sample_rate, samples = read_au(sys.argv[1])
    plt.title("%s (%dHz)" % (sys.argv[1], sample_rate))
    plt.plot(range(len(samples)), samples)
    plt.xlabel("sample number")
    plt.ylabel("value")
    plt.show()
