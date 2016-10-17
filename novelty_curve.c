#include "novelty_curve.h"
void calculate_novelty_curve(SAMPLE * mag_spec,int speclen, int max_frame, int hop_size, SAMPLE* novelty)
{
    SAMPLE *temp = (SAMPLE * ) malloc( sizeof( SAMPLE ) * (max_frame/(hop_size*2)) * (speclen/2 + 1));
    for (int i = 0; i<(speclen/2 + 1); i++)
    {
        for(int j = 0; j<(max_frame/(hop_size*2)); j++)
        {
            temp[i*(max_frame/(hop_size*2)) + j] = mag_spec[i*(max_frame/hop_size) + j+1]-mag_spec[i*(max_frame/hop_size) + j];
            if (temp[i*(max_frame/(hop_size*2)) + j] < 0.0)
                temp[i*(max_frame/(hop_size*2)) + j] = 0.0;
        }
    }
    SAMPLE sum = 0.0;
    for(int j = 0; j<(max_frame/(hop_size*2)); j++)
    {
        novelty[j] = 0.0;
        for (int i = 0; i<(speclen/2 + 1); i++)
        {
            if(!isnan(temp[i*(max_frame/(hop_size*2)) + j]) && !isinf(temp[i*(max_frame/(hop_size*2)) + j]))
                novelty[j] += temp[i*(max_frame/(hop_size*2)) + j];
        }
    }

    // Moving Average Start
    int sizeOfMovingAv = 5;
    int lengthOfMovingAvArray = max_frame/(hop_size*2) - sizeOfMovingAv + 1; //Number of Moving Averages taken
    // printf("%d %d\n",max_frame/(hop_size*2),lengthOfMovingAvArray);
    SAMPLE *movingAverageArray = (SAMPLE *)malloc(sizeof(SAMPLE)*lengthOfMovingAvArray);
    movingAverageOfNoveltyCurve(novelty, movingAverageArray, max_frame/(hop_size*2), sizeOfMovingAv);
    for(int i=0; i < lengthOfMovingAvArray; i++)
    {
      novelty[i] -= movingAverageArray[i];
    }
    // Moving Averge End

    for (int i = 0; i < max_frame/(hop_size*2); i++)
    {
      sum+=(novelty[i]*novelty[i]);
    }
    SAMPLE norm2 = sqrt(sum)/(max_frame/(hop_size*2));
    for(int j = 0; j<(max_frame/(hop_size*2)); j++)
    {
        novelty[j] /= norm2;
    }
    free(temp);
    free(movingAverageArray);
}
