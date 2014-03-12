#include "xbase/x_debug.h"
#include "xbase/x_memory_std.h"
#include "xbase/x_string_std.h"

#include "xbinmaps/binmap.h"

namespace xcore
{

	/**
	* Constructor
	*/
	binmap_t::binmap_t()
		: allocator_(0)
		, binroot_(0)
		, binmap1_(0)
		, binmap0_(0)
	{

	}


	/**
	* Destructor
	*/
	binmap_t::~binmap_t()
	{
		exit();
	}

	/**
	* Initialization of a binmap
	*/
	void binmap_t::init(const bin_t& _bin, allocator* _allocator)
	{
		ASSERT(allocator_ == NULL);
		ASSERT(binmap1_ == NULL);
		ASSERT(binmap0_ == NULL);

		bin_t root = _bin;
		while (root.layer_offset() > 0)
			root.to_parent();
		ASSERT(root.base_length() >= 2);

		u32 const binmap_size = (u32)(((root.base_length() * 2) + 7) / 8);

		allocator_ = _allocator;

		binroot_  = (bin_t*)_allocator->allocate(binmap_size * 2 + sizeof(bin_t), 8);
		*binroot_ = root;

		binmap1_ = (xbyte*)binroot_ + sizeof(bin_t);
		binmap0_ = binmap1_ + binmap_size;

		clear();
	}

	/**
	* Uninitialize a binmap
	*/
	void binmap_t::exit()
	{
		if (allocator_ != NULL)
		{
			// binmap0_ and binmap1_ are part of binroot_, so we
			// do not need to deallocate them.
			if (binroot_ != NULL)
				allocator_->deallocate(binroot_);
		}
		allocator_ = NULL;

		binroot_ = NULL;
		binmap1_ = NULL;
		binmap0_ = NULL;
	}

	/**
	* Whether binmap is empty
	*/
	bool binmap_t::is_empty() const
	{
		bool const r = read_value0_at(*binroot_);
		return !r;
	}


	/**
	* Whether binmap is filled
	*/
	bool binmap_t::is_filled() const
	{
		return is_filled(*binroot_);
	}


	/**
	* Whether range/bin is empty
	*/
	bool binmap_t::is_empty(const bin_t& bin) const
	{
		bool r = false;
		if (bin == bin_t::ALL)
		{
			r = read_value0_at(*binroot_);
		}
		else
		{
			ASSERT(binroot_->contains(bin));
			r = read_value0_at(bin);
		}
		return !r;
	}


	/**
	* Whether range/bin is filled
	*/
	bool binmap_t::is_filled(const bin_t& bin) const
	{
		bool v = false;
		if (bin == bin_t::ALL)
		{
			v = read_value1_at(*binroot_);
		}
		else
		{
			ASSERT(binroot_->contains(bin));
			v = read_value1_at(bin);
		}
		return v;
	}


	/**
	* Return the topmost solid bin which covers the specified bin
	*/
	bin_t binmap_t::cover(const bin_t& bin) const
	{
		bin_t i(bin);
		bool v = read_value1_at(i);
		if (!v)
			return i;

		while (i != *binroot_)
		{
			bin_t p = i.parent();
			v = read_value1_at(p);
			if (!v)
				break;
			i = p;
		}
		return i;
	}


	/**
	* Find first empty bin
	*/
	bin_t binmap_t::find_empty() const
	{
		return find_empty(*binroot_);
	}


	/**
	* Find first filled bin
	*/
	bin_t binmap_t::find_filled() const
	{
		// Can we can find a filled bin in this sub-tree?
		if (read_value0_at(*binroot_)==false)
			return bin_t::NONE;

		bin_t i(*binroot_);
		s32 l = i.layer();
		while (l >= 0)
		{
			bin_t c = i.left();
			if (read_value0_at(c) == false)
			{
				c = i.right();
				ASSERT(read_value0_at(c) == true);
			}
			i = c;
			--l;
		}
		return i;
	}


	/**
	* Find first empty bin right of start (start inclusive)
	*/
	bin_t binmap_t::find_empty(bin_t start) const
	{
		ASSERT(start != bin_t::ALL);

		// does start fall within this binmap?
		if (!binroot_->contains(start))
		{
			return bin_t::NONE;
		}

		if (read_value1_at(*binroot_))
		{	// full, impossible to find an empty bin
			return bin_t::NONE;
		}

		bin_t i(start);
		u32 layer = i.layer();
		// traverse and keep trying to go up until a !filled(bin) is encountered.
		// when going up check if base_left() is still right or equal to the start bin
		do
		{
			if (read_value0_at(i) == false)
				return i;

			// Does this sub-tree has a possible empty bin ?
			if (read_value1_at(i) == false)
				break;

			// traverse horizontally
			i = bin_t(layer, i.layer_offset() + 1);
			if (!binroot_->contains(i))
				return bin_t::NONE;

			bin_t const parent = i.parent();
			if (parent.value() >= start.value())
			{
				i = parent;
				++layer;
			}

		} while (i != *binroot_);

		// We can find an empty bin in this sub-tree
		while (layer >= 0)
		{
			// we can early out when the OR binmap is indicating that
			// the bin (and sub-tree) is empty.
			if (read_value0_at(i) == false)
				return i;

			bin_t c = i.left();
			if (read_value1_at(c) == true)
			{
				c = i.right();
				ASSERT(read_value1_at(c) == false);
			}
			i = c;
			--layer;
		}

		return bin_t::NONE;
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
		return find_complement(destination, source, *source.binroot_, twist);
	}

	bin_t binmap_t::find_complement(const binmap_t& destination, const binmap_t& source, bin_t range, const bin_t::uint_t twist)
	{
		ASSERT(source.binroot_->contains(range));
		ASSERT(destination.binroot_->contains(range));
		ASSERT(*source.binroot_ == *destination.binroot_);

		// Brute Force
		s32 const lbo = (s32)(range.base_left().layer_offset());
		s32 const n = (s32)(range.base_length());

		for (s32 i=0; i<n; ++i)
		{
			bin_t b(0, lbo + i);
			if (source.read_value1_at(b)==true && destination.read_value1_at(b)==false)
			{
				// up
				while (b != *source.binroot_)
				{
					bin_t p = b.parent();
					if (source.read_value1_at(p)==true && destination.read_value0_at(p)==false)
					{
						b = p;
					}
					else
					{
						break;
					}
				}
				return b;
			}
		}

		return bin_t::NONE;
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
			return;

		if (bin == *binroot_ || bin == bin_t::ALL)
		{
			fill();
		}
		else if (binroot_->contains(bin))
		{
			const u32 root_layer = binroot_->layer();
			const u32 bin_layer  = (root_layer>0) ? bin.layer() : 0;

			// check if this action is changing the value to begin with
			// if not we can do an early-out
			
			// binmap0_
			{		
				if (update_value0_at(bin, true) == false)
				{
					bin_t ib = bin;
					u32 ib_layer = bin_layer;
					do
					{
						ib.to_parent();
						++ib_layer;

						bool const oiv = update_value0_at(ib, true);
						if (true == oiv)
							break;

					} while (ib_layer < root_layer);
				}

				if (bin_layer > 0)
				{
					// fill the range
					bin_t il = bin.base_left();
					bin_t ir = bin.base_right();

					// determine start, end and length
					u32 const lo = (u32)(il.value() >> 3);
					u32 const ro = (u32)(ir.value() >> 3);

					xbyte const lm = 0xFF >> (il.value() & 0x07);
					xbyte const rm = (xbyte)(0xFF80 >> (ir.value() & 0x07));

					{
						xbyte* lb = binmap0_ + lo;
						xbyte* rb = binmap0_ + ro;

						s32 const d = ro - lo;
						if (d == 0)
						{
							*lb = *lb | (lm & rm);
						}
						else if (d == 1)
						{
							*lb = *lb | lm;
							*rb = *rb | rm;
						}
						else
						{
							*lb = *lb | lm;
							++lb;
							while (lb < rb)
								*lb++ = 0xff;
							*rb = *rb | rm;
						}
					}
				}
			}

			// binmap1_
			{				

				if (!update_value1_at(bin, true))
				{
					u32 ib_layer = bin_layer;
					bin_t ib = bin;
					bool iv = true;
					while (ib_layer < root_layer)
					{
						bool const sv = read_value1_at(ib.sibling());
						iv = (iv && sv);
						ib.to_parent();
						++ib_layer;

						bool const oiv = update_value1_at(ib, iv);
						if (oiv == iv)
							break;
					};
				}

				if (bin_layer > 0)
				{
					// fill the range
					bin_t il = bin.base_left();
					bin_t ir = bin.base_right();

					// determine start, end and length
					u32 const lo = (u32)(il.value() >> 3);
					u32 const ro = (u32)(ir.value() >> 3);

					xbyte const lm = 0xFF >> (il.value() & 0x07);
					xbyte const rm = (xbyte)(0xFF80 >> (ir.value() & 0x07));

					{
						xbyte* lb = binmap1_ + lo;
						xbyte* rb = binmap1_ + ro;
						s32 const d = ro - lo;
						if (d == 0)
						{
							*lb = *lb | (lm & rm);
						}
						else if (d == 1)
						{
							*lb = *lb | lm;
							*rb = *rb | rm;
						}
						else
						{
							*lb = *lb | lm;
							++lb;
							while (lb < rb)
								*lb++ = 0xff;
							*rb = *rb | rm;
						}
					}
				}
			}
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
			return;

		if (bin == *binroot_ || bin == bin_t::ALL)
		{
			clear();
		}
		else if (binroot_->contains(bin))
		{
			const u32 root_layer = binroot_->layer();
			const u32 bin_layer  = (root_layer>0) ? bin.layer() : 0;

			// check if this action is changing the value to begin with
			// if not we can do an early-out

			// binmap0_
			{	
				if (update_value0_at(bin, false))
				{
					bin_t ib = bin;
					u32 ib_layer = bin_layer;
					while (ib_layer < root_layer)
					{
						bool sv = read_value0_at(ib.sibling());
						if (sv)
							break;

						ib.to_parent();
						++ib_layer;
						bool const oiv = update_value0_at(ib, false);
						if (oiv == false)
							break;
					};
				}

				if (bin_layer > 0)
				{
					// clear the range
					bin_t il = bin.base_left();
					bin_t ir = bin.base_right();

					// determine start, end and length
					u32 const llo = (u32)(il.value() >> 3);
					u32 const lro = (u32)(ir.value() >> 3);

					xbyte const lm = (xbyte)(0xFF00 >> (il.value() & 0x07));
					xbyte const rm = 0x7F >> (ir.value() & 0x07);

					{
						xbyte* lb = binmap0_ + llo;
						xbyte* rb = binmap0_ + lro;

						s32 const d = (s32)(rb - lb);
						if (d == 0)
						{
							*lb = *lb & (lm | rm);
						}
						else if (d == 1)
						{
							*lb = *lb & lm;
							*rb = *rb & rm;
						}
						else
						{
							*lb = *lb & lm;
							++lb;
							while (lb < rb)
								*lb++ = 0;
							*rb = *rb & rm;
						}
					}
				}
			}

			// check if this action is changing the value to begin with
			// if not we can do an early-out
			// binmap1_
			{				
				if (update_value1_at(bin, false))
				{
					bin_t ib = bin;
					u32 ib_layer = bin_layer;
					while (ib_layer < root_layer)
					{
						ib.to_parent();
						++ib_layer;
						bool const oiv = update_value1_at(ib, false);
						if (oiv == false)
							break;
					};
				}

				if (bin_layer > 0)
				{
					// clear the range
					bin_t il = bin.base_left();
					bin_t ir = bin.base_right();

					// determine start, end and length
					u32 const llo = (u32)(il.value() >> 3);
					u32 const lro = (u32)(ir.value() >> 3);

					xbyte const lm = (xbyte)(0xFF00 >> (il.value() & 0x07));
					xbyte const rm = 0x7F >> (ir.value() & 0x07);

					{
						xbyte* lb = binmap1_ + llo;
						xbyte* rb = binmap1_ + lro;

						s32 const d = (s32)(rb - lb);
						if (d == 0)
						{
							*lb = *lb & (lm | rm);
						}
						else if (d == 1)
						{
							*lb = *lb & lm;
							*rb = *rb & rm;
						}
						else
						{
							*lb = *lb & lm;
							++lb;
							while (lb < rb)
								*lb++ = 0;
							*rb = *rb & rm;
						}
					}
				}
			}
		}
	}


	/**
	* Empty all bins
	*/
	void binmap_t::clear()
	{
		u32 const binmap_size = (u32)(((binroot_->base_length() * 2) + 7) / 8);
		x_memzero(binmap1_, binmap_size);
		x_memzero(binmap0_, binmap_size);
	}


	/**
	* Fill the binmap. Creates a new filled binmap. Size is given by the source root
	*/
	void binmap_t::fill()
	{
		u32 const binmap_size = (u32)(((binroot_->base_length() * 2) + 7) / 8);
		x_memset(binmap1_, 0xffffffff, binmap_size);
		x_memset(binmap0_, 0xffffffff, binmap_size);
	}

	/**
	* Get total size of the binmap
	*/
	size_t binmap_t::total_size() const
	{
		u32 const binmap_size = (u32)(((binroot_->base_length() * 2) + 7) / 8);
		return sizeof(binmap_t) + binmap_size + binmap_size;
	}


	/**
	* Echo the binmap status to stdout
	*/
	void binmap_t::status() const
	{
		x_printf("bitmap:\n");
		s32 l = (s32)binroot_->layer();
		s32 f = (s32)binroot_->base_length();
		bin_t li = *binroot_;
		bin_t ri = *binroot_;
		for (int i=l; i>=0; --i) 
		{
			s32 w = (s32)(ri.layer_offset() - li.layer_offset());
			for (int k=0; k<(f+1-w); k+=2)
			{
				x_printf("  ");
			}

			for (int j=0; j<=w; ++j)
			{
				x_printf("%d ", is_filled(bin_t(i,j)));
			}
			li.to_left();
			ri.to_right();
			x_printf("\n");
		}

		x_printf("size: %u bytes\n", static_cast<unsigned int>(total_size()));
		x_printf("root bin: %llu\n", static_cast<unsigned long long>(binroot_->value()));
	}

	/**
	* Copy a binmap to another
	*/
	void binmap_t::copy(binmap_t& destination, const binmap_t& source)
	{
		ASSERT(*source.binroot_ != bin_t::NONE);
		ASSERT(*source.binroot_ == *destination.binroot_);
		u32 const binmap_size = (u32)(((source.binroot_->base_length() * 2) + 7) / 8);
		x_memcpy(destination.binmap1_, source.binmap1_, binmap_size);
		x_memcpy(destination.binmap0_, source.binmap0_, binmap_size);
	}


	/**
	* Copy a range from one binmap to another binmap
	*/
	void binmap_t::copy(binmap_t& destination, const binmap_t& source, const bin_t& range)
	{
		if (!source.read_value0_at(range))
		{
			destination.reset(range);
		}
		else if (source.read_value1_at(range))
		{
			destination.set(range);
		}
		else
		{
			s32 const e = (s32)(range.base_offset() + range.base_length());
			for (s32 i=(s32)range.base_offset(); i<e; ++i)
			{
				bin_t const b(0, i);
				bool const v = source.read_value1_at(b);
				if (v) destination.set(b);
				else destination.reset(b);
			}
		}
	}





}