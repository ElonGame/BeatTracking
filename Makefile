CC = gcc
CFLAGS = -lportaudio -lsndfile

# Install portaudio library, fftw3 along with double precision and libsndfile

spcup:
	$(CC) main.c -o spcup.out $(CFLAGS)
	./spcup.out 20 "output.wav"

clean:
	rm -rf *.out
	rm -rf *.raw
