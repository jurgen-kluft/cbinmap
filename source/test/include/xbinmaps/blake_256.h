#ifndef __BLAKE_256_H__ 
#define __BLAKE_256_H__
#include "xbase/x_target.h"

namespace xcore
{
	struct blake256_state
	{
		u32			h[8], s[4], t[2];
		int			buflen, nullt;
		u8			buf[64];
	};

	void blake256_init	( blake256_state *S );
	void blake256_update( blake256_state *S, const u8 *in, u64 inlen );
	void blake256_final	( blake256_state *S, u8 *out );

}

#endif