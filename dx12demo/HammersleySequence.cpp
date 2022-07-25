#include "HammersleySequence.h"

HammersleySequence::HammersleySequence()
{
	int kk = 0;
	float p = 0.0f;
	float u = 0.0f;
	int idx = 0;
	for (int k = 0; k < K_LENGTH; ++k)
	{
		for (p = 0.5f, kk = k, u = 0.0f; kk; p *= 0.5f, kk >>= 1)
		{
			if (kk & 1)
			{
				u += p;
			}
		}
		float v = (k + 0.5f) / K_LENGTH;
		sequence[idx++] = u;
		sequence[idx++] = v;
	}
}
