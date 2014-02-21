#include "xbinmaps\blake_256.h" 

#include <string>

namespace xcore
{
	typedef		u64				uint64_t;
	typedef		u32				uint32_t;
	typedef		u8				uint8_t;

	typedef		blake256_state	state256;

#define U8TO32_BIG(p)					      \
	(((uint32_t)((p)[0]) << 24) | ((uint32_t)((p)[1]) << 16) |  \
	((uint32_t)((p)[2]) <<  8) | ((uint32_t)((p)[3])      ))

#define U32TO8_BIG(p, v)				        \
	(p)[0] = (uint8_t)((v) >> 24); (p)[1] = (uint8_t)((v) >> 16); \
	(p)[2] = (uint8_t)((v) >>  8); (p)[3] = (uint8_t)((v)      );

#define U8TO64_BIG(p) \
	(((uint64_t)U8TO32_BIG(p) << 32) | (uint64_t)U8TO32_BIG((p) + 4))

#define U64TO8_BIG(p, v)		      \
	U32TO8_BIG((p),     (uint32_t)((v) >> 32)); \
	U32TO8_BIG((p) + 4, (uint32_t)((v)      ));

	typedef state256 state224;

	typedef struct
	{
		uint64_t h[8], s[4], t[2];
		int buflen, nullt;
		uint8_t buf[128];
	} state512;

	typedef state512 state384;

	const uint8_t sigma[][16] =
	{
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
		{14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3 },
		{11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4 },
		{ 7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8 },
		{ 9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13 },
		{ 2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9 },
		{12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11 },
		{13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10 },
		{ 6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5 },
		{10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13 , 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
		{14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3 },
		{11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4 },
		{ 7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8 },
		{ 9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13 },
		{ 2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9 }
	};

	const uint32_t au256[16] =
	{
		0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344,
		0xa4093822, 0x299f31d0, 0x082efa98, 0xec4e6c89,
		0x452821e6, 0x38d01377, 0xbe5466cf, 0x34e90c6c,
		0xc0ac29b7, 0xc97c50dd, 0x3f84d5b5, 0xb5470917
	};

	const uint64_t au512[16] =
	{
		0x243f6a8885a308d3ULL, 0x13198a2e03707344ULL, 
		0xa4093822299f31d0ULL, 0x082efa98ec4e6c89ULL,
		0x452821e638d01377ULL, 0xbe5466cf34e90c6cULL, 
		0xc0ac29b7c97c50ddULL, 0x3f84d5b5b5470917ULL,
		0x9216d5d98979fb1bULL, 0xd1310ba698dfb5acULL, 
		0x2ffd72dbd01adfb7ULL, 0xb8e1afed6a267e96ULL,
		0xba7c9045f12c7f99ULL, 0x24a19947b3916cf7ULL, 
		0x0801f2e2858efc16ULL, 0x636920d871574e69ULL
	};


	static const uint8_t padding[129] =
	{
		0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};



	void blake256_compress( state256 *S, const uint8_t *block )
	{
		uint32_t v[16], m[16], i;
#define ROT(x,n) (((x)<<(32-n))|( (x)>>(n)))
#define G(a,b,c,d,e)          \
	v[a] += (m[sigma[i][e]] ^ au256[sigma[i][e+1]]) + v[b]; \
	v[d] = ROT( v[d] ^ v[a],16);        \
	v[c] += v[d];           \
	v[b] = ROT( v[b] ^ v[c],12);        \
	v[a] += (m[sigma[i][e+1]] ^ au256[sigma[i][e]])+v[b]; \
	v[d] = ROT( v[d] ^ v[a], 8);        \
	v[c] += v[d];           \
	v[b] = ROT( v[b] ^ v[c], 7);

		for( i = 0; i < 16; ++i )  m[i] = U8TO32_BIG( block + i * 4 );

		for( i = 0; i < 8; ++i )  v[i] = S->h[i];

		v[ 8] = S->s[0] ^ au256[0];
		v[ 9] = S->s[1] ^ au256[1];
		v[10] = S->s[2] ^ au256[2];
		v[11] = S->s[3] ^ au256[3];
		v[12] = au256[4];
		v[13] = au256[5];
		v[14] = au256[6];
		v[15] = au256[7];

		/* don't xor t when the block is only padding */
		if ( !S->nullt )
		{
			v[12] ^= S->t[0];
			v[13] ^= S->t[0];
			v[14] ^= S->t[1];
			v[15] ^= S->t[1];
		}

		for( i = 0; i < 14; ++i )
		{
			/* column step */
			G( 0,  4,  8, 12,  0 );
			G( 1,  5,  9, 13,  2 );
			G( 2,  6, 10, 14,  4 );
			G( 3,  7, 11, 15,  6 );
			/* diagonal step */
			G( 0,  5, 10, 15,  8 );
			G( 1,  6, 11, 12, 10 );
			G( 2,  7,  8, 13, 12 );
			G( 3,  4,  9, 14, 14 );
		}

		for( i = 0; i < 16; ++i )  S->h[i % 8] ^= v[i];

		for( i = 0; i < 8 ; ++i )  S->h[i] ^= S->s[i % 4];
	}


	void blake256_init( state256 *S )
	{
		S->h[0] = 0x6a09e667;
		S->h[1] = 0xbb67ae85;
		S->h[2] = 0x3c6ef372;
		S->h[3] = 0xa54ff53a;
		S->h[4] = 0x510e527f;
		S->h[5] = 0x9b05688c;
		S->h[6] = 0x1f83d9ab;
		S->h[7] = 0x5be0cd19;
		S->t[0] = S->t[1] = S->buflen = S->nullt = 0;
		S->s[0] = S->s[1] = S->s[2] = S->s[3] = 0;
	}


	void blake256_update( state256 *S, const uint8_t *in, uint64_t inlen )
	{
		int left = S->buflen;
		int fill = 64 - left;

		/* data left and data received fill a block  */
		if( left && ( inlen >= fill ) )
		{
			memcpy( ( void * ) ( S->buf + left ), ( void * ) in, fill );
			S->t[0] += 512;

			if ( S->t[0] == 0 ) S->t[1]++;

			blake256_compress( S, S->buf );
			in += fill;
			inlen  -= fill;
			left = 0;
		}

		/* compress blocks of data received */
		while( inlen >= 64 )
		{
			S->t[0] += 512;

			if ( S->t[0] == 0 ) S->t[1]++;

			blake256_compress( S, in );
			in += 64;
			inlen -= 64;
		}

		/* store any data left */
		if( inlen > 0 )
		{
			memcpy( ( void * ) ( S->buf + left ),   \
				( void * ) in, ( size_t ) inlen );
			S->buflen = left + ( int )inlen;
		}
		else S->buflen = 0;
	}


	void blake256_final( state256 *S, hash256& out )
	{
		uint8_t msglen[8], zo = 0x01, oo = 0x81;
		uint32_t lo = S->t[0] + ( S->buflen << 3 ), hi = S->t[1];

		/* support for hashing more than 2^32 bits */
		if ( lo < uint32_t( S->buflen << 3 ) ) 
			hi++;

		U32TO8_BIG(  msglen + 0, hi );
		U32TO8_BIG(  msglen + 4, lo );

		if ( S->buflen == 55 )   /* one padding byte */
		{
			S->t[0] -= 8;
			blake256_update( S, &oo, 1 );
		}
		else
		{
			if ( S->buflen < 55 )   /* enough space to fill the block  */
			{
				if ( !S->buflen ) S->nullt = 1;

				S->t[0] -= 440 - ( S->buflen << 3 );
				blake256_update( S, padding, 55 - S->buflen );
			}
			else   /* need 2 compressions */
			{
				S->t[0] -= 512 - ( S->buflen << 3 );
				blake256_update( S, padding, 64 - S->buflen );
				S->t[0] -= 440;
				blake256_update( S, padding + 1, 55 );
				S->nullt = 1;
			}

			blake256_update( S, &zo, 1 );
			S->t[0] -= 8;
		}

		S->t[0] -= 64;
		blake256_update( S, msglen, 8 );
		U32TO8_BIG( &out.hash[ 0], S->h[0] );
		U32TO8_BIG( &out.hash[ 4], S->h[1] );
		U32TO8_BIG( &out.hash[ 8], S->h[2] );
		U32TO8_BIG( &out.hash[12], S->h[3] );
		U32TO8_BIG( &out.hash[16], S->h[4] );
		U32TO8_BIG( &out.hash[20], S->h[5] );
		U32TO8_BIG( &out.hash[24], S->h[6] );
		U32TO8_BIG( &out.hash[28], S->h[7] );
	}


	void blake256_hash( hash256& out, const uint8_t *in, uint64_t inlen )
	{
		state256 S;
		blake256_init( &S );
		blake256_update( &S, in, inlen );
		blake256_final( &S, out );
	}

}