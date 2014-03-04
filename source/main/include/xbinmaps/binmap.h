#ifndef __XBINMAP_BINMAP_H__
#define __XBINMAP_BINMAP_H__
#include "xbase/x_target.h"

#include "xbase/x_idx_allocator.h"
#include "xbinmaps/bin.h"

namespace xcore 
{
	// 
	// binmap class
	// 
	class binmap_t
	{
	public:
		class allocator
		{
		public:
			virtual void*	allocate(xsize_t _size, u32 _alignment) = 0;
			virtual void	deallocate(void*) = 0;
		};

						binmap_t();
						~binmap_t();

		void			init(const u32   bins, allocator* _allocator);
		void			init(const bin_t& bin, allocator* _allocator);

		void			set(const bin_t& bin);
		void			reset(const bin_t& bin);

		void			clear();
		void			fill();

		bool			is_empty() const;
		bool			is_filled() const;

		bool			is_empty(const bin_t& bin) const;
		bool			is_filled(const bin_t& bin) const;

		bin_t			root() const;
		bin_t			cover(const bin_t& bin) const;
		bin_t			find_empty() const;
		bin_t			find_filled() const;
		bin_t			find_empty(bin_t start) const;

		size_t			total_size() const;
		void			status() const;

		static bin_t	find_complement(const binmap_t& destination, const binmap_t& source, const bin_t::uint_t twist);
		static bin_t	find_complement(const binmap_t& destination, const binmap_t& source, bin_t range, const bin_t::uint_t twist);

		static void		copy(binmap_t& destination, const binmap_t& source);
		static void		copy(binmap_t& destination, const binmap_t& source, const bin_t& range);

	private:
		bool			read_value1_at(bin_t) const;
		s32				write_value1_at(bin_t, bool);
		bool			update_value1_at(bin_t, bool);

		bool			read_value0_at(bin_t) const;
		s32				write_value0_at(bin_t, bool);
		bool			update_value0_at(bin_t, bool);

		allocator*		allocator_;

		bin_t			binmax_;
		bin_t			binroot_;
		u32				binmap1_size;
		xbyte*			binmap1_;				// the binmap with bit '0' = empty, bit '1' = full, parent = [left-child] & [right-child]
		u32				binmap0_size;
		xbyte*			binmap0_;				// the binmap with bit '0' = empty, bit '1' = full, parent = [left-child] | [right-child]
		u32				layerToOffset_[32];

		//static void		copy(binmap_t& destination, const pcell_t dref, const binmap_t& source, const pcell_t sref);
		//static void		_copy__range(binmap_t& destination, const binmap_t& source, const pcell_t sref, const bin_t sbin);

		// Find first additional bin in source
		//static bin_t	_find_complement(const bin_t& bin, const pcell_t dref, const binmap_t& destination, const pcell_t sref, const binmap_t& source, const u64 twist);
		//static bin_t	_find_complement(const bin_t& bin, const bitmap_t dbitmap, const pcell_t sref, const binmap_t& source, const u64 twist);
		//static bin_t	_find_complement(const bin_t& bin, const pcell_t dref, const binmap_t& destination, const bitmap_t sbitmap, const u64 twist);
		//static bin_t	_find_complement(const bin_t& bin, const bitmap_t dbitmap, const bitmap_t sbitmap, const bin_t::uint_t twist);

		binmap_t&		operator = (const binmap_t&);
						binmap_t(const binmap_t&);
	};

	inline void binmap_t::init(const u32   bins, allocator* _allocator)
	{
		init(bin_t(0, bins-1), _allocator);
	}

	/**
	* Return the current root of the binmap
	*/
	inline bin_t binmap_t::root() const
	{
		return binroot_;
	}

	/**
	* Get the value of bin_
	*/
	inline bool	binmap_t::read_value1_at(bin_t _bin) const
	{
		u32 const offset = (u32)_bin.layer_offset();
		xbyte const* byte = binmap1_ + layerToOffset_[_bin.layer()] + (offset >> 3);
		xbyte const bit = 0x80 >> (offset & 0x07);
		return (*byte & bit) == bit;
	}

	/**
	* Set the value of bin_
	*/
	inline s32 binmap_t::write_value1_at(bin_t _bin, bool _in_value)
	{
		u32 const offset = (u32)_bin.layer_offset();
		xbyte* byte = binmap1_ + layerToOffset_[_bin.layer()] + (offset >> 3);
		xbyte const bit = 0x80 >> (offset & 0x07);
		if (_in_value) *byte = *byte | bit;
		else *byte = *byte & ~bit;
		return 0;
	}

	/**
	* Update the value of bin_
	*/
	inline bool binmap_t::update_value1_at(bin_t _bin, bool _in_value)
	{
		u32 const offset = (u32)_bin.layer_offset();
		xbyte* byte = binmap1_ + layerToOffset_[_bin.layer()] + (offset >> 3);
		xbyte const bit = 0x80 >> (offset & 0x07);
		bool old_value = (*byte & bit) != 0;
		if (_in_value) *byte = *byte | bit;
		else *byte = *byte & ~bit;
		return old_value;
	}

	/**
	* Get the value of bin_
	*/
	inline bool	binmap_t::read_value0_at(bin_t _bin) const
	{
		ASSERT(!_bin.is_base());
		u32 const offset = (u32)_bin.layer_offset();
		xbyte const* byte = binmap0_ + layerToOffset_[_bin.layer()] + (offset >> 3);
		xbyte const bit = 0x80 >> (offset & 0x07);
		return (*byte & bit) == bit;
	}

	/**
	* Set the value of bin_
	*/
	inline s32 binmap_t::write_value0_at(bin_t _bin, bool _in_value)
	{
		ASSERT(!_bin.is_base());
		u32 const offset = (u32)_bin.layer_offset();
		xbyte* byte = binmap0_ + layerToOffset_[_bin.layer()] + (offset >> 3);
		xbyte const bit = 0x80 >> (offset & 0x07);
		if (_in_value) *byte = *byte | bit;
		else *byte = *byte & ~bit;
		return 0;
	}

	/**
	* Update the value of bin_
	*/
	inline bool binmap_t::update_value0_at(bin_t _bin, bool _in_value)
	{
		ASSERT(!_bin.is_base());
		u32 const offset = (u32)_bin.layer_offset();
		xbyte* byte = binmap0_ + layerToOffset_[_bin.layer()] + (offset >> 3);
		xbyte const bit = 0x80 >> (offset & 0x07);
		bool old_value = (*byte & bit) != 0;
		if (_in_value) *byte = *byte | bit;
		else *byte = *byte & ~bit;
		return old_value;
	}

} // namespace end

#endif // __XBINMAP_BINMAP_H__
