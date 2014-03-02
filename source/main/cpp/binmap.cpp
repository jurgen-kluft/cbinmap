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
	{

	}


	/**
	* Destructor
	*/
	binmap_t::~binmap_t()
	{
		allocator_->deallocate(binmap1_);
	}

	/**
	* Allocates one cell (dirty allocation)
	*/
	void binmap_t::init(const bin_t& _bin, allocator* _allocator)
	{
		if (allocator_ != NULL)
		{
			if (binmap1_ != NULL)
			{	
				allocator_->deallocate(binmap1_);
				binmap1_ = NULL;
			}
		}

		ASSERT(_bin.base_length() >= 2);
		ASSERT(_bin.is_base());
		allocator_ = _allocator;
		binmax_ = _bin;

		for (s32 i=0; i<32; ++i)
			layerToOffset_[i] = 0;

		binroot_ = _bin;
		while (binroot_.layer_offset() > 0)
			binroot_.to_parent();

		bin_t iter(_bin);
		s32 const max_layer = binroot_.layer();
		s32 iter_layer = 0;
		while (true)
		{
			if (iter_layer < max_layer)
			{
				const s32 bits  = ((iter.layer_offset() + 1) + 1) & 0xfffffffe;
				const s32 bytes = (bits + 7) / 8;
				layerToOffset_[iter_layer] = bytes;
				iter.to_parent();
				iter_layer++;
			}

			if (iter_layer == max_layer)
			{
				layerToOffset_[max_layer] = 1;
				break;
			}
		}

		// Remember the size of layer 0 before we convert it to an offset
		s32 const baseLayerSize = layerToOffset_[0];

		// Convert the layer sizes to offsets, do this so that layer 0 is 
		// at the end of the binmap1_ array.
		// This makes it easier for binmap0_ to not include layer 0.
		s32 totalSize = 0;
		for (s32 i=max_layer-1; i>=0; --i)
		{
			s32 const layerSize = layerToOffset_[i];
			layerToOffset_[i] = totalSize;
			totalSize += layerSize;
		}

		binmap1_size = totalSize;
		binmap1_ = (xbyte*)allocator_->allocate(binmap1_size, 8);
		binmap0_size = binmap1_size - baseLayerSize;
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

		s32 l = bin.layer();
		if (l == 0)
		{	// bin
			bool v = read_value1_at(bin);
			return v == false;
		}
		else
		{	// range
			bool const r = read_value0_at(binroot_);
			if (!r)
				return true;

			s32 c = (s32)bin.base_length();
			s32 i = (s32)bin.base_offset();
			s32 e = i + c + 1;
			while (i < e)
			{
				bool v = read_value1_at(bin_t(0, i));
				if (v)
					return false;
				++i;
			}
			return true;
		}
	}


	/**
	* Whether range/bin is filled
	*/
	bool binmap_t::is_filled(const bin_t& bin) const
	{
		bool v = read_value1_at(bin);
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
		// We can find a filled bin in this sub-tree
		bin_t i(binroot_);
		if (read_value0_at(i)==false)
			return bin_t::NONE;

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
		if (is_filled(binroot_))
		{	// the binmap is full
			return bin_t::NONE;
		}

		// traverse and keep trying to go up until a !filled(bin) is encountered.
		// when going up check if base_left() is still right or equal to the start bin
		
		bin_t i(start);
		if (i.is_base() && read_value1_at(i) == false)
			return i;

		i = i.to_right();
		bin_t const base_left = i.base_left();
		while (read_value1_at(i) == true)
		{
			if (i == binroot_)
			{
				ASSERTS(false, "error: root of tree is marked as filled, no empty bins available!");
				break;
			}

			// vertical - up
			bin_t p(i.parent());
			if (p.base_left() >= base_left)
			{
				i = p;
			}
			else
			{
				// horizontal
				i = bin_t(i.layer(), i.layer_offset() + 1);
			}
		}

		// Root of subtree must NOT be filled
		ASSERT (read_value1_at(i) == false);

		// We can find an empty bin in this sub-tree
		s32 l = i.layer();
		while (l > 0)
		{
			bin_t c = i.left();
			if (read_value1_at(c))
			{
				c = i.right();
				ASSERT(read_value1_at(c) == false);
			}
			i = c;
			--l;
		}
		return i;
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
		return bin_t(0,0);
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

		if (binroot_.contains(bin))
		{
			bin_t ib = bin;
			if (read_value1_at(bin) == false)
			{				
				// propagate up
				bool iv = true;
				while (true)
				{
					bool const oiv = update_value1_at(ib, iv);
					if (oiv == iv || ib == binroot_)
						break;

					bin_t const sib = ib.sibling();
					bool const sv = read_value1_at(sib);
					iv = (iv && sv);
					ib.to_parent();
				};

				// fill everything below this bin
				if (!bin.is_base())
				{
					// clear everything below this bin
					// iterate down to layer 0
					// for every layer clear the range of bits
					bin_t il = bin.left();
					bin_t ir = bin.right();
					s32 layer = il.layer();
					do 
					{
						// determine start, end and length
						xbyte* lb = binmap1_ + layerToOffset_[layer] + (il.layer_offset() >> 3);
						xbyte lm = 0xFF >> (il.layer_offset() & 0x07);

						xbyte* rb = binmap1_ + layerToOffset_[layer] + (ir.layer_offset() >> 3);
						xbyte rm = (xbyte)(0xFF80 >> (ir.layer_offset() & 0x07));

						s32 d = rb - lb;
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

						if (layer == 0)
							break;

						il.to_left();
						ir.to_right();
						--layer;
					} while (true);
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

		if (binroot_.contains(bin))
		{
			bin_t ib = bin;
			if (read_value1_at(ib) == true)
			{
				// propagate up
				bool iv = false;
				while (true)
				{
					bool const oiv = update_value1_at(ib, iv);
					if (oiv == iv || ib == binroot_)
						break;

					bin_t const sib = ib.sibling();
					bool const sv = read_value1_at(sib);
					iv = (iv && sv);
					ib.to_parent();
				};

				if (!bin.is_base())
				{
					// clear everything below this bin
					// iterate down to layer 0
					// for every layer clear the range of bits
					bin_t il = bin.left();
					bin_t ir = bin.right();
					s32 layer = il.layer();
					do 
					{
						// determine start, end and length
						xbyte* lb = binmap1_ + layerToOffset_[layer] + (il.layer_offset() >> 3);
						xbyte lm = (xbyte)(0xFF00 >> (il.layer_offset() & 0x07));

						xbyte* rb = binmap1_ + layerToOffset_[layer] + (ir.layer_offset() >> 3);
						xbyte rm = 0x7F >> (ir.layer_offset() & 0x07);

						s32 d = rb - lb;
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

						if (layer == 0)
							break;

						il.to_left();
						ir.to_right();
						--layer;
					} while (true);
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
		x_memset(binmap1_, 0xffffffff, binmap0_size);
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
		for (int i = 0; i < 16; ++i) 
		{
			for (int j = 0; j < 64; ++j) 
			{
				x_printf("%d", is_filled(bin_t(i * 64 + j)));
			}
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
		destination.init(source.binmax_, destination.allocator_);
		x_memcpy(destination.binmap1_, source.binmap1_, destination.binmap1_size);
		x_memcpy(destination.binmap0_, source.binmap0_, destination.binmap0_size);
	}


	/**
	* Copy a range from one binmap to another binmap
	*/
	void binmap_t::copy(binmap_t& destination, const binmap_t& source, const bin_t& range)
	{
		// @TODO
	}





}