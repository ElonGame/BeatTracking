#include "restrict_bpm.h"

SAMPLE restrictBPM(SAMPLE **tempogram, int length, int height)
{
  int *maxBPMArray = malloc(length*sizeof(int));
  int modeBPM;
  int i, j;
  SAMPLE temp;
  for(j=0; j<length; j++)
  {
    temp = tempogram[0][j];
    for(i=1;i<height;i++)
    {
      if(tempogram[i][j] > temp)
      {
        temp = tempogram[i][j];
        *(maxBPMArray+j) = i;
      }
    }
  }
  modeBPM = findMode(maxBPMArray, length);
  return modeBPM;
}
