#ifndef __XBINMAP_BINMAP_H__
#define __XBINMAP_BINMAP_H__
#include "xbase/x_target.h"

#include "xbase/x_idx_allocator.h"
#include "xbinmaps/bin.h"

namespace xcore 
{
	// 
	// const binmap class
	// 
	class const_binmap_t
	{
	public:
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

	protected:
						const_binmap_t();
						const_binmap_t(const const_binmap_t&) {}

		friend class ubinmap_t;

		bool			read_value1_at(bin_t) const;
		bool			read_value0_at(bin_t) const;

		bin_t*			binroot_;
		xbyte*			binmap1_;				// the AND binmap with bit '0' = empty, bit '1' = full, parent = [left-child] & [right-child]
		xbyte*			binmap0_;				// the  OR binmap with bit '0' = empty, bit '1' = full, parent = [left-child] | [right-child]
	};

	// 
	// binmap class
	// 
	class binmap_t: public const_binmap_t
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

		void			set(const bin_t& bin);
		void			reset(const bin_t& bin);

		void			clear();
		void			fill();

		void			status() const;

	protected:
		friend class ubinmap_t;

		s32				write_value1_at(bin_t, bool);
		bool			update_value1_at(bin_t, bool);

		s32				write_value0_at(bin_t, bool);
		bool			update_value0_at(bin_t, bool);

		void			exit();

		allocator*		allocator_;

		binmap_t&		operator = (const binmap_t&);
						binmap_t(const binmap_t&);
	};

	// 
	// binmap utility, static functions 
	// 
	class ubinmap_t
	{
	public:
		static bin_t	find_complement(const const_binmap_t& destination, const const_binmap_t& source, const bin_t::uint_t twist);
		static bin_t	find_complement(const const_binmap_t& destination, const const_binmap_t& source, bin_t range, const bin_t::uint_t twist);

		static void		copy(binmap_t& destination, const const_binmap_t& source);
		static void		copy(binmap_t& destination, const const_binmap_t& source, const bin_t& range);
	};

	/**
	* Return the current root of the binmap
	*/
	inline bin_t const_binmap_t::root() const
	{
		return binroot_!=NULL ? *binroot_ : bin_t::NONE;
	}

	/**
	* Get the value of bin_
	*/
	inline bool	const_binmap_t::read_value1_at(bin_t _bin) const
	{
		ASSERT(binroot_->contains(_bin));
		xbyte const* byte = binmap1_ + (_bin.value() >> 3);
		xbyte const  bit  = 0x80 >> (_bin.value() & 0x07);
		return (*byte & bit) == bit;
	}

	/**
	* Set the value of bin_
	*/
	inline s32 binmap_t::write_value1_at(bin_t _bin, bool _in_value)
	{
		ASSERT(binroot_->contains(_bin));
		xbyte     * byte = binmap1_ + (_bin.value() >> 3);
		xbyte const bit  = 0x80 >> (_bin.value() & 0x07);
		if (_in_value) *byte = *byte | bit;
		else *byte = *byte & ~bit;
		return 0;
	}

	/**
	* Update the value of bin_
	*/
	inline bool binmap_t::update_value1_at(bin_t _bin, bool _in_value)
	{
		ASSERT(binroot_->contains(_bin));
		xbyte     * byte = binmap1_ + (_bin.value() >> 3);
		xbyte const bit  = 0x80 >> (_bin.value() & 0x07);
		bool old_value = (*byte & bit) != 0;
		if (_in_value) *byte = *byte | bit;
		else *byte = *byte & ~bit;
		return old_value;
	}

	/**
	* Get the value of bin_
	*/
	inline bool	const_binmap_t::read_value0_at(bin_t _bin) const
	{
		ASSERT(binroot_->contains(_bin));
		xbyte const* byte = binmap0_ + (_bin.value() >> 3);
		xbyte const  bit  = 0x80 >> (_bin.value() & 0x07);
		return (*byte & bit) == bit;
	}

	/**
	* Set the value of bin_
	*/
	inline s32 binmap_t::write_value0_at(bin_t _bin, bool _in_value)
	{
		ASSERT(binroot_->contains(_bin));
		xbyte     * byte = binmap0_ + (_bin.value() >> 3);
		xbyte const bit  = 0x80 >> (_bin.value() & 0x07);
		if (_in_value) *byte = *byte | bit;
		else *byte = *byte & ~bit;
		return 0;
	}

	/**
	* Update the value of bin_
	*/
	inline bool binmap_t::update_value0_at(bin_t _bin, bool _in_value)
	{
		ASSERT(binroot_->contains(_bin));
		xbyte     * byte = binmap0_ + (_bin.value() >> 3);
		xbyte const bit  = 0x80 >> (_bin.value() & 0x07);
		bool old_value = (*byte & bit) != 0;
		if (_in_value) *byte = *byte | bit;
		else *byte = *byte & ~bit;
		return old_value;
	}

} // namespace end

#endif // __XBINMAP_BINMAP_H__
