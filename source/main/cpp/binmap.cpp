#include "xbase/x_debug.h"
#include "xbase/x_memory.h"
#include "xbase/x_runes.h"

#include "xbinmaps/binmap.h"

namespace xcore
{
	namespace binmaps
	{
		u32	data::size_for(bin_t _root)
		{
			u32 const data_size = (u32)(((((_root.base_length() * 2) + 7) / 8) * 2) + sizeof(bin_t));
			return data_size;
		}

		/**
		* Constructor
		*/
		cbinmap::cbinmap()
			: binroot_(0)
			, binmap1_(0)
			, binmap0_(0)
		{

		}

		cbinmap::cbinmap(user_data _data)
			: binroot_(0)
			, binmap1_(0)
			, binmap0_(0)
		{
			binroot_ = (bin_t*)_data.get_data();
			*binroot_ = _data.get_root();

			u32 const binmap_size = (data::size_for(_data.get_root()) - sizeof(bin_t)) / 2;
			binmap1_ = _data.get_data() + sizeof(bin_t);
			binmap0_ = binmap1_ + binmap_size;
		}

		cbinmap::cbinmap(data& _data)
			: binroot_(0)
			, binmap1_(0)
			, binmap0_(0)
		{
			binroot_ = (bin_t*)_data.get_data();
			*binroot_ = _data.get_root();

			u32 const binmap_size = (data::size_for(_data.get_root()) - sizeof(bin_t)) / 2;
			binmap1_ = _data.get_data() + sizeof(bin_t);
			binmap0_ = binmap1_ + binmap_size;
		}

		/**
		* Constructor
		*/
		binmap::binmap()
			: cbinmap()
		{
		}

		binmap::binmap(user_data _data)
			: cbinmap(_data)
		{

		}

		binmap::binmap(data& _data)
			: cbinmap(_data)
		{

		}

		/**
		* Whether binmap is empty
		*/
		bool cbinmap::is_empty() const
		{
			bool const r = read_om_at(*binroot_);
			return !r;
		}


		/**
		* Whether binmap is filled
		*/
		bool cbinmap::is_filled() const
		{
			return is_filled(*binroot_);
		}


		/**
		* Whether range/bin is empty
		*/
		bool cbinmap::is_empty(const bin_t& bin) const
		{
			bool r = false;
			if (bin == bin_t::ALL)
			{
				r = read_om_at(*binroot_);
			}
			else
			{
				ASSERT(binroot_->contains(bin));
				r = read_om_at(bin);
			}
			return !r;
		}


		/**
		* Whether range/bin is filled
		*/
		bool cbinmap::is_filled(const bin_t& bin) const
		{
			bool v = false;
			if (bin == bin_t::ALL)
			{
				v = read_am_at(*binroot_);
			}
			else
			{
				ASSERT(binroot_->contains(bin));
				v = read_am_at(bin);
			}
			return v;
		}


		/**
		* Return the topmost solid bin which covers the specified bin
		*/
		bin_t cbinmap::cover(const bin_t& bin) const
		{
			bin_t i(bin);
			bool v = read_am_at(i);
			if (!v)
				return i;

			while (i != *binroot_)
			{
				bin_t p = i.parent();
				v = read_am_at(p);
				if (!v)
					break;
				i = p;
			}
			return i;
		}


		/**
		* Find first empty bin
		*/
		bin_t cbinmap::find_empty() const
		{
			return find_empty(*binroot_);
		}


		/**
		* Find first filled bin
		*/
		bin_t cbinmap::find_filled() const
		{
			// Can we can find a filled bin in this sub-tree?
			if (read_om_at(*binroot_)==false)
				return bin_t::NONE;

			bin_t i(*binroot_);
			s32 l = i.layer();
			while (l >= 0)
			{
				bin_t c = i.left();
				if (read_om_at(c) == false)
				{
					c = i.right();
					ASSERT(read_om_at(c) == true);
				}
				i = c;
				--l;
			}
			return i;
		}


		/**
		* Find first empty bin right of start (start inclusive)
		*/
		bin_t cbinmap::find_empty(bin_t start) const
		{
			ASSERT(start != bin_t::ALL);

			// does start fall within this binmap?
			if (!binroot_->contains(start))
			{
				return bin_t::NONE;
			}

			if (read_am_at(*binroot_))
			{	// full, impossible to find an empty bin
				return bin_t::NONE;
			}

			bin_t i(start);
			u32 layer = i.layer();
			// traverse and keep trying to go up until a !filled(bin) is encountered.
			// when going up check if base_left() is still right or equal to the start bin
			do
			{
				if (read_om_at(i) == false)
					return i;

				// Does this sub-tree has a possible empty bin ?
				if (read_am_at(i) == false)
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
				if (read_om_at(i) == false)
					return i;

				bin_t c = i.left();
				if (read_am_at(c) == true)
				{
					c = i.right();
					ASSERT(read_am_at(c) == false);
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
		bin_t find_complement(const cbinmap& destination, const cbinmap& source, const bin_t::uint_t twist)
		{
			return find_complement(destination, source, source.root(), twist);
		}

		bin_t find_complement(const cbinmap& destination, const cbinmap& source, bin_t range, const bin_t::uint_t twist)
		{
			ASSERT(source.root().contains(range));
			ASSERT(destination.root().contains(range));
			ASSERT(source.root() == destination.root());

			// Brute Force
			s32 const lbo = (s32)(range.base_left().layer_offset());
			s32 const n = (s32)(range.base_length());

			for (s32 i=0; i<n; ++i)
			{
				bin_t b(0, lbo + i);
				if (source.read_am_at(b)==true && destination.read_am_at(b)==false)
				{
					// up
					while (b != source.root())
					{
						bin_t p = b.parent();
						if (source.read_am_at(p)==true && destination.read_om_at(p)==false)
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
		void binmap::set(const bin_t& bin)
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
					if (xchg_om_at(bin, true) == false)
					{
						bin_t ib = bin;
						u32 ib_layer = bin_layer;
						do
						{
							ib.to_parent();
							++ib_layer;

							bool const oiv = xchg_om_at(ib, true);
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

					if (!xchg_am_at(bin, true))
					{
						u32 ib_layer = bin_layer;
						bin_t ib = bin;
						bool iv = true;
						while (ib_layer < root_layer)
						{
							bool const sv = read_am_at(ib.sibling());
							iv = (iv && sv);
							ib.to_parent();
							++ib_layer;

							bool const oiv = xchg_am_at(ib, iv);
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
		void binmap::reset(const bin_t& bin)
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
					if (xchg_om_at(bin, false))
					{
						bin_t ib = bin;
						u32 ib_layer = bin_layer;
						while (ib_layer < root_layer)
						{
							bool sv = read_om_at(ib.sibling());
							if (sv)
								break;

							ib.to_parent();
							++ib_layer;
							bool const oiv = xchg_om_at(ib, false);
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
					if (xchg_am_at(bin, false))
					{
						bin_t ib = bin;
						u32 ib_layer = bin_layer;
						while (ib_layer < root_layer)
						{
							ib.to_parent();
							++ib_layer;
							bool const oiv = xchg_am_at(ib, false);
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
		void binmap::clear()
		{
			u32 const binmap_size = (u32)(((binroot_->base_length() * 2) + 7) / 8);
			x_memzero(binmap1_, binmap_size);
			x_memzero(binmap0_, binmap_size);
		}


		/**
		* Fill the binmap. Creates a new filled binmap. Size is given by the source root
		*/
		void binmap::fill()
		{
			u32 const binmap_size = (u32)(((binroot_->base_length() * 2) + 7) / 8);
			x_memset(binmap1_, 0xffffffff, binmap_size);
			x_memset(binmap0_, 0xffffffff, binmap_size);
		}

		/**
		* Get total size of the binmap
		*/
		xsize_t cbinmap::total_size() const
		{
			u32 const binmap_size = (u32)(((binroot_->base_length() * 2) + 7) / 8);
			return sizeof(binmap) + binmap_size + binmap_size;
		}

		void	binmap::copy_from(cbinmap const& _other)
		{
			ASSERT(root() != bin_t::NONE);
			ASSERT(root() == _other.root());

			u32 data_size = data::size_for(root());
			x_memcpy(binmap1_, _other.binmap1_, data_size);
			x_memcpy(binmap0_, _other.binmap0_, data_size);
		}

		/**
		* Copy a binmap to another
		*/
		void copy(binmap& destination, const cbinmap& source)
		{
			destination.copy_from(source);
		}


		/**
		* Copy a range from one binmap to another binmap
		*/
		void copy(binmap& destination, const cbinmap& source, const bin_t& range)
		{
			if (!source.read_om_at(range))
			{
				destination.reset(range);
			}
			else if (source.read_am_at(range))
			{
				destination.set(range);
			}
			else
			{
				s32 const e = (s32)(range.base_offset() + range.base_length());
				for (s32 i=(s32)range.base_offset(); i<e; ++i)
				{
					bin_t const b(0, i);
					bool const v = source.read_am_at(b);
					if (v) destination.set(b);
					else destination.reset(b);
				}
			}
		}

	}
}