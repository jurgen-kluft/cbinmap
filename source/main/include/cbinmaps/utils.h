#ifndef __CBINMAPS_BIN_UTILS_H__
#define __CBINMAPS_BIN_UTILS_H__
#include "ccore/c_target.h"
#include "cbinmaps/bin.h"

namespace ncore
{
	/**
	 * Generating a list of peak bins for corresponding length
	 */
	inline s32 gen_peaks(u64 length, bin_t * peaks) 
	{
		s32 pp = 0;
		u8 layer = 0;

		while (length)
		{
			if (length & 1)
				peaks[pp++] = bin_t((u32)((2 * length - 1) << layer) - 1);
			length >>= 1;
			layer++;
		}

		for(s32 i = 0; i < (pp >> 1); ++i)
		{
			bin_t memo = peaks[pp - 1 - i];
			peaks[pp - 1 - i] = peaks[i];
			peaks[i] = memo;
		}

		peaks[pp] = bin_t::NONE;
		return pp;
	}


	/**
	 * Checking for that the bin value is fit to u32
	 */
	inline bool bin_isUInt32(const bin_t & bin)
	{
		if( bin.is_all() )
			return true;
		if( bin.is_none() )
			return true;

		const u64 v = bin.value();

		return static_cast<u32>(v) == v && v != 0xffffffff && v != 0x7fffffff;
	}


	/**
	 * Convert the bin value to u32
	 */
	inline u32 bin_toUInt32(const bin_t & bin)
	{
		if( bin.is_all() )
			return 0x7fffffff;
		if( bin.is_none() )
			return 0xffffffff;
		return static_cast<u32>(bin.value());
	}


	/**
	 * Convert the bin value to u64
	 */
	inline u64 bin_toUInt64(const bin_t & bin)
	{
		return bin.value();
	}


	/**
	 * Restore the bin from an u32 value
	 */
	inline bin_t bin_fromUInt32(u32 v) 
	{
		if( v == 0x7fffffff )
			return bin_t::ALL;
		if( v == 0xffffffff )
			return bin_t::NONE;
		return bin_t(static_cast<u64>(v));
	}


	/**
	 * Restore the bin from an u64 value
	 */
	inline bin_t bin_fromUInt64(u64 v)
	{
		return bin_t((u32)static_cast<u64>(v));
	}

}


#endif // __CBINMAPS_BIN_UTILS_H__
