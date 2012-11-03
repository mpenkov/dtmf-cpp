"""Plot the magnitudes of each frequency detected at each frame.

usage: detect-au.out file.au | python plot_T.py

Requires that the detector was built with make debug.
"""
import sys
import csv
import matplotlib.pyplot as plt

def from_fp(fp):
    """Convert from fixed-point to float."""
    return float(fp)/2**15

#
# ignore the first and second columns -- the frame number and the tone, 
# respectively.
#
coeff = list()
for row in csv.reader(sys.stdin, delimiter=" "):
    if len(row) != 18+1: # last column is empty string, ignore it
        continue
    coeff.append(map(from_fp, row[:-1])) 
xval = range(len(coeff))

leg = { 0: ("r",        "o", "706Hz"),  # row freq
        1: ("g",        "o", "784Hz"),
        2: ("b",        "o", "863Hz"),
        3: ("c",        "o", "941Hz"),
        4: ("m",        "s", "1176Hz"), # col freq
        5: ("y",        "s", "1333Hz"),
        6: ("orange",   "s", "1490Hz"),
        7: ("pink",     "s", "1547Hz"),
        8: ("black",    "o", "1098Hz"), # unknown
        9: ("black",    "s", "549Hz"),
       10: ("r",        "+", "78Hz"),   # harmonics
       11: ("g",        "+", "235Hz"),
       12: ("b",        "+", "314Hz"),
       13: ("c",        "+", "392Hz"),
       14: ("m",        "x", "2039Hz"),
       15: ("y",        "x", "2510Hz"),
       16: ("orange",   "x", "2980Hz"),
       17: ("pink",     "x", "3529Hz") }
        
for i, col in enumerate(zip(*coeff)):
    c, m, l = leg[i]
    plt.plot(xval, col, color=c, marker=m, label=l);
plt.xlabel("frame number")
plt.ylabel("T")
plt.legend(bbox_to_anchor=(1.05, 1), loc=2, borderaxespad=0.)
plt.subplots_adjust(left=0.1, right=0.75, top=0.9, bottom=0.1)
plt.show()
