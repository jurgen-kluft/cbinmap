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
		struct cell_t;
		typedef s32		bitmap_t;
		typedef cell_t*	pcell_t;

		class allocator
		{
		public:
			virtual void*	allocate(xsize_t _size, u32 _alignment) = 0;
			virtual void	deallocate(void*) = 0;
		};

						binmap_t();
						~binmap_t();

		void			init(u32 _max_bins, allocator* _cell_allocator);

		void			set(const bin_t& bin);
		void			reset(const bin_t& bin);
		void			clear();

		void			fill(const binmap_t& source);
		bool			is_empty() const;
		bool			is_filled() const;

		bool			is_empty(const bin_t& bin) const;
		bool			is_filled(const bin_t& bin) const;

		bin_t			cover(const bin_t& bin) const;
		bin_t			find_empty() const;
		bin_t			find_filled() const;
		bin_t			find_empty(bin_t start) const;
		size_t			cells_number() const;

		size_t			total_size() const;
		void			status() const;

		static bin_t	find_complement(const binmap_t& destination, const binmap_t& source, const bin_t::uint_t twist);
		static bin_t	find_complement(const binmap_t& destination, const binmap_t& source, bin_t range, const bin_t::uint_t twist);

		static void		copy(binmap_t& destination, const binmap_t& source);
		static void		copy(binmap_t& destination, const binmap_t& source, const bin_t& range);

		// Structure of cell halves
		struct half_t
		{
			union
			{
				bitmap_t	bitmap_;
				pcell_t		ref_;
			};
		};

		// Structure of cells
		struct cell_t
		{
			half_t		left_, right_;

			void		clear()								{ left_.ref_ = 0; right_.ref_ = 0; flags_.flags_ = 0; }

			bool		is_left_ref() const					{ return flags_.dir_.b_is_left_ref != 0; }
			bool		is_right_ref() const				{ return flags_.dir_.b_is_right_ref != 0; }

			void		set_is_left_ref(bool flag)			{ flags_.dir_.b_is_left_ref = flag ? 1 : 0; }
			void		set_is_right_ref(bool flag)			{ flags_.dir_.b_is_right_ref = flag ? 1 : 0; }

		private:
			struct dir { u16 b_is_left_ref; u16 b_is_right_ref; };
			union uflags { dir dir_; u32 flags_; };
			uflags		flags_;
		};

	private:
		pcell_t			alloc_cell();
		void			free_cell(pcell_t cell);

		bool			extend_root();
		void			pack_cells(pcell_t* cells);

		allocator*		cell_allocator;

		size_t			allocated_cells_number_;

		pcell_t			root_cell_;
		bin_t			root_bin_;

		void			trace(pcell_t* ref, bin_t* bin, const bin_t& target) const;
		void			trace(pcell_t* ref, bin_t* bin, pcell_t** history, const bin_t& target) const;

		void			_set__low_layer_bitmap(const bin_t& bin, const bitmap_t bitmap);
		void			_set__high_layer_bitmap(const bin_t& bin, const bitmap_t bitmap);

		static void		copy(binmap_t& destination, const pcell_t dref, const binmap_t& source, const pcell_t sref);
		static void		_copy__range(binmap_t& destination, const binmap_t& source, const pcell_t sref, const bin_t sbin);

		// Find first additional bin in source
		static bin_t	_find_complement(const bin_t& bin, const pcell_t dref, const binmap_t& destination, const pcell_t sref, const binmap_t& source, const u64 twist);
		static bin_t	_find_complement(const bin_t& bin, const bitmap_t dbitmap, const pcell_t sref, const binmap_t& source, const u64 twist);
		static bin_t	_find_complement(const bin_t& bin, const pcell_t dref, const binmap_t& destination, const bitmap_t sbitmap, const u64 twist);
		static bin_t	_find_complement(const bin_t& bin, const bitmap_t dbitmap, const bitmap_t sbitmap, const bin_t::uint_t twist);

		binmap_t&		operator = (const binmap_t&);
						binmap_t(const binmap_t&);
	};

} // namespace end

#endif // __XBINMAP_BINMAP_H__
