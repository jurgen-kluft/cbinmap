#ifndef __XBINMAP_BINMAP_H__
#define __XBINMAP_BINMAP_H__
#include "cbase/c_target.h"

#include "cbase/c_debug.h"
#include "cbinmaps/bin.h"

namespace ncore 
{
	namespace binmaps
	{
		typedef		u8		byte;

		class data
		{
		public:
			bin_t					get_root() const				{ return root_; }
			byte*					get_data() const				{ return data_; }

			static u32				size_for(bin_t _root);

		protected:
			inline					data() : root_(bin_t::NONE), data_(0) {}
			inline					data(bin_t _root, u8* _data) : root_(_root), data_(_data) {}

			bin_t					root_;
			byte*					data_;
		};

		class user_data : public data
		{
		public:
			inline					user_data(bin_t _root, u8* _data) : data(_root, _data) {}
		};


		// 
		// const binmap class
		// 
		class cbinmap
		{
		public:
							cbinmap();
							cbinmap(user_data);
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

			uint_t			total_size() const;

			bool			read_am_at(bin_t) const;
			bool			read_om_at(bin_t) const;

		protected:
			friend class binmap;

			bin_t*			binroot_;
			u8*			binmap1_;				// the AND binmap with bit '0' = empty, bit '1' = full, parent = [left-child] & [right-child]
			u8*			binmap0_;				// the  OR binmap with bit '0' = empty, bit '1' = full, parent = [left-child] | [right-child]
		};

		// 
		// binmap class
		// 
		class binmap : public cbinmap
		{
		public:
							binmap();
							binmap(user_data);
							binmap(data&);
							binmap(const binmap&);

			void			clear();
			void			fill();

			void			set(const bin_t& bin);
			void			reset(const bin_t& bin);

			void			copy_from(cbinmap const&);

		protected:
			s32				write_am_at(bin_t, bool);
			bool			xchg_am_at(bin_t, bool);

			s32				write_om_at(bin_t, bool);
			bool			xchg_om_at(bin_t, bool);

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
			return binroot_!=nullptr ? *binroot_ : bin_t::NONE;
		}

		/**
		* Get the value of bin_
		*/
		inline bool	cbinmap::read_am_at(bin_t _bin) const
		{
			ASSERT(binroot_->contains(_bin));
			u8 const* byte = binmap1_ + (_bin.value() >> 3);
			u8 const  bit  = 0x80 >> (_bin.value() & 0x07);
			return (*byte & bit) == bit;
		}

		/**
		* Set the value of bin_
		*/
		inline s32 binmap::write_am_at(bin_t _bin, bool _in_value)
		{
			ASSERT(binroot_->contains(_bin));
			u8     * byte = binmap1_ + (_bin.value() >> 3);
			u8 const bit  = 0x80 >> (_bin.value() & 0x07);
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
			u8     * byte = binmap1_ + (_bin.value() >> 3);
			u8 const bit  = 0x80 >> (_bin.value() & 0x07);
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
			u8 const* byte = binmap0_ + (_bin.value() >> 3);
			u8 const  bit  = 0x80 >> (_bin.value() & 0x07);
			return (*byte & bit) == bit;
		}

		/**
		* Set the value of bin_
		*/
		inline s32 binmap::write_om_at(bin_t _bin, bool _in_value)
		{
			ASSERT(binroot_->contains(_bin));
			u8     * byte = binmap0_ + (_bin.value() >> 3);
			u8 const bit  = 0x80 >> (_bin.value() & 0x07);
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
			u8     * byte = binmap0_ + (_bin.value() >> 3);
			u8 const bit  = 0x80 >> (_bin.value() & 0x07);
			bool old_value = (*byte & bit) != 0;
			if (_in_value) *byte = *byte | bit;
			else *byte = *byte & ~bit;
			return old_value;
		}

	}

} // namespace end

#endif // __XBINMAP_BINMAP_H__
