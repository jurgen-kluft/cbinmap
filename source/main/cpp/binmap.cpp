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
		allocator_->deallocate(binmap_);
	}

	/**
	* Allocates one cell (dirty allocation)
	*/
	void binmap_t::init(const bin_t& _bin, allocator* _allocator)
	{
		if (allocator_ != NULL)
		{
			if (binmap_ != NULL)
			{	
				allocator_->deallocate(binmap_);
				binmap_ = NULL;
			}
		}

		allocator_ = _allocator;
		binmax_ = _bin;
		numset_ = 0;

		for (s32 i=0; i<32; ++i)
		{
			layerToOffset_[i] = 0;
			layerToWidth_[i] = 0;
		}

		binroot_ = _bin;
		while (binroot_.layer_offset() > 0)
			binroot_.to_parent();

		bin_t iter(_bin);
		s32 offset = 0;
		s32 const max_layer = binroot_.layer();
		s32 iter_layer = 0;
		while (true)
		{
			if (iter_layer < max_layer)
			{
				const s32 bits  = ((iter.layer_offset() + 1) + 1) & 0xfffffffe;
				const s32 bytes = (bits + 7) / 8;
				layerToOffset_[iter_layer] = offset;
				layerToWidth_[iter_layer] = bits;
				iter.to_parent();
				iter_layer++;
				offset += bytes;
			}

			if (iter_layer == max_layer)
			{
				layerToOffset_[max_layer] = offset;
				layerToWidth_[max_layer] = 1;
				offset += 1;
				break;
			}
		}
		binmap_size = offset;
		binmap_ = (xbyte*)allocator_->allocate(binmap_size, 8);
		clear();
	}

	/**
	* Whether binmap is empty
	*/
	bool binmap_t::is_empty() const
	{
		return numset_ == 0;
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
		if (numset_ == 0)
			return true;

		s32 l = bin.layer();
		if (l == 0)
		{	// bin
			bool v = value_at(bin);
			return v == false;
		}
		else
		{	// range
			s32 c = (s32)bin.base_length();
			s32 i = (s32)bin.base_offset();
			s32 e = i + c + 1;
			while (i < e)
			{
				bool v = value_at(bin_t(0, i));
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
		bool v = value_at(bin);
		return v;
	}


	/**
	* Return the topmost solid bin which covers the specified bin
	*/
	bin_t binmap_t::cover(const bin_t& bin) const
	{
		bin_t i(bin);
		bool v = value_at(i);
		if (!v)
			return i;

		while (i != binroot_)
		{
			bin_t p = i.parent();
			v = value_at(p);
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
		return bin_t(0,0);

	}


	/**
	* Find first empty bin right of start (start inclusive)
	*/
	bin_t binmap_t::find_empty(bin_t start) const
	{
		bool v = value_at(binroot_);
		if (v)
		{	// the binmap is full
			return bin_t::NONE;
		}
		
		bin_t i(start);
		v = value_at(i);
		if (v)
		{	// subtree is full
			return bin_t::NONE;
		}

		s32 l = i.layer();
		while (l > 0)
		{
			bin_t c = i.left();
			if (value_at(c))
			{
				c = i.right();
				ASSERT(value_at(c) == false);
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
			if (value_at(bin) == false)
			{
				++numset_;
				
				// propagate up
				bool iv = true;
				while (true)
				{
					bool const oiv = update_value_at(ib, iv);
					if (oiv == iv || ib == binroot_)
						break;

					bin_t const sib = ib.sibling();
					bool const sv = value_at(sib);
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
						xbyte* lb = binmap_ + layerToOffset_[layer] + (il.layer_offset() >> 3);
						xbyte lm = 0xFF >> (il.layer_offset() & 0x07);

						xbyte* rb = binmap_ + layerToOffset_[layer] + (ir.layer_offset() >> 3);
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
			if (value_at(ib) == true)
			{
				--numset_;

				// propagate up
				bool iv = false;
				while (true)
				{
					bool const oiv = update_value_at(ib, iv);
					if (oiv == iv || ib == binroot_)
						break;

					bin_t const sib = ib.sibling();
					bool const sv = value_at(sib);
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
						xbyte* lb = binmap_ + layerToOffset_[layer] + (il.layer_offset() >> 3);
						xbyte lm = (xbyte)(0xFF00 >> (il.layer_offset() & 0x07));

						xbyte* rb = binmap_ + layerToOffset_[layer] + (ir.layer_offset() >> 3);
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
		numset_ = 0;
		x_memzero(binmap_, binmap_size);
	}


	/**
	* Fill the binmap. Creates a new filled binmap. Size is given by the source root
	*/
	void binmap_t::fill()
	{
		numset_ = (u32)binmax_.layer_offset();
		x_memset(binmap_, 0xffffffff, binmap_size);
	}

	/**
	* Get total size of the binmap
	*/
	size_t binmap_t::total_size() const
	{
		return sizeof(*this) + binmap_size;
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
		x_memcpy(destination.binmap_, source.binmap_, destination.binmap_size);
	}


	/**
	* Copy a range from one binmap to another binmap
	*/
	void binmap_t::copy(binmap_t& destination, const binmap_t& source, const bin_t& range)
	{
		// @TODO
	}





}