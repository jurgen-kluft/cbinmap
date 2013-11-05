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
		typedef s32 bitmap_t;
		typedef u32 ref_t;

						binmap_t(x_iidx_allocator* _cell_allocator);
						~binmap_t();

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

		static bin_t	find_complement(const binmap_t& destination, const binmap_t& source, const u64 twist);
		static bin_t	find_complement(const binmap_t& destination, const binmap_t& source, bin_t range, const u64 twist);

		static void		copy(binmap_t& destination, const binmap_t& source);
		static void		copy(binmap_t& destination, const binmap_t& source, const bin_t& range);

#pragma pack(push, 1)
		
		// Structure of cell halves
		struct half_t
		{
			union
			{
				bitmap_t	bitmap_;
				ref_t		ref_;
			};
		};

		// Structure of cells
		struct cell_t
		{
			enum eflags { eis_left_ref = 1, eis_right_ref = 2 };

			half_t		left_;
			half_t		right_;
			u32			flags_;

			bool		is_left_ref() const			{ return (flags_ & eis_left_ref) == eis_left_ref; }
			bool		is_right_ref() const		{ return (flags_ & eis_right_ref) == eis_right_ref; }

			void		set_is_left_ref(bool flag) 
			{
				if (flag) flags_ = flags_ | eis_left_ref;
				else flags_ = flags_ & ~eis_left_ref;
			}

			void set_is_right_ref(bool flag) 
			{
				if (flag) flags_ = flags_ | eis_right_ref;
				else flags_ = flags_ & ~eis_right_ref;
			}
		};

#pragma pack(pop)

	private:
		ref_t			_alloc_cell();
		ref_t			alloc_cell();
		void			free_cell(ref_t cell);

		bool			extend_root();
		void			pack_cells(ref_t* cells);

		x_iidx_allocator* allocator;

		class cell_access
		{
		public:
			inline				cell_access(x_iidx_allocator* _allocator) : _a(_allocator) {}

			inline cell_t&		operator[](u32 i) { cell_t* cell_ptr = (cell_t *)_a->to_ptr(i); return *cell_ptr; }
			inline cell_t const& operator[](u32 i) const { cell_t const* cell_ptr = (cell_t const*)_a->to_ptr(i); return *cell_ptr; }

		private:
			x_iidx_allocator*	_a;
		};
		cell_access		cell_;

		size_t			allocated_cells_number_;

		ref_t			root_ref_;
		bin_t			root_bin_;

		void			trace(ref_t* ref, bin_t* bin, const bin_t& target) const;
		void			trace(ref_t* ref, bin_t* bin, ref_t** history, const bin_t& target) const;

		void			_set__low_layer_bitmap(const bin_t& bin, const bitmap_t bitmap);
		void			_set__high_layer_bitmap(const bin_t& bin, const bitmap_t bitmap);

		static void		copy(binmap_t& destination, const ref_t dref, const binmap_t& source, const ref_t sref);
		static void		_copy__range(binmap_t& destination, const binmap_t& source, const ref_t sref, const bin_t sbin);

		// Find first additional bin in source
		static bin_t	_find_complement(const bin_t& bin, const ref_t dref, const binmap_t& destination, const ref_t sref, const binmap_t& source, const u64 twist);
		static bin_t	_find_complement(const bin_t& bin, const bitmap_t dbitmap, const ref_t sref, const binmap_t& source, const u64 twist);
		static bin_t	_find_complement(const bin_t& bin, const ref_t dref, const binmap_t& destination, const bitmap_t sbitmap, const u64 twist);
		static bin_t	_find_complement(const bin_t& bin, const bitmap_t dbitmap, const bitmap_t sbitmap, const u64 twist);

		binmap_t&		operator = (const binmap_t&);
						binmap_t(const binmap_t&);
	};

} // namespace end

#endif // __XBINMAP_BINMAP_H__
