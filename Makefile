CC = gcc
CFLAGS = -lportaudio

# Install portaudio library, fftw3 along with double precision

spcup:
	$(CC) main.c -o spcup.out $(CFLAGS)
	./spcup.out 20 "output.raw"

clean:
	rm -rf *.out
	rm -rf *.raw
