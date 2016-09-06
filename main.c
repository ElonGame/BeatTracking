#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
// #include <complex.h>
#include <fftw3.h>
#include <portaudio.h>

// Does not work in ARM arch without compiled library. Not needed either. Only for testing
#include <sndfile.h>

#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (512)
#define NUM_CHANNELS    (1)
#define DITHER_FLAG     (0)

#define STFT_WINDOW_SIZE 2048
#define STFT_HOP_SIZE 1024

/* Select sample format. */
#if 1
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
// #elif 1
// #define PA_SAMPLE_TYPE  paInt16
// typedef short SAMPLE;
// #define SAMPLE_SILENCE  (0)
// #define PRINTF_S_FORMAT "%d"
// #elif 0
// #define PA_SAMPLE_TYPE  paInt8
// typedef char SAMPLE;
// #define SAMPLE_SILENCE  (0)
// #define PRINTF_S_FORMAT "%d"
// #else
// #define PA_SAMPLE_TYPE  paUInt8
// typedef unsigned char SAMPLE;
// #define SAMPLE_SILENCE  (128)
// #define PRINTF_S_FORMAT "%d"
#endif

typedef struct
{
    int          frameIndex;  /* Index into sample array. */
    int          maxFrameIndex;
    SAMPLE      *recordedSamples;
} paTestData;

#include "algorithms.c"

static int recordCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    paTestData *data = (paTestData*)userData;
    const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
    SAMPLE *wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    long framesToCalc;
    long i;
    int finished;
    unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

    (void) outputBuffer; /* Prevent unused variable warnings. */
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;

    if( framesLeft < framesPerBuffer )
    {
        framesToCalc = framesLeft;
        finished = paComplete;
    }
    else
    {
        framesToCalc = framesPerBuffer;
        finished = paContinue;
    }

    if( inputBuffer == NULL )
    {
        for( i=0; i<framesToCalc; i++ )
        {
            *wptr++ = SAMPLE_SILENCE;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = SAMPLE_SILENCE;  /* right */
        }
    }
    else
    {
        for( i=0; i<framesToCalc; i++ )
        {
            *wptr++ = *rptr++;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        }
    }
    data->frameIndex += framesToCalc;
    return finished;
}

static int playCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData )
{
    paTestData *data = (paTestData*)userData;
    SAMPLE *rptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    SAMPLE *wptr = (SAMPLE*)outputBuffer;
    unsigned int i;
    int finished;
    unsigned int framesLeft = data->maxFrameIndex - data->frameIndex;

    (void) inputBuffer; /* Prevent unused variable warnings. */
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;

    if( framesLeft < framesPerBuffer )
    {
        /* final buffer... */
        for( i=0; i<framesLeft; i++ )
        {
            *wptr++ = *rptr++;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        }
        for( ; i<framesPerBuffer; i++ )
        {
            *wptr++ = 0;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = 0;  /* right */
        }
        data->frameIndex += framesLeft;
        finished = paComplete;
    }
    else
    {
        for( i=0; i<framesPerBuffer; i++ )
        {
            *wptr++ = *rptr++;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        }
        data->frameIndex += framesPerBuffer;
        finished = paContinue;
    }
    return finished;
}

/*******************************************************************/
int main(int argc, char** argv);
int main(int argc, char** argv)
{
    PaStreamParameters  inputParameters,
                        outputParameters;
    PaStream*           stream;
    PaError             err = paNoError;
    paTestData          data;
    int                 i;
    int                 totalFrames;
    int                 numSamples;
    int                 numBytes;
    int                 num_seconds;
    char                *out;
    SNDFILE *file;
	SF_INFO info;
    sf_count_t count;
    if (argc == 3)
	{
		num_seconds = NUM_CHANNELS*atoi(argv[1]);
		out = argv[2];
	}
  else
	{
		exit(-1);
  	}
    // Open File for output
    memset (&info, 0, sizeof(info)) ;
    info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    info.samplerate = SAMPLE_RATE;
    info.channels = NUM_CHANNELS;
    file = sf_open (out, SFM_WRITE, &info);
    if (file == NULL)
    {
        printf ("\nError : Not able to create file named '%s' : %s/\n", out, sf_strerror (NULL));
        exit (1);
    };
    sf_set_string (file, SF_STR_TITLE, "SPCUP Recorded Output");
    sf_set_string (file, SF_STR_SOFTWARE, "spcup") ;
    sf_set_string (file, SF_STR_COPYRIGHT, "BITS Goa");

    // Initialize for Recording
    data.maxFrameIndex = totalFrames = num_seconds * SAMPLE_RATE;
    data.frameIndex = 0;
    numSamples = totalFrames * NUM_CHANNELS;
    numBytes = numSamples * sizeof(SAMPLE);
    data.recordedSamples = (SAMPLE *) malloc( numBytes );
    if( data.recordedSamples == NULL )
    {
        printf("Could not allocate record array.\n");
        Pa_Terminate(); if( data.recordedSamples ) free( data.recordedSamples ); return -1;
    }
    for( i=0; i<numSamples; i++ ) data.recordedSamples[i] = 0;

    err = Pa_Initialize();
    if( err != paNoError ) {Pa_Terminate(); if( data.recordedSamples ) free( data.recordedSamples ); return -1;}
    inputParameters.device = Pa_GetDefaultInputDevice();
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
        Pa_Terminate(); if( data.recordedSamples ) free( data.recordedSamples ); return -1;
    }
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
              &stream,
              &inputParameters,
              NULL,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,
              recordCallback,
              &data );
    if( err != paNoError ) {Pa_Terminate(); if( data.recordedSamples ) free( data.recordedSamples ); return -1;}

    err = Pa_StartStream( stream );
    if( err != paNoError ) {Pa_Terminate(); if( data.recordedSamples ) free( data.recordedSamples ); return -1;}
    printf("\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stdout);

    while( ( err = Pa_IsStreamActive( stream ) ) == 1 )
    {
        Pa_Sleep(1000);
    }
    if( err < 0 ) {Pa_Terminate(); if( data.recordedSamples ) free( data.recordedSamples ); return -1;}

    err = Pa_CloseStream( stream );
    if( err != paNoError ) {Pa_Terminate(); if( data.recordedSamples ) free( data.recordedSamples ); return -1;}

    // Write Recorded Audio to output file
    for (i=0; i<num_seconds;i++)
    {
        sf_write_float (file, &data.recordedSamples[i*SAMPLE_RATE], SAMPLE_RATE);
    }
	sf_close (file);

    // Begin Algorithms
    float* stft_result_final  = (float*) malloc(sizeof(float) * (data.maxFrameIndex/STFT_HOP_SIZE) * (STFT_WINDOW_SIZE/2 + 1));
    STFT(stft_result_final, data.recordedSamples, data.maxFrameIndex, STFT_WINDOW_SIZE, STFT_HOP_SIZE);
    // for (int i = 0; i<(STFT_WINDOW_SIZE/2 + 1); i++)
    // {
    //     for(int j = 0; j<(data.maxFrameIndex/STFT_HOP_SIZE); j++)
    //     {
    //         printf("%f ",stft_result_final[i*(data.maxFrameIndex/STFT_HOP_SIZE)+j]);
    //     }
    //     printf("\n");
    // }
    // TODO:
    // Compute Novelty curve from spectogram (stft_result_final). Discrete Derivative (positive difference only) row wise
    // and accumulate column wise.
    // Subract local average curve from Novelty curve and also normalize it.
    // Novelty curve to Tempogram.Optimized local periodicity kernels using fourier analysis. Accumulate all maximising kernels in time.
    // Apply halfwave rectification to obtain PLP (Predominant Local Pulse Curve)
    // Restrict Tempogram to a range of BPM to obtain better PLP curve.

    /* Playback recorded data.  -------------------------------------------- */
    data.frameIndex = 0;

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default output device.\n");
        Pa_Terminate(); if( data.recordedSamples ) free( data.recordedSamples ); return -1;
    }
    outputParameters.channelCount = NUM_CHANNELS;                     /* stereo output */
    outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    printf("\n=== Now playing back. ===\n"); fflush(stdout);
    err = Pa_OpenStream(
              &stream,
              NULL, /* no input */
              &outputParameters,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              playCallback,
              &data );
    if( err != paNoError ) {Pa_Terminate(); if( data.recordedSamples ) free( data.recordedSamples ); return -1;}

    if( stream )
    {
        err = Pa_StartStream( stream );
        if( err != paNoError ) {Pa_Terminate(); if( data.recordedSamples ) free( data.recordedSamples ); return -1;}

        printf("Waiting for playback to finish.\n"); fflush(stdout);

        while( ( err = Pa_IsStreamActive( stream ) ) == 1 )
        {
            Pa_Sleep(100);
        }
        if( err < 0 ) {Pa_Terminate(); if( data.recordedSamples ) free( data.recordedSamples ); return -1;}

        err = Pa_CloseStream( stream );
        if( err != paNoError ) {Pa_Terminate(); if( data.recordedSamples ) free( data.recordedSamples ); return -1;}

        printf("Done.\n"); fflush(stdout);
    }
    Pa_Terminate();
    if( data.recordedSamples )       /* Sure it is NULL or valid. */
        free( data.recordedSamples );
    if( err != paNoError )
    {
        fprintf( stderr, "An error occured while using the portaudio stream\n" );
        fprintf( stderr, "Error number: %d\n", err );
        fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
        err = 1;          /* Always return 0 or 1, but no other return codes. */
    }
    fftw_free(stft_result_final);
    return err;
}
