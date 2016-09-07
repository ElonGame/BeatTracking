CC = gcc
CFLAGS = -lportaudio -lsndfile -lm -lfftw3

# Install portaudio library, fftw3 along with double precision and libsndfile

spcup: algorithms.c
	$(CC) -g main.c -o spcup.out $(CFLAGS)
	./spcup.out 0 20 "2.flac" "output.wav"

convert:
	ffmpeg -i "1.flac" -ac 1 "2.flac"

clean:
	rm -rf *.out
	rm -rf *.raw
