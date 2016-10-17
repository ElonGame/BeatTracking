#include "common.h"
#include "algorithms.h"
#include "callbacks.h"

int main(int argc, char** argv)
{
  PaStreamParameters inputParameters,outputParameters;
  PaStream* stream;
  PaError err = paNoError;
  paTestData data;
  int i;
  int totalFrames;
  int numSamples;
  int numBytes;
  int num_seconds, record;
  char *out, *in;
  SNDFILE *file, *infile;
  SF_INFO info, ininfo;
  sf_count_t count,count2;
  if (argc == 5)
  {
    record = atoi(argv[1]);
    num_seconds = NUM_CHANNELS*atoi(argv[2]);
    in = argv[3];
    out = argv[4];
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
  if (record)
  {
    // Initialize for Recording
    inputParameters.device = Pa_GetDefaultInputDevice();
    if (inputParameters.device == paNoDevice) {
      fprintf(stderr,"Error: No default input device.\n");
      Pa_Terminate(); if( data.recordedSamples ) free( data.recordedSamples ); return -1;
    }
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(&stream,&inputParameters,NULL,SAMPLE_RATE,FRAMES_PER_BUFFER,paClipOff,recordCallback,&data );
    if( err != paNoError ) {Pa_Terminate(); if( data.recordedSamples ) free( data.recordedSamples ); return -1;}

    err = Pa_StartStream( stream );
    if( err != paNoError ) {Pa_Terminate(); if( data.recordedSamples ) free( data.recordedSamples ); return -1;}
    printf("\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stdout);

    while( ( err = Pa_IsStreamActive( stream ) ) == 1 )
    {
      //Algorithmns need to be called here for realtime processing.
      Pa_Sleep(1000);
    }
    if( err < 0 ) {Pa_Terminate(); if( data.recordedSamples ) free( data.recordedSamples ); return -1;}

    err = Pa_CloseStream( stream );
    if( err != paNoError ) {Pa_Terminate(); if( data.recordedSamples ) free( data.recordedSamples ); return -1;}
  }
  else
  {
    memset (&ininfo, 0, sizeof(ininfo));
    infile = sf_open (in, SFM_READ, &ininfo) ;
    if (infile == NULL)
    {
      printf ("Error : failed to open file '%s' : \n%s\n", in, sf_strerror(NULL));
      return -1;
    };
    if (ininfo.channels != 1)
    {
      printf("The input file does not contain 1 channel audio\n");
      exit(1);
    }
    if (ininfo.samplerate != SAMPLE_RATE)
    {
      printf("The sample rate of the input file is not %d\n", SAMPLE_RATE);
      exit(1);
    }
    for (i=0; i<num_seconds;i++)
    {
      sf_read_float(infile, &data.recordedSamples[i*SAMPLE_RATE], SAMPLE_RATE);
    }
    sf_close (infile);
  }

  // Write Recorded Audio to output file
  for (i=0; i<num_seconds;i++)
  {
    sf_write_float(file, &data.recordedSamples[i*SAMPLE_RATE], SAMPLE_RATE);
  }
  sf_close (file);

  // Begin Algorithms
  // Audio to Spectogram
  float fft_freq = SAMPLE_RATE/SPECTROGRAM_PARAMETER;
  int speclen = calc_spec_len(fft_freq);
  int hop_size = speclen/2;
  float min_freq = 20.0, max_freq = 20000.0;
  spectrum *spec = create_spectrum(data.recordedSamples,data.maxFrameIndex, speclen,hop_size,KAISER, min_freq, max_freq);
  // printf("%d\n",speclen );
  SAMPLE max_mag= calc_magnitude_spectrum (spec,2);
  // printf("%f\n",max_mag);

  // Spectogram Visualisation
  FILE * spectrogram = fopen("spectrogram.txt", "w");
  fprintf(spectrogram, "%d %d\n", speclen/2 + 1 ,data.maxFrameIndex/(hop_size*2));
  for (int i = 0; i<(speclen/2 + 1); i++)
  {
    for(int j = 0; j<(data.maxFrameIndex/(hop_size*2)); j++)
    {
      fprintf(spectrogram, "%4.7f ",spec->mag_spec[i*(data.maxFrameIndex/speclen)+j]);
    }
    fprintf(spectrogram,"\n");
  }
  fclose(spectrogram);
  // Audio to Spectogram end
  // Spectrogram to Novelty Curve Begin
  SAMPLE *novelty_curve = (SAMPLE * ) malloc( sizeof( SAMPLE ) * (data.maxFrameIndex/(hop_size*2)));
  calculate_novelty_curve(spec->mag_spec, speclen, data.maxFrameIndex, hop_size, novelty_curve);
  // Visualize novelty curve
  FILE * novelty = fopen("novelty.txt", "w");
  fprintf(novelty, "%d\n",data.maxFrameIndex/(hop_size*2));
  for(int j = 0; j<(data.maxFrameIndex/(hop_size*2)); j++)
  {
    int ret = fprintf(novelty, "%4.7f\n",novelty_curve[j]);
    printf("%d %4.7f %d\n",j,novelty_curve[j],ret);
  }
  fprintf(novelty,"\n");
  fclose(novelty);
  // Spectrogram to Novelty Curve End


  // Novelty Curve to Tempogram Begin
  float fft_freq_tempo = SAMPLE_RATE/TEMPOGRAM_PARAMETER;
  int speclen_tempo = calc_spec_len(fft_freq_tempo);
  int hop_size_tempo = speclen_tempo/2;
  float min_freq_tempo = 20.0, max_freq_tempo = 20000.0;
  spectrum *spec_tempo = create_spectrum(novelty_curve,(data.maxFrameIndex/(hop_size*2)), speclen_tempo,hop_size_tempo,KAISER, min_freq_tempo, max_freq_tempo);
  // printf("%d\n",speclen_tempo);
  SAMPLE max_mag_tempo= calc_magnitude_spectrum (spec_tempo,0);
  // printf("%f\n",max_mag_tempo);

  // tempogram Visualisation
  FILE * tempogram = fopen("tempogram.txt", "w");
  fprintf(tempogram, "%d %d\n", speclen_tempo/2 + 1 ,(data.maxFrameIndex/(hop_size*2))/(hop_size_tempo*2));
  for (int i = 0; i<(speclen_tempo/2 + 1); i++)
  {
    for(int j = 0; j<((data.maxFrameIndex/(hop_size*2))/(hop_size_tempo*2)); j++)
    {
      fprintf(tempogram, "%4.7f ",spec_tempo->mag_spec[i*((data.maxFrameIndex/(hop_size*2))/speclen_tempo)+j]);
    }
    fprintf(tempogram,"\n");
  }
  fclose(tempogram);
  // Novelty Curve to Tempogram End
  // TODO:
  // Visualize Spectogram. The function should be such that it can handle a Tempogram, Novelty curve and PLP curve too.
  // Compute Novelty curve from spectogram (stft_result_final). Discrete Derivative (positive difference only) row wise
  // and accumulate column wise.
  // From Novelty curve annotate music with Beat info using taps at note onsets and make the function in such a way that it can handle PLP and resricted PLP curve.
  // Subract local average curve from Novelty curve and also normalize it.
  // Novelty curve to Tempogram (Ensure Tempogram can be beat restricted) by obtaining Optimized local periodicity kernels using fourier analysis.
  // Accumulate all maximising kernels in time.
  // Apply halfwave rectification to obtain PLP (Predominant Local Pulse Curve)
  // Restrict Tempogram to a range of BPM to obtain better PLP curve.

  /* Playback recorded data.  -------------------------------------------- */
  data.frameIndex = 0;

  outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
  if (outputParameters.device == paNoDevice) {
    fprintf(stderr,"Error: No default output device.\n");
    Pa_Terminate(); if( data.recordedSamples ) free( data.recordedSamples ); return -1;
  }
  outputParameters.channelCount = NUM_CHANNELS;
  outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
  outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = NULL;

  printf("\n=== Now playing back. ===\n"); fflush(stdout);
  err = Pa_OpenStream(&stream,NULL,&outputParameters,SAMPLE_RATE,FRAMES_PER_BUFFER,paClipOff,playCallback,&data);
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
  destroy_spectrum(spec);
  free (novelty_curve);
  //   fftw_free(stft_result_final);
  return err;
}
