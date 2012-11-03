Files downloaded from:

http://en.wikipedia.org/wiki/Dual-tone_multi-frequency_signaling.

The originals were in Vorbis (.ogg) format.  I converted them to AU using ffmpeg.

for f in ~/Downloads/*.ogg; do ffmpeg -i $f -acodec pcm_s8 -ar 8000 test-data/$(basename "$f" .ogg).au; done
