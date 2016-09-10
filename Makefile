CC = gcc
CFLAGS = -I/usr/include/cairo/ -L/usr/local/lib -lportaudio -lsndfile -lm -lfftw3f -lcairo

# Install portaudio library, fftw3 along with double precision and libsndfile and ffmpeg for conversion to required audio type and libcairo for spectogram image generation.

%.o:%.c %.h
	$(CC)  -c -o $@ $< $(CFLAGS)

spcup: main.o spectogram.o spectrum.o window.o algorithms.h callbacks.h common.h spectrum.h spectogram.h window.h
	$(CC) window.o spectrum.o spectogram.o main.o -o spcup.out $(CFLAGS)
	./spcup.out 0 20 "2.flac" "output.wav"

convert:
	ffmpeg -i "1.flac" -ac 1 "2.flac"

clean:
	rm -rf *.out
	rm -rf *.o
	rm -rf *.raw
