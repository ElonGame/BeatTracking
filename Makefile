CC = gcc
CFLAGS = -lportaudio -lsndfile -lm -lfftw3

# Install portaudio library, fftw3 along with double precision and libsndfile

spcup: algorithms.c
	$(CC) -g main.c -o spcup.out $(CFLAGS)
	./spcup.out 20 "output.wav"

clean:
	rm -rf *.out
	rm -rf *.raw
