#define PI 3.14159265359
// Create a hamming window of windowLength samples in buffer
void hamming_window(int windowLength, float *buffer)
{
    for(int i = 0; i < windowLength; i++)
    {
        buffer[i] = 0.54 - (0.46 * cos( 2 * PI * (i / ((windowLength - 1) * 1.0))));
    }
}

void STFT(float *stft_result_final, float *signal, int signalLength, int windowSize, int hopSize) {


    fftw_complex    *data, *fft_result, *ifft_result;
    fftw_plan       plan_forward, plan_backward;
    int             i;

    data        = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * windowSize );
    fft_result  = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * windowSize );
    ifft_result = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * windowSize );

    plan_forward  = fftw_plan_dft_1d( windowSize, data, fft_result, FFTW_FORWARD, FFTW_ESTIMATE );

    // Create a hamming window of appropriate length
    float window[windowSize];
    hamming_window(windowSize, window);

    int chunkPosition = 0;

    int readIndex;

    // Should we stop reading in chunks?
    int bStop = 0;

    int numChunks = 0;
    // printf("%d\n",signalLength);
    // Process each chunk of the signal
    while(numChunks < (signalLength/hopSize) && !bStop)
    {
        // Copy the chunk into our buffer
        for(i = 0; i < windowSize; i++)
        {
            readIndex = chunkPosition + i;
            if(readIndex < signalLength)
            {
                // Note the windowing!
                data[i][0] = signal[readIndex] * window[i];
                data[i][1] = 0.0;
            } else
            {
                // we have read beyond the signal, so zero-pad it!

                data[i][0] = 0.0;
                data[i][1] = 0.0;
                bStop = 1;
            }
        }
        // Perform the FFT on our chunk
        fftw_execute( plan_forward );

        // Uncomment to see the raw-data output from the FFT calculation
        // printf("Column: %d\n",chunkPosition);
        // for(i = 0 ; i < windowSize ; i++ )
        // {
        //     fprintf( stdout, "fft_result[%d] = { %2.2f, %2.2f }\n",i, fft_result[i][0], fft_result[i][1] );
        // }



        // Copy the first (windowSize/2 + 1) data points into your spectrogram.
        // We do this because the FFT output is mirrored about the nyquist
        // frequency, so the second half of the data is redundant. This is how
        // Matlab's spectrogram routine works.


        for (i = 0; i < (windowSize/2 + 1); i++)
        {
            stft_result_final[i*(signalLength/hopSize) + numChunks] = 0.5 * log(fft_result[i][0]*fft_result[i][0] + fft_result[i][1]*fft_result[i][1]);
            // float temp = 0.5 * fabs(log(fft_result[i][0]*fft_result[i][0] + fft_result[i][1]*fft_result[i][1]));
            // printf("%d ", i);
        }
        // printf("%d\n",numChunks);

        chunkPosition += hopSize;
        numChunks++;
    }; // Excuse the formatting, the while ends here.

    fftw_destroy_plan(plan_forward);

    fftw_free(data);
    fftw_free(fft_result);
    fftw_free(ifft_result);
}
