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
		, binroot_(63)
		, maxlayer_(0)
		, binmap1_size(0)
		, binmap1_(0)
		, binmap0_size(0)
		, binmap0_(0)
	{

	}


	/**
	* Destructor
	*/
	binmap_t::~binmap_t()
	{
		if (binmap1_ != NULL)
			allocator_->deallocate(binmap1_);
		if (binmap0_ != NULL)
			allocator_->deallocate(binmap0_);
	}

	/**
	* Allocates one cell (dirty allocation)
	*/
	void binmap_t::init(const bin_t& _bin, allocator* _allocator)
	{
		if (allocator_ != NULL)
		{
			if (binmap1_ != NULL)
				allocator_->deallocate(binmap1_);
			if (binmap0_ != NULL)
				allocator_->deallocate(binmap0_);

			binmap1_ = NULL;
			binmap0_ = NULL;
		}

		allocator_ = _allocator;

		binroot_ = _bin;
		while (binroot_.layer_offset() > 0)
			binroot_.to_parent();
		ASSERT(binroot_.base_length() >= 2);
		maxlayer_ = binroot_.layer();

		binmap1_size = ((binroot_.base_length() * 2) + 7) / 8;
		binmap1_ = (xbyte*)allocator_->allocate(binmap1_size, 8);
		binmap0_size = binmap1_size;
		binmap0_ = (xbyte*)allocator_->allocate(binmap0_size, 8);
		clear();
	}

	/**
	* Whether binmap is empty
	*/
	bool binmap_t::is_empty() const
	{
		bool const r = read_value0_at(binroot_);
		return !r;
	}


	/**
	* Whether binmap is filled
	*/
	bool binmap_t::is_filled() const
	{
		return is_filled(binroot_);
	}


	/**
	* Whether range/bin is empty
	*/
	bool binmap_t::is_empty(const bin_t& bin) const
	{
		if (bin == bin_t::ALL)
		{
			bool const r = read_value0_at(binroot_);
			return !r;
		}
		else
		{
			if (!binroot_.contains(bin))
				return false;
			
			if (bin.is_base())
			{
				bool const r = read_value1_at(bin);
				return !r;
			}
			else
			{
				bool const r = read_value0_at(bin);
				return !r;
			}
		}
	}


	/**
	* Whether range/bin is filled
	*/
	bool binmap_t::is_filled(const bin_t& bin) const
	{
		bool v = false;
		if (bin == bin_t::ALL)
		{
			v = read_value1_at(binroot_);
		}
		else
		{
			if (binroot_.contains(bin))
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

		while (i != binroot_)
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
		return find_empty(binroot_);
	}


	/**
	* Find first filled bin
	*/
	bin_t binmap_t::find_filled() const
	{
		// Can we can find a filled bin in this sub-tree?
		if (read_value0_at(binroot_)==false)
			return bin_t::NONE;

		bin_t i(binroot_);
		s32 l = i.layer();
		while (l > 0)
		{
			bin_t c = i.left();
			if (read_value0_at(c)==false)
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
		// does start fall within this binmap?
		if (!binroot_.contains(start))
			return bin_t::NONE;

		if (is_filled(start))
		{	// full, impossible to find an empty bin
			return bin_t::NONE;
		}

		const bin_t::uint_t bl = start.value();

		bin_t i(start);
		u32 layer = i.layer();
		if (i.is_base())
		{
			i = bin_t(layer, i.layer_offset() + 1);			
			if (!binroot_.contains(i))
				return bin_t::NONE;
		}

		// traverse and keep trying to go up until a !filled(bin) is encountered.
		// when going up check if base_left() is still right or equal to the start bin
		if (layer > 0)
		{
			do
			{
				if (read_value0_at(i) == false)
					return i;

				// Does this sub-tree has a possible empty bin ?
				if (read_value1_at(i) == false)
					break;

				// traverse horizontally
				i = bin_t(layer, i.layer_offset() + 1);
				if (!binroot_.contains(i))
					return bin_t::NONE;

				i.to_parent();
				++layer;
			} while (i != binroot_);

			// We can find an empty bin in this sub-tree
			while (layer > 0)
			{
				// we can early out when the OR binmap is indicating that
				// the bin (and sub-tree) is empty.
				if (read_value0_at(i)==false)
					break;

				bin_t c = i.left();
				if (read_value1_at(c))
				{
					c = i.right();
					ASSERT(read_value1_at(c) == false);
				}
				i = c;
				--layer;
			}
		}

		if (read_value1_at(i) == false)
			return i;

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
		return find_complement(destination, source, bin_t::ALL, twist);
	}


	bin_t binmap_t::find_complement(const binmap_t& destination, const binmap_t& source, bin_t range, const bin_t::uint_t twist)
	{
		ASSERT(source.binroot_.contains(range));
		ASSERT(destination.binroot_.contains(range));
		ASSERT(source.binroot_ == destination.binroot_);

		// Brute Force
		s32 const lbo = (s32)(range.base_left().layer_offset());
		s32 const n = (s32)(range.base_length());

		for (s32 i=0; i<n; ++i)
		{
			bin_t b(0, lbo + i);
			if (source.read_value1_at(b)==true && destination.read_value1_at(b)==false)
				return b;
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

		if (bin == binroot_ || bin == bin_t::ALL)
		{
			fill();
		}
		else if (binroot_.contains(bin))
		{
			const u32 root_layer = binroot_.layer();
			const u32 bin_layer  = (root_layer>0) ? bin.layer() : 0;

			// check if this action is changing the value to begin with
			// if not we can do an early-out
			
			// binmap0_
			{		
				bin_t ib = bin;
				bool change = false;
				{	// upwards
					u32 ib_layer = bin_layer;
					if (ib_layer == 0)
					{
						// check binmap1_ for any change
						change = read_value1_at(ib) == false && read_value1_at(ib.sibling()) == false;
						if (change)
						{
							ib.to_parent();
							ib_layer += 1;
						}
					}
					else
					{
						change = read_value0_at(ib) == false && read_value0_at(ib.sibling()) == false;
					}

					if (change == true)
					{
						bool iv = true;
						write_value0_at(ib, iv);

						// can we propagate up?
						while (ib_layer < root_layer)
						{
							ib.to_parent();
							++ib_layer;
							bool const oiv = update_value0_at(ib, iv);
							if (oiv == iv)
								break;
						};
					}
					else if (ib_layer > 0)
					{
						write_value0_at(ib, false);
					}
				}

				if (bin_layer > 1)
				{
					// fill everything below this bin
					// iterate down to layer 1
					// for every layer fill the range of bits
					bin_t il = bin.base_left();
					bin_t ir = bin.base_right();

					// determine start, end and length
					u32 const lo = (il.value() >> 3);
					u32 const ro = (ir.value() >> 3);

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

			// check if this action is changing the value to begin with
			// if not we can do an early-out
			// binmap1_
			if (read_value1_at(bin)==false)
			{				
				write_value1_at(bin, true);

				const u32 root_layer = binroot_.layer();
				const u32 bin_layer  = (root_layer>0) ? bin.layer() : 0;

				// can we propagate up?
				if (bin_layer < root_layer)
				{
					u32 ib_layer = bin_layer;
					bool iv = true;
					bin_t ib = bin;
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
					// fill everything below this bin
					// iterate down to layer 0
					// for every layer fill the range of bits
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

		if (bin == binroot_ || bin == bin_t::ALL)
		{
			clear();
		}
		else if (binroot_.contains(bin))
		{
			const u32 root_layer = binroot_.layer();
			const u32 bin_layer  = (root_layer>0) ? bin.layer() : 0;

			// check if this action is changing the value to begin with
			// if not we can do an early-out

			// binmap0_
			{	
				bin_t ib = bin;
				bool change = false;
				u32 ib_layer = bin_layer;

				{	// upwards
					if (ib_layer == 0)
					{
						// check binmap1_ for any change
						change = (read_value1_at(ib) == true) && (read_value1_at(ib.sibling()) == false);
						if (change)
						{
							ib.to_parent();
							ib_layer += 1;
						}
					}
					else
					{
						change = (read_value0_at(ib) == true) && (read_value0_at(ib.sibling()) == false);
					}

					if (change == true)
					{
						bool iv = false;
						write_value0_at(ib, iv);

						// can we propagate up?
						while (ib_layer < root_layer)
						{
							ib.to_parent();
							++ib_layer;
							bool const oiv = update_value0_at(ib, iv);
							if (oiv == iv)
								break;
						};
					}
					else if (ib_layer > 0)
					{
						write_value0_at(ib, false);
					}
				}

				if (bin_layer > 1)
				{
					// fill everything below this bin
					// iterate down to layer 1
					// for every layer fill the range of bits
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

						s32 const d = rb - lb;
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
			if (read_value1_at(bin)==true)
			{				
				write_value1_at(bin, false);

				// can we propagate up?
				if (bin_layer < root_layer)
				{
					u32 ib_layer = bin_layer;
					bool iv = false;
					bin_t ib = bin;
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
					// fill everything below this bin
					// iterate down to layer 0
					// for every layer fill the range of bits
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
						s32 const d = rb - lb;
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
		x_memzero(binmap1_, binmap1_size);
		x_memzero(binmap0_, binmap0_size);
	}


	/**
	* Fill the binmap. Creates a new filled binmap. Size is given by the source root
	*/
	void binmap_t::fill()
	{
		x_memset(binmap1_, 0xffffffff, binmap1_size);
		x_memset(binmap0_, 0xffffffff, binmap0_size);
	}

	/**
	* Get total size of the binmap
	*/
	size_t binmap_t::total_size() const
	{
		return sizeof(binmap_t) + binmap1_size + binmap0_size;
	}


	/**
	* Echo the binmap status to stdout
	*/
	void binmap_t::status() const
	{
		x_printf("bitmap:\n");
		s32 l = (s32)binroot_.layer();
		s32 f = (s32)binroot_.base_length();
		bin_t li = binroot_;
		bin_t ri = binroot_;
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
		x_printf("root bin: %llu\n", static_cast<unsigned long long>(binroot_.value()));
	}

	/**
	* Copy a binmap to another
	*/
	void binmap_t::copy(binmap_t& destination, const binmap_t& source)
	{
		destination.init(source.binroot_, destination.allocator_);
		x_memcpy(destination.binmap1_, source.binmap1_, destination.binmap1_size);
		x_memcpy(destination.binmap0_, source.binmap0_, destination.binmap0_size);
	}


	/**
	* Copy a range from one binmap to another binmap
	*/
	void binmap_t::copy(binmap_t& destination, const binmap_t& source, const bin_t& range)
	{
		if (!range.is_base() && !source.read_value0_at(range))
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