#include  "moving_average.h"

SAMPLE* movingAverageOfNoveltyCurve(SAMPLE *noveltyCurve, long sizeOfArray, int sizeOfMovingAv)
{
  int lengthOfMovingAvArray = sizeOfArray - sizeOfMovingAv + 1;
  SAMPLE oneBySizeOfMovingAvArray = 1.0/sizeOfMovingAv;
  SAMPLE* movingAverageArray = malloc(lengthOfMovingAvArray*sizeof(SAMPLE));
  int i, k;

  movingAverageArray[0] = 0;
  for(i=0; i<sizeOfMovingAv; i++)
  {
    movingAverageArray[0] += noveltyCurve[i];
  }
  movingAverageArray[0] = movingAverageArray[0]/sizeOfMovingAv;

  for(i=0, k = 0; i<lengthOfMovingAvArray-1; i++)
  {
    movingAverageArray[i+1] = movingAverageArray[i] + (oneBySizeOfMovingAvArray)*(noveltyCurve[k+sizeOfMovingAv] - noveltyCurve[k]);
    k++;
  }
  return movingAverageArray;
  // Remember to free() pointer in main after porcessing is done
}
