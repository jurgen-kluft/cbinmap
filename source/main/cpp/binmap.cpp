#include "xbase/x_debug.h"
#include "xbase/x_memory_std.h"
#include "xbase/x_string_std.h"

#include "xbinmaps/binmap.h"

namespace xcore
{
	inline size_t _max_(const size_t x, const size_t y)
	{
		return x < y ? y : x;
	}

	typedef binmap_t::pcell_t pcell_t;
	typedef binmap_t::bitmap_t bitmap_t;

	/* Bitmap constants */
	const bitmap_t BITMAP_EMPTY  = static_cast<bitmap_t>(0);
	const bitmap_t BITMAP_FILLED = static_cast<bitmap_t>(-1);

	const bin_t::uint_t BITMAP_LAYER_BITS = 2 * 8 * sizeof(bitmap_t) - 1;

#ifdef _MSC_VER
#  pragma warning (push)
#  pragma warning ( disable:4309 )
#endif

	const bitmap_t BITMAP[] =
	{
		static_cast<bitmap_t>(0x00000001), static_cast<bitmap_t>(0x00000003),
		static_cast<bitmap_t>(0x00000002), static_cast<bitmap_t>(0x0000000f),
		static_cast<bitmap_t>(0x00000004), static_cast<bitmap_t>(0x0000000c),
		static_cast<bitmap_t>(0x00000008), static_cast<bitmap_t>(0x000000ff),
		static_cast<bitmap_t>(0x00000010), static_cast<bitmap_t>(0x00000030),
		static_cast<bitmap_t>(0x00000020), static_cast<bitmap_t>(0x000000f0),
		static_cast<bitmap_t>(0x00000040), static_cast<bitmap_t>(0x000000c0),
		static_cast<bitmap_t>(0x00000080), static_cast<bitmap_t>(0x0000ffff),
		static_cast<bitmap_t>(0x00000100), static_cast<bitmap_t>(0x00000300),
		static_cast<bitmap_t>(0x00000200), static_cast<bitmap_t>(0x00000f00),
		static_cast<bitmap_t>(0x00000400), static_cast<bitmap_t>(0x00000c00),
		static_cast<bitmap_t>(0x00000800), static_cast<bitmap_t>(0x0000ff00),
		static_cast<bitmap_t>(0x00001000), static_cast<bitmap_t>(0x00003000),
		static_cast<bitmap_t>(0x00002000), static_cast<bitmap_t>(0x0000f000),
		static_cast<bitmap_t>(0x00004000), static_cast<bitmap_t>(0x0000c000),
		static_cast<bitmap_t>(0x00008000), static_cast<bitmap_t>(0xffffffff),
		static_cast<bitmap_t>(0x00010000), static_cast<bitmap_t>(0x00030000),
		static_cast<bitmap_t>(0x00020000), static_cast<bitmap_t>(0x000f0000),
		static_cast<bitmap_t>(0x00040000), static_cast<bitmap_t>(0x000c0000),
		static_cast<bitmap_t>(0x00080000), static_cast<bitmap_t>(0x00ff0000),
		static_cast<bitmap_t>(0x00100000), static_cast<bitmap_t>(0x00300000),
		static_cast<bitmap_t>(0x00200000), static_cast<bitmap_t>(0x00f00000),
		static_cast<bitmap_t>(0x00400000), static_cast<bitmap_t>(0x00c00000),
		static_cast<bitmap_t>(0x00800000), static_cast<bitmap_t>(0xffff0000),
		static_cast<bitmap_t>(0x01000000), static_cast<bitmap_t>(0x03000000),
		static_cast<bitmap_t>(0x02000000), static_cast<bitmap_t>(0x0f000000),
		static_cast<bitmap_t>(0x04000000), static_cast<bitmap_t>(0x0c000000),
		static_cast<bitmap_t>(0x08000000), static_cast<bitmap_t>(0xff000000),
		static_cast<bitmap_t>(0x10000000), static_cast<bitmap_t>(0x30000000),
		static_cast<bitmap_t>(0x20000000), static_cast<bitmap_t>(0xf0000000),
		static_cast<bitmap_t>(0x40000000), static_cast<bitmap_t>(0xc0000000),
		static_cast<bitmap_t>(0x80000000), /* special */ static_cast<bitmap_t>(0xffffffff) /* special */
	};

#ifdef _MSC_VER
#pragma warning (pop)
#endif

	/**
	* Get the leftmost bin that corresponded to bitmap (the bin is filled in bitmap)
	*/
	bin_t::uint_t bitmap_to_bin(bitmap_t b)
	{
		static const u8 BITMAP_TO_BIN[] =
		{
			0xff, 
			    0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
			 8, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
			10, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
			 9, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
			12, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
			 8, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
			10, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
			 9, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
			14, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
			 8, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
			10, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
			 9, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
			13, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
			 8, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
			10, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 3,
			11, 0, 2, 1, 4, 0, 2, 1, 6, 0, 2, 1, 5, 0, 2, 7
		};

		ASSERT (sizeof(bitmap_t) == 4);
		ASSERT (b != BITMAP_EMPTY);

		u8 t;

		t = BITMAP_TO_BIN[ b & 0xff ];
		if (t < 16)
		{
			if (t != 7)
				return static_cast<bin_t::uint_t>(t);

			b += 1;
			b &= -b;
			if (0 == b)
				return BITMAP_LAYER_BITS / 2;
			if (0 == (b & 0xffff))
				return 15;
			return 7;
		}

		b >>= 8;
		t = BITMAP_TO_BIN[ b & 0xff ];
		if (t <= 15)
		{
			return 16 + t;
		}

		/* Recursion */
		// return 32 + bitmap_to_bin( b >> 8 );

		b >>= 8;
		t = BITMAP_TO_BIN[ b & 0xff ];
		if (t < 16)
		{
			if (t != 7)
				return 32 + static_cast<bin_t::uint_t>(t);

			b += 1;
			b &= -b;
			if (0 == (b & 0xffff))
				return 47;
			return 39;
		}

		b >>= 8;
		return 48 + BITMAP_TO_BIN[ b & 0xff ];
	}


	/**
	* Get the leftmost bin that corresponded to bitmap (the bin is filled in bitmap)
	*/
	bin_t bitmap_to_bin(const bin_t& bin, const bitmap_t bitmap)
	{
		ASSERT (bitmap != BITMAP_EMPTY);

		if (bitmap == BITMAP_FILLED)
		{
			return bin;
		}

		return bin_t(bin.base_left().value() + bitmap_to_bin(bitmap));
	}


	/* Methods */


	/**
	* Constructor
	*/
	binmap_t::binmap_t()
		: cell_allocator(0)
		, root_bin_(63)
	{
		ASSERT (sizeof(bitmap_t) <= 4);

		allocated_cells_number_ = 0;
		root_cell_ = NULL;
	}


	/**
	* Destructor
	*/
	binmap_t::~binmap_t()
	{
		if (root_cell_ != NULL)
		{
			free_cell(root_cell_);
			root_cell_ = NULL;
		}
	}

	/**
	* Allocates one cell (dirty allocation)
	*/
	void binmap_t::init(u32 _max_bins, allocator* _cell_allocator)
	{
		cell_allocator = _cell_allocator;
		root_cell_ = alloc_cell();
	}

	/**
	* Allocates one cell
	*/
	pcell_t binmap_t::alloc_cell()
	{
		++allocated_cells_number_;
		void *p = cell_allocator->allocate(sizeof(cell_t), sizeof(X_PTR_SIZED_INT));
		ASSERT(p!=NULL);
		pcell_t cell = (pcell_t)p;
		cell->clear();
		return cell;
	}

	/**
	* Free the cell including all children
	*/
	void binmap_t::free_cell(pcell_t ref)
	{
		ASSERT(ref != NULL);

		if (ref->is_left_ref())
			free_cell(ref->left_.ref_);

		if (ref->is_right_ref())
			free_cell(ref->right_.ref_);

		cell_allocator->deallocate(ref);
		--allocated_cells_number_;
	}

	/**
	* Extend root
	*/
	bool binmap_t::extend_root()
	{
		ASSERT (!root_bin_.is_all());

		if (!root_cell_->is_left_ref() && !root_cell_->is_right_ref() && root_cell_->left_.bitmap_ == root_cell_->right_.bitmap_)
		{
			/* Setup the root cell */
			root_cell_->right_.bitmap_ = BITMAP_EMPTY;

		} 
		else 
		{
			/* Allocate new cell */
			pcell_t ref = alloc_cell();

			/* Swap old root cell with the newly allocated cell */
			*ref = *root_cell_;

			/* Setup new root */
			root_cell_->set_is_left_ref(true);
			root_cell_->set_is_right_ref(false);

			root_cell_->left_.ref_ = ref;
			root_cell_->right_.bitmap_ = BITMAP_EMPTY;
		}

		/* Reset bin */
		root_bin_.to_parent();
		return true;
	}

	/**
	* Pack a trace of cells
	*/
	void binmap_t::pack_cells(pcell_t* href)
	{
		pcell_t ref = *href--;
		if (ref == root_cell_) 
		{
			return;
		}

		if (ref->is_left_ref() || ref->is_right_ref() || ref->left_.bitmap_ != ref->right_.bitmap_)
		{
			return;
		}

		const bitmap_t bitmap = ref->left_.bitmap_;

		do 
		{
			ref = *href--;

			if (!ref->is_left_ref())
			{
				if (ref->left_.bitmap_ != bitmap)
				{
					break;
				}
			}
			else if (!ref->is_right_ref()) 
			{
				if (ref->right_.bitmap_ != bitmap)
				{
					break;
				}
			}
			else
			{
				break;
			}

		} while (ref != root_cell_);

		pcell_t child = href[2];
		
		if (ref->is_left_ref() && ref->left_.ref_ == child)
		{
			ref->set_is_left_ref(false);
			ref->left_.bitmap_ = bitmap;
		} 
		else
		{
			ASSERT(ref->right_.ref_ == child);
			ref->set_is_right_ref(false);
			ref->right_.bitmap_ = bitmap;
		}

		href[2] = NULL;
		free_cell(child);
	}

	/**
	* Whether binmap is empty
	*/
	bool binmap_t::is_empty() const
	{
		pcell_t cell = root_cell_;
		return !cell->is_left_ref() && !cell->is_right_ref() && cell->left_.bitmap_ == BITMAP_EMPTY && cell->right_.bitmap_ == BITMAP_EMPTY;
	}


	/**
	* Whether binmap is filled
	*/
	bool binmap_t::is_filled() const
	{
		cell_t* cell = root_cell_;
		return root_bin_.is_all() && !cell->is_left_ref() && !cell->is_right_ref() && cell->left_.bitmap_ == BITMAP_FILLED && cell->right_.bitmap_ == BITMAP_FILLED;
	}


	/**
	* Whether range/bin is empty
	*/
	bool binmap_t::is_empty(const bin_t& bin) const
	{
		/* Process hi-layers case */
		if (!root_bin_.contains(bin))
		{
			return !bin.contains(root_bin_) || is_empty();
		}

		/* Trace the bin */
		pcell_t cur_ref;
		bin_t cur_bin;

		trace(&cur_ref, &cur_bin, bin);

		ASSERT (cur_bin.layer_bits() > BITMAP_LAYER_BITS);

		/* Process common case */
		cell_t* cell = cur_ref;

		if (bin.layer_bits() > BITMAP_LAYER_BITS) 
		{
			if (bin < cur_bin) 
			{
				return cell->left_.bitmap_ == BITMAP_EMPTY;
			}
			if (cur_bin < bin)
			{
				return cell->right_.bitmap_ == BITMAP_EMPTY;
			}
			return !cell->is_left_ref() && !cell->is_right_ref() && cell->left_.bitmap_ == BITMAP_EMPTY && cell->right_.bitmap_ == BITMAP_EMPTY;
		}

		/* Process low-layers case */
		ASSERT (bin != cur_bin);

		const bitmap_t bm1 = (bin < cur_bin) ? cell->left_.bitmap_ : cell->right_.bitmap_;
		const bitmap_t bm2 = BITMAP[ BITMAP_LAYER_BITS & bin.value() ];

		return (bm1 & bm2) == BITMAP_EMPTY;
	}


	/**
	* Whether range/bin is filled
	*/
	bool binmap_t::is_filled(const bin_t& bin) const
	{
		/* Process hi-layers case */
		if (!root_bin_.contains(bin)) 
		{
			return false;
		}

		/* Trace the bin */
		pcell_t cur_ref;
		bin_t cur_bin;

		trace(&cur_ref, &cur_bin, bin);

		ASSERT (cur_bin.layer_bits() > BITMAP_LAYER_BITS);

		/* Process common case */
		cell_t* cell = cur_ref;

		if (bin.layer_bits() > BITMAP_LAYER_BITS)
		{
			if (bin < cur_bin) 
			{
				return cell->left_.bitmap_ == BITMAP_FILLED;
			}
			if (cur_bin < bin) 
			{
				return cell->right_.bitmap_ == BITMAP_FILLED;
			}
			return !cell->is_left_ref() && !cell->is_right_ref() && cell->left_.bitmap_ == BITMAP_FILLED && cell->right_.bitmap_ == BITMAP_FILLED;
		}

		/* Process low-layers case */
		ASSERT (bin != cur_bin);

		const bitmap_t bm1 = (bin < cur_bin) ? cell->left_.bitmap_ : cell->right_.bitmap_;
		const bitmap_t bm2 = BITMAP[ BITMAP_LAYER_BITS & bin.value() ];

		return (bm1 & bm2) == bm2;
	}


	/**
	* Return the topmost solid bin which covers the specified bin
	*/
	bin_t binmap_t::cover(const bin_t& bin) const
	{
		/* Process hi-layers case */
		if (!root_bin_.contains(bin))
		{
			if (!bin.contains(root_bin_)) 
			{
				return root_bin_.sibling();
			}
			if (is_empty())
			{
				return bin_t::ALL;
			}
			return bin_t::NONE;
		}

		/* Trace the bin */
		pcell_t cur_ref;
		bin_t cur_bin;

		trace(&cur_ref, &cur_bin, bin);

		ASSERT (cur_bin.layer_bits() > BITMAP_LAYER_BITS);

		/* Process common case */
		cell_t* cell = cur_ref;

		if (bin.layer_bits() > BITMAP_LAYER_BITS)
		{
			if (bin < cur_bin) 
			{
				if (cell->left_.bitmap_ == BITMAP_EMPTY || cell->left_.bitmap_ == BITMAP_FILLED)
				{
					return cur_bin.left();
				}
				return bin_t::NONE;
			}
			if (cur_bin < bin) 
			{
				if (cell->right_.bitmap_ == BITMAP_EMPTY || cell->right_.bitmap_ == BITMAP_FILLED)
				{
					return cur_bin.right();
				}
				return bin_t::NONE;
			}
			if (cell->is_left_ref() || cell->is_right_ref())
			{
				return bin_t::NONE;
			}
			if (cell->left_.bitmap_ != cell->right_.bitmap_) 
			{
				return bin_t::NONE;
			}
			ASSERT (cur_bin == root_bin_);
			if (cell->left_.bitmap_ == BITMAP_EMPTY)
			{
				return bin_t::ALL;
			}
			if (cell->left_.bitmap_ == BITMAP_FILLED)
			{
				return cur_bin;
			}
			return bin_t::NONE;
		}

		/* Process low-layers case */
		ASSERT (bin != cur_bin);

		bitmap_t bm1;
		if (bin < cur_bin) 
		{
			bm1 = cell->left_.bitmap_;
			cur_bin.to_left();
		}
		else
		{
			bm1 = cell->right_.bitmap_;
			cur_bin.to_right();
		}

		if (bm1 == BITMAP_EMPTY) 
		{
			if (is_empty())
			{
				return bin_t::ALL;
			}
			return cur_bin;
		}
		if (bm1 == BITMAP_FILLED) 
		{
			if (is_filled())
			{
				return bin_t::ALL;
			}
			return cur_bin;
		}

		/* Trace the bitmap */
		bin_t b = bin;
		bitmap_t bm2 = BITMAP[ BITMAP_LAYER_BITS & b.value() ];

		if ((bm1 & bm2) == BITMAP_EMPTY) 
		{
			do 
			{
				cur_bin = b;
				b.to_parent();
				bm2 = BITMAP[ BITMAP_LAYER_BITS & b.value() ];
			} while ((bm1 & bm2) == BITMAP_EMPTY);

			return cur_bin;

		} 
		else if ((bm1 & bm2) == bm2)
		{
			do
			{
				cur_bin = b;
				b.to_parent();
				bm2 = BITMAP[ BITMAP_LAYER_BITS & b.value() ];
			} while ((bm1 & bm2) == bm2);

			return cur_bin;
		}

		return bin_t::NONE;
	}


	/**
	* Find first empty bin
	*/
	bin_t binmap_t::find_empty() const
	{
		/* Trace the bin */
		bitmap_t bitmap = BITMAP_FILLED;

		pcell_t cur_ref;
		bin_t cur_bin;

		do
		{
			/* Processing the root */
			if (root_cell_->is_left_ref())
			{
				cur_ref = root_cell_->left_.ref_;
				cur_bin = root_bin_.left();
			}
			else if (root_cell_->left_.bitmap_ != BITMAP_FILLED)
			{
				if ( root_cell_->left_.bitmap_ == BITMAP_EMPTY)
				{
					if (! root_cell_->is_right_ref() &&  root_cell_->right_.bitmap_ == BITMAP_EMPTY)
					{
						return bin_t::ALL;
					}
					return root_bin_.left();
				}
				bitmap = root_cell_->left_.bitmap_;
				cur_bin = root_bin_.left();
				break;
			} 
			else if (root_cell_->is_right_ref()) 
			{
				cur_ref = root_cell_->right_.ref_;
				cur_bin = root_bin_.right();
			}
			else
			{
				if (root_cell_->right_.bitmap_ == BITMAP_FILLED)
				{
					if (root_bin_.is_all())
					{
						return bin_t::NONE;
					}
					return root_bin_.sibling();
				}
				bitmap = root_cell_->right_.bitmap_;
				cur_bin = root_bin_.right();
				break;
			}

			/* Processing middle layers */
			for ( ;;)
			{
				if (cur_ref->is_left_ref())
				{
					cur_ref = cur_ref->left_.ref_;
					cur_bin.to_left();
				} 
				else if (cur_ref->left_.bitmap_ != BITMAP_FILLED)
				{
					bitmap = cur_ref->left_.bitmap_;
					cur_bin.to_left();
					break;
				}
				else if (cur_ref->is_right_ref())
				{
					cur_ref = cur_ref->right_.ref_;
					cur_bin.to_right();
				}
				else
				{
					ASSERT (cur_ref->right_.bitmap_ != BITMAP_FILLED);
					bitmap = cur_ref->right_.bitmap_;
					cur_bin.to_right();
					break;
				}
			}
		} while (false);

		/* Getting result */
		ASSERT (bitmap != BITMAP_FILLED);

		return bitmap_to_bin(cur_bin, ~bitmap);
	}


	/**
	* Find first filled bin
	*/
	bin_t binmap_t::find_filled() const
	{
		/* Trace the bin */
		bitmap_t bitmap = BITMAP_EMPTY;

		pcell_t cur_ref;
		bin_t cur_bin;

		do
		{
			/* Processing the root */
			if (root_cell_->is_left_ref()) 
			{
				cur_ref = root_cell_->left_.ref_;
				cur_bin = root_bin_.left();
			}
			else if (root_cell_->left_.bitmap_ != BITMAP_EMPTY)
			{
				if ( root_cell_->left_.bitmap_ == BITMAP_FILLED)
				{
					if (! root_cell_->is_right_ref() &&  root_cell_->right_.bitmap_ == BITMAP_FILLED)
					{
						return root_bin_;
					}
					return root_bin_.left();
				}
				bitmap = root_cell_->left_.bitmap_;
				cur_bin = root_bin_.left();
				break;
			}
			else if (root_cell_->is_right_ref())
			{
				cur_ref = root_cell_->right_.ref_;
				cur_bin = root_bin_.right();
			}
			else
			{
				if (root_cell_->right_.bitmap_ == BITMAP_EMPTY)
				{
					return bin_t::NONE;
				}
				bitmap = root_cell_->right_.bitmap_;
				cur_bin = root_bin_.right();
				break;
			}

			/* Processing middle layers */
			for ( ;;)
			{
				if (cur_ref->is_left_ref()) 
				{
					cur_ref = cur_ref->left_.ref_;
					cur_bin.to_left();
				} 
				else if (cur_ref->left_.bitmap_ != BITMAP_EMPTY)
				{
					bitmap = cur_ref->left_.bitmap_;
					cur_bin.to_left();
					break;
				} 
				else if (cur_ref->is_right_ref())
				{
					cur_ref = cur_ref->right_.ref_;
					cur_bin.to_right();
				}
				else 
				{
					ASSERT (cur_ref->right_.bitmap_ != BITMAP_EMPTY);
					bitmap = cur_ref->right_.bitmap_;
					cur_bin.to_right();
					break;
				}
			}

		} while (false);

		/* Getting result */
		ASSERT (bitmap != BITMAP_EMPTY);

		return bitmap_to_bin(cur_bin, bitmap);
	}


	/**
	* Arno: Find first empty bin right of start (start inclusive)
	*/
	bin_t binmap_t::find_empty(bin_t start) const
	{
		bin_t cur_bin = start;

		if (is_empty(cur_bin))
			return cur_bin;

		do
		{
			// Move up till we find ancestor that is not filled.
			cur_bin = cur_bin.parent();
			if (!is_filled(cur_bin))
			{
				// Ancestor is not filled
				break;
			}
			if (cur_bin == root_bin_)
			{
				// Hit top, full tree, sort of. For some reason root_bin_ not
				// set to real top (but to ALL), so we may actually return a
				// bin that is outside the size of the content here.
				return bin_t::NONE;
			}
		}
		while (true);

		// Move down
		do
		{
			if (!is_filled(cur_bin.left()))
			{
				cur_bin.to_left();
			}
			else if (!is_filled(cur_bin.right()))
			{
				cur_bin.to_right();
			}
			if (cur_bin.is_base())
			{
				// Found empty bin
				return cur_bin;
			}
		} while(!cur_bin.is_base()); // safety catch

		return bin_t::NONE;
	}



	#define LR_LEFT   (0x00)
	#define RL_RIGHT  (0x01)
	#define RL_LEFT   (0x02)
	#define LR_RIGHT  (0x03)


	#define SSTACK()                                    \
		s32 _top_ = 0;                                  \
		bin_t _bin_[64];                                \
		pcell_t _sref_[64];                             \
		s8 _dir_[64];

	#define DSTACK()                                    \
		s32 _top_ = 0;                                  \
		bin_t _bin_[64];                                \
		pcell_t _dref_[64];                             \
		s8 _dir_[64];

	#define SDSTACK()                                   \
		s32 _top_ = 0;                                  \
		bin_t _bin_[64];                                \
		pcell_t _sref_[64];                             \
		pcell_t _dref_[64];                             \
		s8 _dir_[64];


	#define SPUSH(b, sr, twist)                         \
		do {                                            \
		_bin_[_top_] = b;                               \
		_sref_[_top_] = sr;                             \
		_dir_[_top_] = (0 != (twist & (b.base_length() >> 1))); \
		++_top_;                                        \
		} while (false)

	#define DPUSH(b, dr, twist)                         \
		do {                                            \
		_bin_[_top_] = b;                               \
		_dref_[_top_] = dr;                             \
		_dir_[_top_] = (0 != (twist & (b.base_length() >> 1))); \
		++_top_;                                        \
		} while (false)

	#define SDPUSH(b, sr, dr, twist)                    \
		do {                                            \
		_bin_[_top_] = b;                               \
		_sref_[_top_] = sr;                             \
		_dref_[_top_] = dr;                             \
		_dir_[_top_] = (0 != (twist & (b.base_length() >> 1))); \
		++_top_;                                        \
		} while (false)


	#define SPOP()                                      \
		ASSERT (_top_ < 65);                            \
		--_top_;                                        \
		const bin_t b = _bin_[_top_];                   \
		cell_t* sc = _sref_[_top_];						\
		const bool is_left = !(_dir_[_top_] & 0x01);    \
		if (0 == (_dir_[_top_] & 0x02)) {               \
		_dir_[_top_++] ^= 0x03;							\
		}

	#define DPOP()                                      \
		ASSERT (_top_ < 65);                            \
		--_top_;                                        \
		const bin_t b = _bin_[_top_];                   \
		cell_t* dc = _dref_[_top_];						\
		const bool is_left = !(_dir_[_top_] & 0x01);    \
		if (0 == (_dir_[_top_] & 0x02)) {               \
		_dir_[_top_++] ^= 0x03;							\
		}

	#define SDPOP()                                     \
		ASSERT (_top_ < 65);                            \
		--_top_;                                        \
		const bin_t b = _bin_[_top_];                   \
		cell_t* sc = _sref_[_top_];                     \
		cell_t* dc = _dref_[_top_];                     \
		const bool is_left = !(_dir_[_top_] & 0x01);    \
		if (0 == (_dir_[_top_] & 0x02)) {               \
		    _dir_[_top_++] ^= 0x03;                     \
		}


	/**
	* Find first additional bin in source
	*
	* @param destination
	*             the destination binmap
	* @param source
	*             the source binmap
	*/
	bin_t binmap_t::find_complement(const binmap_t& destination, const binmap_t& source, const bin_t::uint_t twist)
	{
		return find_complement(destination, source, bin_t::ALL, twist);
	}


	bin_t binmap_t::find_complement(const binmap_t& destination, const binmap_t& source, bin_t range, const bin_t::uint_t twist)
	{
		pcell_t sref = source.root_cell_;
		bitmap_t sbitmap = BITMAP_EMPTY;
		bool is_sref = true;

		if (range.contains(source.root_bin_)) 
		{
			range = source.root_bin_;
			is_sref = true;
		}
		else if (source.root_bin_.contains(range))
		{
			bin_t sbin;
			source.trace(&sref, &sbin, range);

			if (range == sbin) 
			{
				is_sref = true;
			}
			else
			{
				is_sref = false;

				if (range < sbin) 
				{
					sbitmap = sref->left_.bitmap_;
				} 
				else 
				{
					sbitmap = sref->right_.bitmap_;
				}

				sbitmap &= BITMAP[ BITMAP_LAYER_BITS & range.value() ];

				if (sbitmap == BITMAP_EMPTY) 
				{
					return bin_t::NONE;
				}
			}

		}
		else
		{
			return bin_t::NONE;
		}

		ASSERT (is_sref || sbitmap != BITMAP_EMPTY);

		if (destination.is_empty())
		{
			if (is_sref)
			{
				cell_t* cell = sref;
				if (!cell->is_left_ref() && !cell->is_right_ref() && cell->left_.bitmap_ == BITMAP_FILLED && cell->right_.bitmap_ == BITMAP_FILLED) 
				{
					return range;
				} 
				else
				{
					return _find_complement(range, BITMAP_EMPTY, sref, source, twist);
				}
			}
			else 
			{
				return _find_complement(range, BITMAP_EMPTY, sbitmap, twist);
			}
		}

		if (destination.root_bin_.contains(range)) 
		{
			pcell_t dref;
			bin_t dbin;
			destination.trace(&dref, &dbin, range);

			if (range == dbin)
			{
				if (is_sref) 
				{
					return _find_complement(range, dref, destination, sref, source, twist);
				} 
				else
				{
					return _find_complement(range, dref, destination, sbitmap, twist);
				}

			}
			else 
			{
				bitmap_t dbitmap;

				if (range < dbin)
				{
					dbitmap = dref->left_.bitmap_;
				} 
				else 
				{
					dbitmap = dref->right_.bitmap_;
				}

				if (dbitmap == BITMAP_FILLED)
				{
					return bin_t::NONE;

				} else if (is_sref)
				{
					if (dbitmap == BITMAP_EMPTY)
					{
						cell_t* cell = sref;
						if (!cell->is_left_ref() && !cell->is_right_ref() && cell->left_.bitmap_ == BITMAP_FILLED && cell->right_.bitmap_ == BITMAP_FILLED)
						{
							return range;
						}
					}

					return _find_complement(range, dbitmap, sref, source, twist);
				}
				else
				{
					if ((sbitmap & ~dbitmap) != BITMAP_EMPTY)
					{
						return _find_complement(range, dbitmap, sbitmap, twist);
					}
					else 
					{
						return bin_t::NONE;
					}
				}
			}

		} 
		else if (!range.contains(destination.root_bin_))
		{
			if (is_sref) 
			{
				return _find_complement(range, BITMAP_EMPTY, sref, source, twist);
			}
			else 
			{
				return _find_complement(range, BITMAP_EMPTY, sbitmap, twist);
			}

		} 
		else 
		{ // range.contains(destination.m_root_bin)
			if (is_sref) 
			{
				SSTACK();

				SPUSH(range, sref, twist);

				do
				{
					SPOP();

					if (is_left) 
					{
						if (b.left() == destination.root_bin_) 
						{
							if (sc->is_left_ref())
							{
								const bin_t res = binmap_t::_find_complement(destination.root_bin_, destination.root_cell_, destination, sc->left_.ref_, source, twist);
								if (!res.is_none()) 
								{
									return res;
								}
							}
							else if (sc->left_.bitmap_ != BITMAP_EMPTY) 
							{
								const bin_t res = binmap_t::_find_complement(destination.root_bin_, destination.root_cell_, destination, sc->left_.bitmap_, twist);
								if (!res.is_none())
								{
									return res;
								}
							}
							continue;
						}

						if (sc->is_left_ref()) 
						{
							SPUSH(b.left(), sc->left_.ref_, twist);
							continue;

						}
						else if (sc->left_.bitmap_ != BITMAP_EMPTY) 
						{
							if (0 == (twist & (b.left().base_length() - 1) & ~(destination.root_bin_.base_length() - 1)))
							{
								const bin_t res = binmap_t::_find_complement(destination.root_bin_, destination.root_cell_, destination, sc->left_.bitmap_, twist);
								if (!res.is_none())
								{
									return res;
								}
								return binmap_t::_find_complement(destination.root_bin_.sibling(), BITMAP_EMPTY, sc->left_.bitmap_, twist);

							}
							else if (sc->left_.bitmap_ != BITMAP_FILLED)
							{
								return binmap_t::_find_complement(b.left(), BITMAP_EMPTY, sc->left_.bitmap_, twist);
							} 
							else 
							{
								bin_t::uint_t s = twist & (b.left().base_length() - 1);
								/* Sorry for the following hardcode hack: Flow the highest bit of s */
								s |= s >> 1; s |= s >> 2;
								s |= s >> 4; s |= s >> 8;
								s |= s >> 16;
								s |= (s >> 16) >> 16;   // FIXME: hide warning
								return bin_t(s + 1 + (s >> 1)); /* bin_t(s >> 1).sibling(); */
							}
						}

					} 
					else
					{
						if (sc->is_right_ref())
						{
							return binmap_t::_find_complement(b.right(), BITMAP_EMPTY, sc->right_.ref_, source, twist);
						} 
						else if (sc->right_.bitmap_ != BITMAP_EMPTY) 
						{
							return binmap_t::_find_complement(b.right(), BITMAP_EMPTY, sc->right_.bitmap_, twist);
						}
						continue;
					}
				} while (_top_ > 0);

				return bin_t::NONE;

			}
			else
			{
				if (0 == (twist & (range.base_length() - 1) & ~(destination.root_bin_.base_length() - 1)))
				{
					const bin_t res = binmap_t::_find_complement(destination.root_bin_, destination.root_cell_, destination, sbitmap, twist);
					if (!res.is_none())
					{
						return res;
					}
					return binmap_t::_find_complement(destination.root_bin_.sibling(), BITMAP_EMPTY, sbitmap, twist);
				}
				else if (sbitmap != BITMAP_FILLED) 
				{
					return binmap_t::_find_complement(range, BITMAP_EMPTY, sbitmap, twist);
				} 
				else 
				{
					bin_t::uint_t s = twist & (range.base_length() - 1);
					/* Sorry for the following hardcode hack: Flow the highest bit of s */
					s |= s >> 1; s |= s >> 2;
					s |= s >> 4; s |= s >> 8;
					s |= s >> 16;
					s |= (s >> 16) >> 16;   // FIXME: hide warning
					return bin_t(s + 1 + (s >> 1)); /* bin_t(s >> 1).sibling(); */
				}
			}
		}
	}


	bin_t binmap_t::_find_complement(const bin_t& bin, const pcell_t dref, const binmap_t& destination, const pcell_t sref, const binmap_t& source, const u64 twist)
	{
		/* Initialization */
		SDSTACK();
		SDPUSH(bin, sref, dref, twist);

		/* Main loop */
		do
		{
			SDPOP();

			if (is_left)
			{
				if (sc->is_left_ref()) 
				{
					if (dc->is_left_ref()) 
					{
						SDPUSH(b.left(), sc->left_.ref_, dc->left_.ref_, twist);
						continue;
					}
					else if (dc->left_.bitmap_ != BITMAP_FILLED)
					{
						const bin_t res = binmap_t::_find_complement(b.left(), dc->left_.bitmap_, sc->left_.ref_, source, twist);
						if (!res.is_none()) 
						{
							return res;
						}
						continue;
					}

				} 
				else if (sc->left_.bitmap_ != BITMAP_EMPTY) 
				{
					if (dc->is_left_ref())
					{
						const bin_t res = binmap_t::_find_complement(b.left(), dc->left_.ref_, destination, sc->left_.bitmap_, twist);
						if (!res.is_none())
						{
							return res;
						}
						continue;

					}
					else if ((sc->left_.bitmap_ & ~dc->left_.bitmap_) != BITMAP_EMPTY)
					{
						return binmap_t::_find_complement(b.left(), dc->left_.bitmap_, sc->left_.bitmap_, twist);
					}
				}

			} else
			{
				if (sc->is_right_ref())
				{
					if (dc->is_right_ref()) 
					{
						SDPUSH(b.right(), sc->right_.ref_, dc->right_.ref_, twist);
						continue;
					} else if (dc->right_.bitmap_ != BITMAP_FILLED)
					{
						const bin_t res = binmap_t::_find_complement(b.right(), dc->right_.bitmap_, sc->right_.ref_, source, twist);
						if (!res.is_none())
						{
							return res;
						}
						continue;
					}

				} 
				else if (sc->right_.bitmap_ != BITMAP_EMPTY) 
				{
					if (dc->is_right_ref()) 
					{
						const bin_t res = binmap_t::_find_complement(b.right(), dc->right_.ref_, destination, sc->right_.bitmap_, twist);
						if (!res.is_none())
						{
							return res;
						}
						continue;

					} 
					else if ((sc->right_.bitmap_ & ~dc->right_.bitmap_) != BITMAP_EMPTY)
					{
						return binmap_t::_find_complement(b.right(), dc->right_.bitmap_, sc->right_.bitmap_, twist);
					}
				}
			}
		} while (_top_ > 0);

		return bin_t::NONE;
	}


	bin_t binmap_t::_find_complement(const bin_t& bin, const bitmap_t dbitmap, const pcell_t sref, const binmap_t& source, const u64 twist)
	{
		ASSERT (dbitmap != BITMAP_EMPTY || sref != source.root_cell_ ||
			source.root_cell_->is_left_ref() ||
			source.root_cell_->is_right_ref() ||
			source.root_cell_->left_.bitmap_ != BITMAP_FILLED ||
			source.root_cell_->right_.bitmap_ != BITMAP_FILLED);

		/* Initialization */
		SSTACK();
		SPUSH(bin, sref, twist);

		/* Main loop */
		do
		{
			SPOP();

			if (is_left)
			{
				if (sc->is_left_ref())
				{
					SPUSH(b.left(), sc->left_.ref_, twist);
					continue;
				} 
				else if ((sc->left_.bitmap_ & ~dbitmap) != BITMAP_EMPTY)
				{
					return binmap_t::_find_complement(b.left(), dbitmap, sc->left_.bitmap_, twist);
				}

			} 
			else 
			{
				if (sc->is_right_ref())
				{
					SPUSH(b.right(), sc->right_.ref_, twist);
					continue;
				} 
				else if ((sc->right_.bitmap_ & ~dbitmap) != BITMAP_EMPTY) 
				{
					return binmap_t::_find_complement(b.right(), dbitmap, sc->right_.bitmap_, twist);
				}
			}
		} while (_top_ > 0);

		return bin_t::NONE;
	}


	bin_t binmap_t::_find_complement(const bin_t& bin, const pcell_t dref, const binmap_t& destination, const bitmap_t sbitmap, const bin_t::uint_t twist)
	{
		/* Initialization */
		DSTACK();
		DPUSH(bin, dref, twist);

		/* Main loop */
		do 
		{
			DPOP();

			if (is_left)
			{
				if (dc->is_left_ref())
				{
					DPUSH(b.left(), dc->left_.ref_, twist);
					continue;
				}
				else if ((sbitmap & ~dc->left_.bitmap_) != BITMAP_EMPTY)
				{
					return binmap_t::_find_complement(b.left(), dc->left_.bitmap_, sbitmap, twist);
				}

			} else {
				if (dc->is_right_ref())
				{
					DPUSH(b.right(), dc->right_.ref_, twist);
					continue;
				}
				else if ((sbitmap & ~dc->right_.bitmap_) != BITMAP_EMPTY)
				{
					return binmap_t::_find_complement(b.right(), dc->right_.bitmap_, sbitmap, twist);
				}
			}
		} while (_top_ > 0);

		return bin_t::NONE;
	}


	bin_t binmap_t::_find_complement(const bin_t& bin, const bitmap_t dbitmap, const bitmap_t sbitmap, bin_t::uint_t twist)
	{
		bitmap_t bitmap = sbitmap & ~dbitmap;

		ASSERT (bitmap != BITMAP_EMPTY);

		if (bitmap == BITMAP_FILLED)
		{
			return bin;
		}

		twist &= bin.base_length() - 1;

		if (sizeof(bitmap_t) == 2) 
		{
			if (twist & 1)
			{
				bitmap = ((bitmap & 0x5555) << 1)  | ((bitmap & 0xaaaa) >> 1);
			}
			if (twist & 2) 
			{
				bitmap = ((bitmap & 0x3333) << 2)  | ((bitmap & 0xcccc) >> 2);
			}
			if (twist & 4) 
			{
				bitmap = ((bitmap & 0x0f0f) << 4)  | ((bitmap & 0xf0f0) >> 4);
			}
			if (twist & 8) 
			{
				bitmap = ((bitmap & 0x00ff) << 8)  | ((bitmap & 0xff00) >> 8);
			}

			// Arno, 2012-03-21: Do workaround (see below) here as well?

			return bin_t(bin.base_left().twisted(twist & ~0x0f).value() + bitmap_to_bin(bitmap)).to_twisted(twist & 0x0f);

		} 
		else 
		{
			if (twist & 1) 
			{
				bitmap = ((bitmap & 0x55555555) << 1)  | ((bitmap & 0xaaaaaaaa) >> 1);
			}
			if (twist & 2) 
			{
				bitmap = ((bitmap & 0x33333333) << 2)  | ((bitmap & 0xcccccccc) >> 2);
			}
			if (twist & 4)
			{
				bitmap = ((bitmap & 0x0f0f0f0f) << 4)  | ((bitmap & 0xf0f0f0f0) >> 4);
			}
			if (twist & 8)
			{
				bitmap = ((bitmap & 0x00ff00ff) << 8)  | ((bitmap & 0xff00ff00) >> 8);
			}
			if (twist & 16)
			{
				bitmap = ((bitmap & 0x0000ffff) << 16)  | ((bitmap & 0xffff0000) >> 16);
			}

			bin_t diff = bin_t(bin.base_left().twisted(twist & ~0x1f).value() + bitmap_to_bin(bitmap)).to_twisted(twist & 0x1f);

			// Arno, 2012-03-21: Sanity check, if it fails, attempt workaround
			if (!bin.contains(diff))
			{
				// Bug: Proposed bin is outside of specified range. The bug appears
				// to be that the code assumes that the range parameter (called bin
				// here) is aligned on a 32-bit boundary. I.e. the width of a
				// half_t. Hence when the code does range + bitmap_to_bin(x)
				// to find the base-layer offset of the bit on which the source
				// and dest bitmaps differ, the result may be too high.
				//
				// What I do here is to round the rangestart to 32 bits, and
				// then add bitmap_to_bin(bitmap), divided by two as that function
				// returns the bit in a "bin number" format (=bit * 2).
				//
				// In other words, the "bin" parameter should tell us at what
				// base offset of the 32-bit dbitmap and sbitmap is. At the moment
				// it doesn't always, because "bin" is not rounded to 32-bit.
				//
				// see tests/binstest3.cpp

				bin_t::uint_t rangestart = bin.base_left().twisted(twist & ~0x1f).layer_offset();
				bin_t::uint_t b2b = bitmap_to_bin(bitmap);
				bin_t::uint_t absoff = ((int)(rangestart/32))*32 + b2b/2;

				diff = bin_t(0,absoff);
				diff = diff.to_twisted(twist & 0x1f);

				//char binstr[32];
				//fprintf(stderr,"__fc solution %s\n", diff.str(binstr) );
			}
			return diff;
		}
	}


	/**
	* Sets bins
	*
	* @param bin
	*             the bin
	*/
	void binmap_t::set(const bin_t& bin)
	{
		if (bin.is_none())
		{
			return;
		}

		if (bin.layer_bits() > BITMAP_LAYER_BITS)
		{
			_set__high_layer_bitmap(bin, BITMAP_FILLED);
		}
		else
		{
			_set__low_layer_bitmap(bin, BITMAP_FILLED);
		}
	}


	/**
	* Resets bins
	*
	* @param bin
	*             the bin
	*/
	void binmap_t::reset(const bin_t& bin)
	{
		if (bin.is_none()) 
		{
			return;
		}

		if (bin.layer_bits() > BITMAP_LAYER_BITS) 
		{
			_set__high_layer_bitmap(bin, BITMAP_EMPTY);
		} 
		else 
		{
			_set__low_layer_bitmap(bin, BITMAP_EMPTY);
		}
	}


	/**
	* Empty all bins
	*/
	void binmap_t::clear()
	{
		cell_t* cell = root_cell_;
		if (cell->is_left_ref()) 
		{
			free_cell(cell->left_.ref_);
		}
		if (cell->is_right_ref())
		{
			free_cell(cell->right_.ref_);
		}

		cell->set_is_left_ref(false);
		cell->set_is_right_ref(false);
		cell->left_.bitmap_ = BITMAP_EMPTY;
		cell->right_.bitmap_ = BITMAP_EMPTY;
	}


	/**
	* Fill the binmap. Creates a new filled binmap. Size is given by the source root
	*/
	void binmap_t::fill(const binmap_t& source)
	{
		root_bin_ = source.root_bin_;

		/* Extends root if needed */
		while (!root_bin_.contains(source.root_bin_)) 
		{
			if (!extend_root()) 
			{
				return /* ALLOC ERROR */;
			}
		}
		set(source.root_bin_);

		cell_t* cell = root_cell_;

		cell->set_is_left_ref(false);
		cell->set_is_right_ref(false);
		cell->left_.bitmap_ = BITMAP_FILLED;
		cell->right_.bitmap_ = BITMAP_FILLED;
	}



	/**
	* Get number of allocated cells
	*/
	size_t binmap_t::cells_number() const
	{
		return allocated_cells_number_;
	}


	/**
	* Get total size of the binmap
	*/
	size_t binmap_t::total_size() const
	{
		return sizeof(*this) + sizeof(cell_t) * allocated_cells_number_;
	}



	/**
	* Echo the binmap status to stdout
	*/
	void binmap_t::status() const
	{
		x_printf("bitmap:\n");
		for (int i = 0; i < 16; ++i) 
		{
			for (int j = 0; j < 64; ++j) 
			{
				x_printf("%d", is_filled(bin_t(i * 64 + j)));
			}
			x_printf("\n");
		}

		x_printf("size: %u bytes\n", static_cast<unsigned int>(total_size()));
		x_printf("cells number: %u (of %u)\n", static_cast<unsigned int>(allocated_cells_number_), static_cast<unsigned int>(allocated_cells_number_));
		x_printf("root bin: %llu\n", static_cast<unsigned long long>(root_bin_.value()));
	}


	/** Trace the bin */
	inline void binmap_t::trace(pcell_t* ref, bin_t* bin, const bin_t& target) const
	{
		ASSERT (root_bin_.contains(target));

		pcell_t cur_ref = root_cell_;
		bin_t cur_bin = root_bin_;

		while (target != cur_bin) 
		{
			if (target < cur_bin)
			{
				if (cur_ref->is_left_ref())
				{
					cur_ref = cur_ref->left_.ref_;
					cur_bin.to_left();
				} 
				else
				{
					break;
				}
			}
			else
			{
				if (cur_ref->is_right_ref()) 
				{
					cur_ref = cur_ref->right_.ref_;
					cur_bin.to_right();
				} 
				else
				{
					break;
				}
			}
		}

		ASSERT (cur_bin.layer_bits() > BITMAP_LAYER_BITS);

		if (ref)
		{
			*ref = cur_ref;
		}
		if (bin)
		{
			*bin = cur_bin;
		}
	}


	/** Trace the bin */
	inline void binmap_t::trace(pcell_t* ref, bin_t* bin, pcell_t** history, const bin_t& target) const
	{
		ASSERT (history);
		ASSERT (root_bin_.contains(target));

		pcell_t* href = *history;
		pcell_t cur_ref = root_cell_;
		bin_t cur_bin = root_bin_;

		*href++ = root_cell_;
		while (target != cur_bin) 
		{
			if (target < cur_bin) 
			{
				if (cur_ref->is_left_ref()) 
				{
					cur_ref = cur_ref->left_.ref_;
					cur_bin.to_left();
				} 
				else
				{
					break;
				}
			}
			else
			{
				if (cur_ref->is_right_ref())
				{
					cur_ref = cur_ref->right_.ref_;
					cur_bin.to_right();
				}
				else
				{
					break;
				}
			}

			*href++ = cur_ref;
		}

		ASSERT (cur_bin.layer_bits() > BITMAP_LAYER_BITS);

		if (ref) 
		{
			*ref = cur_ref;
		}
		if (bin)
		{
			*bin = cur_bin;
		}

		*history = href;
	}


	/**
	* Copy a binmap to another
	*/
	void binmap_t::copy(binmap_t& destination, const binmap_t& source)
	{
		destination.root_bin_ = source.root_bin_;
		binmap_t::copy(destination, destination.root_cell_, source, source.root_cell_);
	}


	/**
	* Copy a range from one binmap to another binmap
	*/
	void binmap_t::copy(binmap_t& destination, const binmap_t& source, const bin_t& range)
	{
		pcell_t int_ref;
		bin_t int_bin;

		if (range.contains(destination.root_bin_))
		{
			if (source.root_bin_.contains(range))
			{
				source.trace(&int_ref, &int_bin, range);
				destination.root_bin_ = range;
				binmap_t::copy(destination, destination.root_cell_, source, int_ref);
			} 
			else if (range.contains(source.root_bin_))
			{
				destination.root_bin_ = source.root_bin_;
				binmap_t::copy(destination, destination.root_cell_, source, source.root_cell_);
			} 
			else 
			{
				destination.reset(range);
			}

		} 
		else 
		{
			if (source.root_bin_.contains(range))
			{
				source.trace(&int_ref, &int_bin, range);

				cell_t* cell = int_ref;

				if (range.layer_bits() <= BITMAP_LAYER_BITS)
				{
					if (range < int_bin)
					{
						destination._set__low_layer_bitmap(range, cell->left_.bitmap_);
					}
					else 
					{
						destination._set__low_layer_bitmap(range, cell->right_.bitmap_);
					}
				}
				else 
				{
					if (range == int_bin) 
					{
						if (cell->is_left_ref() || cell->is_right_ref() || cell->left_.bitmap_ != cell->right_.bitmap_)
						{
							binmap_t::_copy__range(destination, source, int_ref, range);
						}
						else 
						{
							destination._set__high_layer_bitmap(range, cell->left_.bitmap_);
						}
					}
					else if (range < int_bin)
					{
						destination._set__high_layer_bitmap(range, cell->left_.bitmap_);
					}
					else 
					{
						destination._set__high_layer_bitmap(range, cell->right_.bitmap_);
					}
				}

			}
			else if (range.contains(source.root_bin_)) 
			{
				destination.reset(range);   // Probably it could be optimized

				cell_t* cell = source. root_cell_;

				if (cell->is_left_ref() || cell->is_right_ref() || cell->left_.bitmap_ != cell->right_.bitmap_) 
				{
					binmap_t::_copy__range(destination, source, source.root_cell_, source.root_bin_);
				}
				else
				{
					destination._set__high_layer_bitmap(source.root_bin_, cell->left_.bitmap_);
				}

			}
			else
			{
				destination.reset(range);
			}
		}
	}


	inline void binmap_t::_set__low_layer_bitmap(const bin_t& bin, const bitmap_t _bitmap)
	{
		ASSERT (bin.layer_bits() <= BITMAP_LAYER_BITS);

		const bitmap_t bin_bitmap = BITMAP[ bin.value() & BITMAP_LAYER_BITS ];
		const bitmap_t bitmap = _bitmap & bin_bitmap;

		/* Extends root if needed */
		if (!root_bin_.contains(bin))
		{
			/* Trivial case */
			if (bitmap == BITMAP_EMPTY) 
				return;

			do
			{
				if (!extend_root()) 
				{
					return /* ALLOC ERROR */;
				}
			} while (!root_bin_.contains(bin));
		}

		/* Get the pre-range */
		const bin_t pre_bin( (bin.value() & (~(BITMAP_LAYER_BITS + 1))) | BITMAP_LAYER_BITS );

		/* Trace the bin with history */
		pcell_t _href[64];
		pcell_t* href = _href;
		pcell_t cur_ref = 0;
		bin_t cur_bin;

		/* Process first stage -- do not touch existing tree */
		trace(&cur_ref, &cur_bin, &href, pre_bin);

		ASSERT (cur_bin.layer_bits() > BITMAP_LAYER_BITS);

		/* Checking that we need to do anything */
		bitmap_t bm = BITMAP_EMPTY;
		{
			cell_t* cell = cur_ref;

			if (bin < cur_bin)
			{
				ASSERT (!cell->is_left_ref());
				bm = cell->left_.bitmap_;
				if ((bm & bin_bitmap) == bitmap)
				{
					return;
				}
				if (cur_bin == pre_bin)
				{
					cell->left_.bitmap_ = (cell->left_.bitmap_ & ~bin_bitmap) | bitmap;
					pack_cells(href - 1);
					return;
				}
			}
			else 
			{
				ASSERT (!cell->is_right_ref());
				bm = cell->right_.bitmap_;
				if ((bm & bin_bitmap) == bitmap)
				{
					return;
				}
				if (cur_bin == pre_bin)
				{
					cell->right_.bitmap_ = (cell->right_.bitmap_ & ~bin_bitmap) | bitmap;
					pack_cells(href - 1);
					return;
				}
			}
		}

		/* Continue to trace */
		do 
		{
			const pcell_t ref = alloc_cell();
			cell_t* ref_cell = ref;
			ref_cell->set_is_left_ref(false);
			ref_cell->set_is_right_ref(false);
			ref_cell->left_.bitmap_ = bm;
			ref_cell->right_.bitmap_ = bm;

			cell_t* cur_cell = cur_ref;
			if (pre_bin < cur_bin)
			{
				cur_cell->set_is_left_ref(true);
				cur_cell->left_.ref_ = ref;
				cur_bin.to_left();
			}
			else 
			{
				cur_cell->set_is_right_ref(true);
				cur_cell->right_.ref_ = ref;
				cur_bin.to_right();
			}

			cur_ref = ref;
		} while (cur_bin != pre_bin);

		ASSERT (cur_bin == pre_bin);
		ASSERT (cur_bin.layer_bits() > BITMAP_LAYER_BITS);

		/* Complete setting */
		cell_t* cur_cell = cur_ref;
		if (bin < cur_bin) 
		{
			cur_cell->left_.bitmap_ = (cur_cell->left_.bitmap_ & ~bin_bitmap) | bitmap;
		}
		else 
		{
			cur_cell->right_.bitmap_ = (cur_cell->right_.bitmap_ & ~bin_bitmap) | bitmap;
		}
	}


	inline void binmap_t::_set__high_layer_bitmap(const bin_t& bin, const bitmap_t bitmap)
	{
		ASSERT (bin.layer_bits() > BITMAP_LAYER_BITS);

		/* First trivial case */
		if (bin.contains(root_bin_)) 
		{
			cell_t* cell = root_cell_;
			if (cell->is_left_ref()) 
			{
				free_cell(cell->left_.ref_);
			}
			if (cell->is_right_ref()) 
			{
				free_cell(cell->right_.ref_);
			}

			root_bin_ = bin;
			cell->set_is_left_ref(false);
			cell->set_is_right_ref(false);
			cell->left_.bitmap_ = bitmap;
			cell->right_.bitmap_ = bitmap;

			return;
		}

		/* Get the pre-range */
		bin_t pre_bin = bin.parent();

		/* Extends root if needed */
		if (!root_bin_.contains(pre_bin))
		{
			/* Second trivial case */
			if (bitmap == BITMAP_EMPTY) 
			{
				return;
			}

			do
			{
				if (!extend_root())
				{
					return /* ALLOC ERROR */;
				}
			} while (!root_bin_.contains(pre_bin));
		}

		/* The trace the bin with history */
		pcell_t _href[64];
		pcell_t* href = _href;
		pcell_t cur_ref;
		bin_t cur_bin;

		/* Process first stage -- do not touch existed tree */
		trace(&cur_ref, &cur_bin, &href, pre_bin);

		/* Checking that we need to do anything */
		bitmap_t bm = BITMAP_EMPTY;
		{
			cell_t* cell = cur_ref;
			if (bin < cur_bin)
			{
				if (cell->is_left_ref()) 
				{
					/* ASSERT (cur_bin == pre_bin); */
					cell->set_is_left_ref(false);
					free_cell(cell->left_.ref_);
				}
				else
				{
					bm = cell->left_.bitmap_;
					if (bm == bitmap) 
					{
						return;
					}
				}
				if (cur_bin == pre_bin)
				{
					cell->left_.bitmap_ = bitmap;
					pack_cells(href - 1);
					return;
				}
			}
			else 
			{
				if (cell->is_right_ref())
				{
					/* ASSERT (cur_bin == pre_bin); */
					cell->set_is_right_ref(false);
					free_cell(cell->right_.ref_);
				}
				else
				{
					bm = cell->right_.bitmap_;
					if (bm == bitmap) 
					{
						return;
					}
				}
				if (cur_bin == pre_bin) 
				{
					cell->right_.bitmap_ = bitmap;
					pack_cells(href - 1);
					return;
				}
			}
		}

		/* Continue to trace */
		do
		{
			const pcell_t ref = alloc_cell();
			cell_t* ref_cell = ref;
			ref_cell->set_is_left_ref(false);
			ref_cell->set_is_right_ref(false);
			ref_cell->left_.bitmap_ = bm;
			ref_cell->right_.bitmap_ = bm;

			cell_t* cur_cell = cur_ref;
			if (pre_bin < cur_bin)
			{
				cur_cell->set_is_left_ref(true);
				cur_cell->left_.ref_ = ref;
				cur_bin.to_left();
			}
			else
			{
				cur_cell->set_is_right_ref(true);
				cur_cell->right_.ref_ = ref;
				cur_bin.to_right();
			}

			cur_ref = ref;
		} while (cur_bin != pre_bin);

		ASSERT (cur_bin == pre_bin);
		ASSERT (cur_bin.layer_bits() > BITMAP_LAYER_BITS);

		/* Complete setting */
		if (bin < cur_bin)
		{
			cur_ref->left_.bitmap_ = bitmap;
		} 
		else
		{
			cur_ref->right_.bitmap_ = bitmap;
		}
	}


	void binmap_t::_copy__range(binmap_t& destination, const binmap_t& source, const pcell_t sref, const bin_t sbin)
	{
		ASSERT (sbin.layer_bits() > BITMAP_LAYER_BITS);

		ASSERT (sref == source.root_cell_ ||
			sref->is_left_ref() || sref->is_right_ref() ||
			sref->left_.bitmap_ != sref->right_.bitmap_
			);

		/* Extends root if needed */
		while (!destination.root_bin_.contains(sbin)) 
		{
			if (!destination.extend_root())
			{
				return /* ALLOC ERROR */;
			}
		}

		/* The trace the bin */
		pcell_t cur_ref;
		bin_t cur_bin;

		/* Process first stage -- do not touch existed tree */
		destination.trace(&cur_ref, &cur_bin, sbin);

		/* Continue unpacking if needed */
		if (cur_bin != sbin)
		{
			bitmap_t bm = BITMAP_EMPTY;

			if (sbin < cur_bin) {
				bm = cur_ref->left_.bitmap_;
			} else {
				bm = cur_ref->right_.bitmap_;
			}

			/* Continue to trace */
			do
			{
				const pcell_t ref = destination.alloc_cell();
				cell_t* ref_cell = cur_ref;
				ref_cell->set_is_left_ref(false);
				ref_cell->set_is_right_ref(false);
				ref_cell->left_.bitmap_ = bm;
				ref_cell->right_.bitmap_ = bm;

				cell_t* cur_cell = cur_ref;
				if (sbin < cur_bin)
				{
					cur_cell->set_is_left_ref(true);
					cur_cell->left_.ref_ = ref;
					cur_bin.to_left();
				}
				else 
				{
					cur_cell->set_is_right_ref(true);
					cur_cell->right_.ref_ = ref;
					cur_bin.to_right();
				}

				cur_ref = ref;
			} while (cur_bin != sbin);
		}

		/* Make copying */
		copy(destination, cur_ref, source, sref);
	}


	/**
	* Clone binmap cells to another binmap
	*/
	void binmap_t::copy(binmap_t& destination, const pcell_t dref, const binmap_t& source, const pcell_t sref)
	{
		ASSERT (dref == destination.root_cell_ || sref->is_left_ref() || sref->is_right_ref() || sref->left_.bitmap_ != sref->right_.bitmap_);

		size_t sref_size = 0;
		size_t dref_size = 0;

		pcell_t sstack[128];
		pcell_t dstack[128];
		size_t top = 0;

		/* Get size of the source subtree */
		sstack[top++] = sref;
		do 
		{
			ASSERT (top < sizeof(sstack) / sizeof(sstack[0]));

			++sref_size;

			cell_t* scell = sstack[--top];
			if (scell->is_left_ref()) 
			{
				sstack[top++] = scell->left_.ref_;
			}
			if (scell->is_right_ref())
			{
				sstack[top++] = scell->right_.ref_;
			}

		} while (top > 0);

		/* Get size of the destination subtree */
		dstack[top++] = dref;
		do 
		{
			ASSERT (top < sizeof(dstack) / sizeof(dstack[0]));

			++dref_size;

			cell_t* dcell = dstack[--top];
			if (dcell->is_left_ref()) 
			{
				dstack[top++] = dcell->left_.ref_;
			}
			if (dcell->is_right_ref()) 
			{
				dstack[top++] = dcell->right_.ref_;
			}

		} while (top > 0);

		/* Reserving proper number of cells */
		if (dref_size < sref_size) 
		{

		}

		/* Release the destination subtree */
		cell_t* dref_cell = dref;
		if (dref_cell->is_left_ref())
		{
			destination.free_cell(dref_cell->left_.ref_);
		}
		if (dref_cell->is_right_ref()) 
		{
			destination.free_cell(dref_cell->right_.ref_);
		}

		/* Make cloning */
		sstack[top] = sref;
		dstack[top] = dref;
		++top;

		do
		{
			--top;
			cell_t* scell = sstack[top];
			cell_t* dcell = dstack[top];

			/* Processing left ref */
			if (scell->is_left_ref()) 
			{
				dcell->set_is_left_ref(true);
				dcell->left_.ref_ = destination.alloc_cell();

				sstack[top] = scell->left_.ref_;
				dstack[top] = dcell->left_.ref_;
				++top;
			}
			else 
			{
				dcell->set_is_left_ref(false);
				dcell->left_.bitmap_ = scell->left_.bitmap_;
			}

			/* Processing right ref */
			if (scell->is_right_ref()) 
			{
				dcell->set_is_right_ref(true);
				dcell->right_.ref_ = destination.alloc_cell();

				sstack[top] = scell->right_.ref_;
				dstack[top] = dcell->right_.ref_;
				++top;
			} 
			else 
			{
				dcell->set_is_right_ref(false);
				dcell->right_.bitmap_ = scell->right_.bitmap_;
			}
		} while (top > 0);
	}

}