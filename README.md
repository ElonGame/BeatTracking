# spcup
Beat Tracking for IEEE SP Cup 2016

#Instructions
Install portaudio library along with alsa lib, fftw3/kissfft along with double and single precision and libsndfile

##Ubuntu packages (install using apt-get install)
- libportaudio2 libportaudio2 portaudio19-dev libsndfile1 libsndfile1-dev libfftw3-bin libfftw3-dev

- ffmpeg to convert audio to required format. (mono, uncompressed wav or flac)

- python3 along with python3-numpy and python3-numpy for visualisation

Use "make" to compile and run and "make clean" to delete executable and output file.
Run the python scripts with python3 <script-name> to visualise variouys graphs and grams.

#Current Status:
- Records audio in CD quality from available microphone and plays it back.
- The recorded data is stored in data.recordedSamples and available for further processing.
- Can also read from a uncompressed (flac,wav) single channel audio file and write output to a file.
- Computes Spectogram

#Objective
- Implement restricted PLP curve based beat Tracking
- audio -> Spectogram (via STFT) -> logarithmic compression -> positive differential along rows -> accumulation along columns -> Novelty curve
- Novelty curve -> subtract local average -> normalize -> convert to tempogram

- Convert to tempogram (time vs BPM) by finding locally periodic patterns using fourier analysis.
- Find optimised local periodicity kernel and accumulate maximising kernels.

- To this apply halfwave rectification to obtain Predominant Local Pulse (PLP) curve.

- To further optimise beat tracking and select a specific level (from measure, tactus, and tactum) restrict the tempogram to a specific BPM range before finding local periodicity kernel.
