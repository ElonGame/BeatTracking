CC = gcc
CFLAGS =  -lportaudio -lsndfile -lm -lfftw3 -g

# Install portaudio library, fftw3 along with double precision and libsndfile and ffmpeg for conversion to required audio type.
# Using numpy and matplotlib in python for visualisation of the various obtained grams and graphs.
%.o:%.c %.h
	$(CC)  -c -o $@ $< $(CFLAGS)

spcup: main.o spectrum.o window.o moving_average.o novelty_curve.o restrict_bpm.o find_mode.o algorithms.h callbacks.h common.h spectrum.h window.h novelty_curve.h moving_average.h restrict_bpm.h find_mode.h
	$(CC) window.o spectrum.o moving_average.o novelty_curve.o main.o -o spcup.out $(CFLAGS)
	./spcup.out 0 180 "2.flac" "output.wav"

convert:
	ffmpeg -i "1.flac" -ac 1 "2.flac"

clean:
	rm -rf *.out
	rm -rf *.o
	rm -rf *.raw
	rm -rf *.txt
