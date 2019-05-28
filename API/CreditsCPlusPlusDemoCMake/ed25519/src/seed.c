#include "ed25519.h"

#ifndef ED25519_NO_SEED

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#else
#include <stdio.h>
#endif

#include <stdint.h>
#include <limits.h>

// Simple RNG implementation in C
// (PCG is a family of simple fast space-efficient statistically good algorithms for random number generation)

// *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / pcg-random.org
// Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)

typedef struct { uint64_t state;  uint64_t inc; } pcg32_random_t;

uint32_t pcg32_random_r(pcg32_random_t* rng)
{
    uint64_t oldstate = rng->state;
    // Advance internal state
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc|1);
    // Calculate output function (XSH RR), uses old state for max ILP
    uint32_t xorshifted = ( uint32_t )(((oldstate >> 18u) ^ oldstate) >> 27u);
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << (( INT_MAX - rot + 1 ) & 31));
}

#define PCG32_INITIALIZER   { 0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL }

pcg32_random_t pcg32_random = PCG32_INITIALIZER;

#include <time.h>

int ed25519_init_seed()
{
    time_t t = time(NULL);
    pcg32_random.inc ^= t;
    pcg32_random.state ^= t;
    
    return 0;
}

int ed25519_create_seed(unsigned char *seed)
{
    static int init = 0;
    if ( init == 0 )
    {
        init = 1;
        ed25519_init_seed();
    }
    
    for ( int i = 0; i < 32; i += sizeof(uint32_t)	 )
    {
        uint32_t r = pcg32_random_r(&pcg32_random);
        
        seed[i + 0] = r & 0xFF;
        seed[i + 1] = (r >> 8) & 0xFF;
        seed[i + 2] = (r >> 16) & 0xFF;
        seed[i + 3] = (r >> 24) & 0xFF;
    }
    
    return 0;
    
#ifdef _WIN32
    HCRYPTPROV prov;

    if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))  {
        return 1;
    }

    if (!CryptGenRandom(prov, 32, seed))  {
        CryptReleaseContext(prov, 0);
        return 1;
    }

    CryptReleaseContext(prov, 0);
#else
    FILE *f = fopen("/dev/urandom", "rb");

    if (f == NULL) {
        return 1;
    }

    fread(seed, 1, 32, f);
    fclose(f);
#endif

    return 0;
}

#endif
