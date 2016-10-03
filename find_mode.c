#include "find_mode.h"

SAMPLE findMode(int sequence[], int N)
{
  int maxValue = 0, maxCount = 0, i, j;

    for (i = 0; i < N; ++i) {
        int count = 0;
        for (j = 0; j < N; ++j) {
            if (sequence[j] == sequence[i])
                ++count;
        }
        if (count > maxCount) {
            maxCount = count;
            maxValue = sequence[i];
        }
    }

    return maxValue;
}
