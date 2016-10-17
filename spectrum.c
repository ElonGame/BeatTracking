#include "spectrum.h"
/* Pick the best FFT length good for FFTW?
**
** We use fftw_plan_r2r_1d() for which the documantation
** http://fftw.org/fftw3_doc/Real_002dto_002dReal-Transforms.html says:
**
** "FFTW is generally best at handling sizes of the form
** 2^a 3^b 5^c 7^d 11^e 13^f
** where e+f is either 0 or 1, and the other exponents are arbitrary."
*/

/* Helper function: does N have only 2, 3, 5 and 7 as its factors? */
static bool is_2357 (int n)
{
	/* Just eliminate all factors os 2, 3, 5 and 7 and see if 1 remains */
	while (n % 2 == 0) n /= 2;
	while (n % 3 == 0) n /= 3;
	while (n % 5 == 0) n /= 5;
	while (n % 7 == 0) n /= 7;
	return (n == 1);
}

/* Helper function: is N a "fast" value for the FFT size? */
static bool is_good_speclen (int n)
{
	/* It wants n, 11*n, 13*n but not (11*13*n)
	** where n only has as factors 2, 3, 5 and 7
	*/
	if (n % (11 * 13) == 0)
	return 0; /* No good */

	return is_2357 (n)	|| ((n % 11 == 0) && is_2357 (n / 11)) || ((n % 13 == 0) && is_2357 (n / 13));
}

int calc_spec_len(SAMPLE fft_freq)
{
	int speclen;
	/*
	** Choose a speclen value, the spectrum length.
	** The FFT window size is twice this.
	*/
	if (fft_freq != 0.0)
	/* Choose an FFT window size of 1/fft_freq seconds of audio */
	speclen = (SAMPLE_RATE / fft_freq) ;
	else
	/* Long enough to represent frequencies down to 20Hz. */
	speclen =  (SAMPLE_RATE / 20);

	/* Find the nearest fast value for the FFT size. */
	int d;	/* difference */

	for (d = 0; /* Will terminate */; d++)
	{	/* Logarithmically, the integer above is closer than
		** the integer below, so prefer it to the one below.
		*/
		if (is_good_speclen (speclen + d))
		{
			speclen += d;
			break;
		}
		/* FFT length must also be >= the output height,
		** otherwise repeated pixel rows occur in the output.
		*/
		if (speclen - d >= 20000 && is_good_speclen (speclen - d))
		{
			speclen -= d;
			break;
		}
	}
	return speclen;
}

spectrum * create_spectrum (SAMPLE * data, int max_frame, int speclen, int hop_size, enum WINDOW_FUNCTION window_function, float min_freq, float max_freq)
{
	spectrum *spec;

	spec = calloc (1, sizeof (spectrum));
	if (spec == NULL)
	{
		printf ("%s : Not enough memory.\n", __func__);
		exit (1);
	};

	spec->wfunc = window_function;
	spec->speclen = speclen;
	spec->data = data;
	spec->max_frame = max_frame;
	spec->hop_size = hop_size;
	spec->min_freq = min_freq;
	spec->max_freq = max_freq;

	spec->time_domain = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * speclen );
	spec->window = (SAMPLE * ) malloc( sizeof( SAMPLE ) * speclen );
	spec->freq_domain = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * speclen);
	spec->mag_spec = (SAMPLE * ) malloc( sizeof( SAMPLE ) * (max_frame/hop_size) * (speclen/2 + 1));

	if (spec->time_domain == NULL || spec->window == NULL || spec->freq_domain == NULL || spec->mag_spec == NULL)
	{
		printf ("%s : Not enough memory.\n", __func__);
		exit (1);
	};

	spec->plan = fftw_plan_dft_1d( speclen, spec->time_domain, spec->freq_domain, FFTW_FORWARD, FFTW_ESTIMATE );

	if (spec->plan == NULL)
	{
		printf ("%s:%d : fftw create plan failed.\n", __func__, __LINE__);
		free (spec);
		exit (1);
	};

	switch (spec->wfunc)
	{
		case RECTANGULAR :
		break;
		case KAISER :
		calc_kaiser_window (spec->window, speclen, 20.0);
		break;
		case NUTTALL:
		calc_nuttall_window (spec->window, speclen);
		break;
		case HANN :
		calc_hann_window (spec->window,  speclen);
		break;
		default :
		printf ("Internal error: Unknown window_function.\n");
		free (spec);
		exit (1);
	};



	return spec;
} /* create_spectrum */


void destroy_spectrum (spectrum * spec)
{
	fftw_destroy_plan (spec->plan);
	free (spec->time_domain);
	free (spec->window);
	free (spec->freq_domain);
	free (spec->mag_spec);
	free (spec);
} /* destroy_spectrum */

SAMPLE calc_magnitude_spectrum (spectrum * spec, int type)
{
	SAMPLE max = FLT_MIN;
	int k;

	int chunk_position = 0;
	int read_index;
	int b_stop = 0;
	int num_chunks = 0;
	while(num_chunks < (spec->max_frame/spec->hop_size) && !b_stop)
	{
		for (k = 0; k < spec->speclen; k++)
		{
			read_index = chunk_position + k;
			if (read_index < spec->max_frame)
			{
				spec->time_domain [k][0] = spec->data[read_index];
				spec->time_domain [k][1] = 0.0;
			} else
			{
				// we have read beyond the signal, so zero-pad it!
				spec->time_domain [k][0] = 0;
				spec->time_domain [k][1] = 0.0;
				b_stop = 1;
			}
		}

		if (spec->wfunc != RECTANGULAR)
		for (k = 0; k < spec->speclen; k++)
		{
			spec->time_domain [k][0] *= spec->window [k];
		}

		fftw_execute (spec->plan);

		/* Convert from FFTW's "half complex" format to an array of magnitudes.
		** In HC format, the values are stored:
		** r0, r1, r2 ... r(n/2), i(n+1)/2-1 .. i2, i1
		**/
		for (k = 0; k < (spec->speclen/2 +1); k++)
		{
			SAMPLE re = spec->freq_domain [k][0];
			SAMPLE im = spec->freq_domain [k][1];
			SAMPLE value;
			switch (type) {
				case 0:
				value = sqrt (re * re + im * im);
				break;
				case 1:
				value = 0.21714724095 * log (re * re + im * im);
				break;
				case 2:
				value = 4.34294481903 * log(re * re + im * im);
				break;
			}
			spec->mag_spec [k*(spec->max_frame/spec->hop_size) + num_chunks] = value;
			max = MAX (max, spec->mag_spec [k]);
		};

		chunk_position += spec->hop_size;
		num_chunks++;
	};
	return max;
} /* calc_magnitude_spectrum */
