#ifndef __XBINMAP_BINMAP_H__
#define __XBINMAP_BINMAP_H__
#include "xbase/x_target.h"

#include "xbase/x_idx_allocator.h"
#include "xbinmaps/bin.h"

namespace xcore 
{
	namespace binmaps
	{
		typedef		xbyte		byte;

		class iallocator
		{
		public:
			virtual void*	allocate(xsize_t _size, u32 _alignment) = 0;
			virtual void	deallocate(void*) = 0;
		};

		class data
		{
		public:
			bin_t					get_root() const				{ return root_; }
			byte*					get_data() const				{ return data_; }

			static u32				size_for(bin_t _root);

		protected:
			inline					data() : root_(bin_t::NONE), data_(0) {}
			inline					data(bin_t _root, xbyte* _data) : root_(_root), data_(_data) {}

			bin_t					root_;
			byte*					data_;
		};

		class user_data : public data
		{
		public:
			inline					user_data(bin_t _root, xbyte* _data) : data(_root, _data) {}
		};


		// 
		// const binmap class
		// 
		class cbinmap
		{
		public:
							cbinmap();
							cbinmap(data&);
							cbinmap(const cbinmap&);

			bin_t const&	root() const;

			bool			is_empty() const;
			bool			is_filled() const;

			bool			is_empty(const bin_t& bin) const;
			bool			is_filled(const bin_t& bin) const;

			bool			get(const bin_t& bin) const;
			bin_t			cover(const bin_t& bin) const;

			bin_t			find_empty() const;
			bin_t			find_filled() const;
			bin_t			find_empty(bin_t start) const;

			size_t			total_size() const;

			bool			read_am_at(bin_t) const;
			bool			read_om_at(bin_t) const;

		protected:
			friend class binmap;

			bin_t*			binroot_;
			xbyte*			binmap1_;				// the AND binmap with bit '0' = empty, bit '1' = full, parent = [left-child] & [right-child]
			xbyte*			binmap0_;				// the  OR binmap with bit '0' = empty, bit '1' = full, parent = [left-child] | [right-child]
		};

		// 
		// binmap class
		// 
		class binmap : public cbinmap
		{
		public:
							binmap();
							binmap(data&);
							binmap(const binmap&);

			void			clear();
			void			fill();

			void			set(const bin_t& bin);
			void			reset(const bin_t& bin);

			s32				write_am_at(bin_t, bool);
			bool			xchg_am_at(bin_t, bool);

			s32				write_om_at(bin_t, bool);
			bool			xchg_om_at(bin_t, bool);

			void			copy_from(cbinmap const&);

		protected:
			binmap&			operator = (const binmap&);
		};

		// 
		// binmap utility, static functions 
		// 
		extern bin_t	find_complement(const cbinmap& destination, const cbinmap& source, const bin_t::uint_t twist);
		extern bin_t	find_complement(const cbinmap& destination, const cbinmap& source, bin_t range, const bin_t::uint_t twist);

		extern void		copy(binmap& destination, const cbinmap& source);
		extern void		copy(binmap& destination, const cbinmap& source, const bin_t& range);

		/**
		* Return the current root of the binmap
		*/
		inline bin_t const& cbinmap::root() const
		{
			return binroot_!=NULL ? *binroot_ : bin_t::NONE;
		}

		/**
		* Get the value of bin_
		*/
		inline bool	cbinmap::read_am_at(bin_t _bin) const
		{
			ASSERT(binroot_->contains(_bin));
			xbyte const* byte = binmap1_ + (_bin.value() >> 3);
			xbyte const  bit  = 0x80 >> (_bin.value() & 0x07);
			return (*byte & bit) == bit;
		}

		/**
		* Set the value of bin_
		*/
		inline s32 binmap::write_am_at(bin_t _bin, bool _in_value)
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
		inline bool binmap::xchg_am_at(bin_t _bin, bool _in_value)
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
		inline bool	cbinmap::read_om_at(bin_t _bin) const
		{
			ASSERT(binroot_->contains(_bin));
			xbyte const* byte = binmap0_ + (_bin.value() >> 3);
			xbyte const  bit  = 0x80 >> (_bin.value() & 0x07);
			return (*byte & bit) == bit;
		}

		/**
		* Set the value of bin_
		*/
		inline s32 binmap::write_om_at(bin_t _bin, bool _in_value)
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
		inline bool binmap::xchg_om_at(bin_t _bin, bool _in_value)
		{
			ASSERT(binroot_->contains(_bin));
			xbyte     * byte = binmap0_ + (_bin.value() >> 3);
			xbyte const bit  = 0x80 >> (_bin.value() & 0x07);
			bool old_value = (*byte & bit) != 0;
			if (_in_value) *byte = *byte | bit;
			else *byte = *byte & ~bit;
			return old_value;
		}

	}

} // namespace end

#endif // __XBINMAP_BINMAP_H__
