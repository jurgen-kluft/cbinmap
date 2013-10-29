#ifndef __XBINMAP_BINMAP_H__
#define __XBINMAP_BINMAP_H__
#include "xbase/x_target.h"

#include "xbase/x_idx_allocator.h"
#include "xbinmaps/bin.h"

namespace xcore 
{

	/**
	* Binmap class
	*/
	class binmap_t
	{
	public:
		/** Type of bitmap */
		typedef s32 bitmap_t;
		/** Type of reference */
		typedef u32 ref_t;


		/**
		* Constructor
		*/
		binmap_t(x_iidx_allocator* _allocator);


		/**
		* Destructor
		*/
		~binmap_t();


		/**
		* Set the bin
		*/
		void set(const bin_t& bin);


		/**
		* Reset the bin
		*/
		void reset(const bin_t& bin);


		/**
		* Empty all bins
		*/
		void clear();


		/**
		* Ric: Fill all bins, size is given by the source's root
		*/
		void fill(const binmap_t& source);


		/**
		* Whether binmap is empty
		*/
		bool is_empty() const;


		/**
		* Whether binmap is filled
		*/
		bool is_filled() const;


		/**
		* Whether range/bin is empty
		*/
		bool is_empty(const bin_t& bin) const;


		/**
		* Whether range/bin is filled
		*/
		bool is_filled(const bin_t& bin) const;


		/**
		* Return the topmost solid bin which covers the specified bin
		*/
		bin_t cover(const bin_t& bin) const;


		/**
		* Find first empty bin
		*/
		bin_t find_empty() const;


		/**
		* Find first filled bin
		*/
		bin_t find_filled() const;

		/**
		* Arno: Find first empty bin right of start (start inclusive)
		*/
		bin_t find_empty(bin_t start) const;

		/**
		* Get number of allocated cells
		*/
		size_t cells_number() const;


		/**
		* Get total size of the binmap (Arno: =number of bytes it occupies in memory)
		*/
		size_t total_size() const;


		/**
		* Echo the binmap status to stdout
		*/
		void status() const;


		/**
		* Find first additional bin in source
		*/
		static bin_t find_complement(const binmap_t& destination, const binmap_t& source, const u32 twist);


		/**
		* Find first additional bin of the source inside specified range
		*/
		static bin_t find_complement(const binmap_t& destination, const binmap_t& source, bin_t range, const u32 twist);


		/**
		* Copy one binmap to another
		*/
		static void copy(binmap_t& destination, const binmap_t& source);


		/**
		* Copy a range from one binmap to another binmap
		*/
		static void copy(binmap_t& destination, const binmap_t& source, const bin_t& range);


	private:
#pragma pack(push, 1)

		/**
		* Structure of cell halves
		*/
		typedef struct 
		{
			union
			{
				bitmap_t bitmap_;
				ref_t ref_;
			};
		} half_t;

		/**
		* Structure of cells
		*/
		struct cell_t
		{
			half_t left_;
			half_t right_;
			bool is_left_ref_ : 1;
			bool is_right_ref_ : 1;
			bool is_free_ : 1;
		};

#pragma pack(pop)

	private:
		/** Allocates one cell (dirty allocation) */
		ref_t _alloc_cell();

		/** Allocates one cell */
		ref_t alloc_cell();

		/** Reserve cells allocation capacity */
		bool reserve_cells(size_t count);

		/** Releases the cell */
		void free_cell(ref_t cell);

		/** Extend root */
		bool extend_root();

		/** Pack a trace of cells */
		void pack_cells(ref_t* cells);

		/** Allocator used for allocating and freeing cells */
		x_iidx_allocator* allocator;

		/** Pointer to the list of blocks */
		cell_t* cell_;

		/** Number of available cells */
		size_t cells_number_;

		/** Number of allocated cells */
		size_t allocated_cells_number_;

		/** Front of the free cell list */
		ref_t free_top_;

		/** The root bin */
		bin_t root_bin_;


		/** Trace the bin */
		void trace(ref_t* ref, bin_t* bin, const bin_t& target) const;

		/** Trace the bin */
		void trace(ref_t* ref, bin_t* bin, ref_t** history, const bin_t& target) const;


		/** Sets low layer bitmap */
		void _set__low_layer_bitmap(const bin_t& bin, const bitmap_t bitmap);

		/** Sets high layer bitmap */
		void _set__high_layer_bitmap(const bin_t& bin, const bitmap_t bitmap);


		/** Clone binmap cells to another binmap */
		static void copy(binmap_t& destination, const ref_t dref, const binmap_t& source, const ref_t sref);

		static void _copy__range(binmap_t& destination, const binmap_t& source, const ref_t sref, const bin_t sbin);


		/** Find first additional bin in source */
		static bin_t _find_complement(const bin_t& bin, const ref_t dref, const binmap_t& destination, const ref_t sref, const binmap_t& source, const u32 twist);
		static bin_t _find_complement(const bin_t& bin, const bitmap_t dbitmap, const ref_t sref, const binmap_t& source, const u32 twist);
		static bin_t _find_complement(const bin_t& bin, const ref_t dref, const binmap_t& destination, const bitmap_t sbitmap, const u32 twist);
		static bin_t _find_complement(const bin_t& bin, const bitmap_t dbitmap, const bitmap_t sbitmap, const u32 twist);


		/* Disabled */
		binmap_t& operator = (const binmap_t&);

		/* Disabled */
		binmap_t(const binmap_t&);
	};

} // namespace end

#endif // __XBINMAP_BINMAP_H__
