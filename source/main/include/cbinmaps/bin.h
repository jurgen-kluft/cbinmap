#ifndef __CBINMAPS_BIN_H__
#define __CBINMAPS_BIN_H__
#include "ccore/c_target.h"
#include "ccore/c_debug.h"

namespace ncore
{
	//  
	//  Numbering for (aligned) logarithmic bins.
	//  
	//  Each number stands for an interval
	//    [layer_offset * 2^layer, (layer_offset + 1) * 2^layer).
	//  
	//  The following value is called as base_offset:
	//    layer_offset * 2^layer -- is called
	//  
	//  Bin numbers in the tail111 encoding: meaningless bits in
	//  the tail are set to 0111...11, while the head denotes the offset.
	//  bin = 2 ^ (layer + 1) * layer_offset + 2 ^ layer - 1
	//  
	//  Thus, 1101 is the bin at layer 1, offset 3 (i.e. fourth).
	// 
	//                   +-----------------00111-----------------+
	//                   |                                       |
	//         +-------00011-------+                   +-------01011-------+
	//         |                   |                   |                   |
	//    +--00001--+         +--00101--+         +--01001--+         +--01101--+
	//    |         |         |         |         |         |         |         |
	//  00000     00010     00100     00110     01000     01010     01100     1110
	//  
	//  
	//                7                   -
	//            /       \               -
	//        3              11           -
	//      /   \           /  \          -
	//    1       5       9     13        -
	//   / \     / \     / \    / \       -
	//  0   2   4   6   8  10  12 14      -
	//  
	//  Once we have peak hashes, this structure is more natural than bin-v1
	//  

	class bin_t 
	{
	public:
		typedef u64		uint_t;

		static const bin_t NONE;
		static const bin_t ALL;

						bin_t(void);
		explicit		bin_t(uint_t val);
						bin_t(s32 layer, uint_t layer_offset);

		uint_t			value(void) const;

		bool			operator == (const bin_t& bin) const;
		bool			operator != (const bin_t& bin) const;
		bool			operator  < (const bin_t& bin) const;
		bool			operator  > (const bin_t& bin) const;
		bool			operator <= (const bin_t& bin) const;
		bool			operator >= (const bin_t& bin) const;

		bool			contains(const bin_t& bin) const;
		void			decompose(s32& layer, uint_t& layer_offset) const;

		uint_t			base_offset(void) const;
		uint_t			base_length(void) const;

		s32				layer(void) const;
		uint_t			layer_bits(void) const;
		uint_t			layer_offset(void) const;

		bool			is_none(void) const;
		bool			is_all(void) const;
		bool			is_base(void) const;
		bool			is_left(void) const;
		bool			is_right(void) const;

		bin_t&			to_parent(void);
		bin_t&			to_left(void);
		bin_t&			to_right(void);
		bin_t&			to_sibling(void);
		bin_t&			to_base_left(void);
		bin_t&			to_base_right(void);
		bin_t&			to_twisted(uint_t mask);
		bin_t&			to_layer_shifted(s32 zlayer);

		bin_t			parent(void) const;
		bin_t			left(void) const;
		bin_t			right(void) const;
		bin_t			sibling(void) const;
		bin_t			base_left(void) const;
		bin_t			base_right(void) const;
		bin_t			twisted(uint_t mask) const;
		bin_t			layer_shifted(s32 zlayer) const;

		static bin_t	to_root(uint_t max_layer0_bin);

		const char*		str(char* buf) const;

	private:
		uint_t			v_;
	};

	inline bin_t::bin_t(void)
		: v_(bin_t::NONE.v_)
	{ }

	inline bin_t::bin_t(uint_t val)
		: v_(val)
	{ }

	inline bin_t::bin_t(s32 layer, uint_t offset)
	{
		if (static_cast<s32>(layer) < 8 * sizeof(bin_t::uint_t)) 
			v_ = ((2 * offset + 1) << layer) - 1;
		else 
			v_ = static_cast<uint_t>(-1); // Definition of the NONE bin
	}

	inline bin_t::uint_t bin_t::value(void) const				{ return v_; }

	inline bool bin_t::operator == (const bin_t& bin) const		{ return v_ == bin.v_; }
	inline bool bin_t::operator != (const bin_t& bin) const		{ return v_ != bin.v_; }
	inline bool bin_t::operator < (const bin_t& bin) const		{ return v_ < bin.v_; }
	inline bool bin_t::operator > (const bin_t& bin) const		{ return v_ > bin.v_; }
	inline bool bin_t::operator <= (const bin_t& bin) const		{ return v_ <= bin.v_; }
	inline bool bin_t::operator >= (const bin_t& bin) const		{ return v_ >= bin.v_; }

	inline void bin_t::decompose(s32& layer, uint_t& layer_offset) const
	{
		const s32 l = this->layer();
		layer = l;
		layer_offset = v_ >> (l + 1);
	}

	inline bin_t::uint_t bin_t::base_offset(void) const
	{
		return (v_ & (v_ + 1)) >> 1;
	}

	inline bin_t::uint_t bin_t::base_length(void) const
	{
		bin_t::uint_t const t = ((v_ + 1) & (-1 - v_));
		return t;
	}

	inline bin_t::uint_t bin_t::layer_bits(void) const
	{
		return v_ ^ (v_ + 1);
	}

	inline bin_t::uint_t bin_t::layer_offset(void) const
	{
		return v_ >> (layer() + 1);
	}

	inline bool bin_t::is_none(void) const						{ return v_ == NONE.v_; }
	inline bool bin_t::is_all(void) const						{ return v_ == ALL.v_; }

	inline bool bin_t::is_base(void) const
	{
		return (v_ & 1) == 0;
	}

	inline bool bin_t::is_left(void) const
	{
		return !(v_ & (layer_bits() + 1));
	}

	inline bool bin_t::is_right(void) const
	{
		return (v_ & (layer_bits() + 1));
	}

	inline bin_t& bin_t::to_parent(void)
	{
		const bin_t::uint_t lbs = layer_bits();
		const bin_t::uint_t nlbs = -2 - lbs; /* ~(lbs + 1) */
		v_ = (v_ | lbs) & nlbs;
		return *this;
	}

	inline bin_t& bin_t::to_left(void)
	{
		bin_t::uint_t const t = ((v_ + 1) & (-1 - v_)) >> 1;
		v_ ^= t;
		return *this;
	}

	inline bin_t& bin_t::to_right(void)
	{
		bin_t::uint_t const t = ((v_ + 1) & (-1 - v_)) >> 1;
		v_ += t;
		return *this;
	}

	inline bin_t& bin_t::to_sibling(void)
	{
		v_ ^= (v_ ^ (v_ + 1)) + 1;
		return *this;
	}

	inline bin_t& bin_t::to_base_left(void)
	{
		if (!is_none()) 
			v_ &= (v_ + 1);
		return *this;
	}

	inline bin_t& bin_t::to_base_right(void)
	{
		if (!is_none()) 
			v_ = (v_ | (v_ + 1)) - 1;
		return *this;
	}

	inline bin_t& bin_t::to_twisted(bin_t::uint_t mask)
	{
		v_ ^= ((mask << 1) & ~layer_bits());
		return *this;
	}

	inline bin_t& bin_t::to_layer_shifted(s32 zlayer)
	{
		if (layer_bits() >> zlayer) 
			v_ >>= zlayer;
		else 
			v_ = (v_ >> zlayer) & ~static_cast<bin_t::uint_t>(1);
		return *this;
	}

	inline bin_t bin_t::parent(void) const
	{
		const bin_t::uint_t lbs = layer_bits();
		const bin_t::uint_t nlbs = -2 - lbs; /* ~(lbs + 1) */
		return bin_t((v_ | lbs) & nlbs);
	}

	inline bin_t bin_t::left(void) const
	{
		bin_t::uint_t const t = ((v_ + 1) & (-1 - v_)) >> 1;
		return bin_t(v_ ^ t);
	}

	inline bin_t bin_t::right(void) const
	{
		bin_t::uint_t const t = ((v_ + 1) & (-1 - v_)) >> 1;
		return bin_t(v_ + t);
	}

	inline bin_t bin_t::sibling(void) const
	{
		return bin_t(v_ ^ (layer_bits() + 1));
	}

	inline bin_t bin_t::base_left(void) const
	{
		if (is_none())
			return NONE;
		return bin_t(v_ & (v_ + 1));
	}

	inline bin_t bin_t::base_right(void) const
	{
		if (is_none())
			return NONE;
		return bin_t((v_ | (v_ + 1)) - 1);
	}

	inline bin_t bin_t::twisted(bin_t::uint_t mask) const
	{
		return bin_t( v_ ^ ((mask << 1) & ~layer_bits()) );
	}

	inline bin_t bin_t::layer_shifted(s32 zlayer) const
	{
		if (layer_bits() >> zlayer) 
			return bin_t( v_  >> zlayer );
		else 
			return bin_t( (v_ >> zlayer) & ~static_cast<bin_t::uint_t>(1) );
	}

	inline bool bin_t::contains(const bin_t& bin) const
	{
		if (is_none()) 
			return false;
		return (v_ & (v_ + 1)) <= bin.v_ && bin.v_ < (v_ | (v_ + 1));
	}

}


#endif // __CBINMAPS_BIN_H__
