#include "ccore/c_debug.h"
#include "cbase/c_memory.h"
#include "cbase/c_runes.h"

#include "cbinmaps/c_binmap.h"

namespace ncore
{
	namespace binmaps
	{
		static inline u32	data_size_for(bin_t _root)
		{
			u32 const data_size = (u32)(((((_root.base_length() * 2) + 7) / 8) * 2) + sizeof(bin_t));
			return data_size;
		}

		binmap::binmap()
            : binroot_(nullptr)
            , binmap1_(nullptr)
            , binmap0_(nullptr)
		{
		}

		binmap::binmap(bin_t root, byte* data)
			: binroot_((bin_t*)data)
			, binmap1_(data + sizeof(bin_t))
			, binmap0_(0)
		{
			*binroot_ = root;

			u32 const binmap_size = (data_size_for(root) - sizeof(bin_t)) / 2;
			binmap0_ = binmap1_ + binmap_size;
		}


		/**
		* Whether binmap is empty
		*/
		bool binmap::is_empty() const
		{
			bool const r = read_om_at(*binroot_);
			return !r;
		}


		/**
		* Whether binmap is filled
		*/
		bool binmap::is_filled() const
		{
			return is_filled(*binroot_);
		}


		/**
		* Whether range/bin is empty
		*/
		bool binmap::is_empty(const bin_t& bin) const
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
		bool binmap::is_filled(const bin_t& bin) const
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
		bin_t binmap::cover(const bin_t& bin) const
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
		bin_t binmap::find_empty() const
		{
			return find_empty(*binroot_);
		}


		/**
		* Find first filled bin
		*/
		bin_t binmap::find_filled() const
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
		bin_t binmap::find_empty(bin_t start) const
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
		bin_t find_complement(const binmap& destination, const binmap& source, const bin_t::uint_t twist)
		{
			return find_complement(destination, source, source.root(), twist);
		}

		bin_t find_complement(const binmap& destination, const binmap& source, bin_t range, const bin_t::uint_t twist)
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

						u8 const lm = 0xFF >> (il.value() & 0x07);
						u8 const rm = (u8)(0xFF80 >> (ir.value() & 0x07));

						{
							u8* lb = binmap0_ + lo;
							u8* rb = binmap0_ + ro;

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

						u8 const lm = 0xFF >> (il.value() & 0x07);
						u8 const rm = (u8)(0xFF80 >> (ir.value() & 0x07));

						{
							u8* lb = binmap1_ + lo;
							u8* rb = binmap1_ + ro;
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

						u8 const lm = (u8)(0xFF00 >> (il.value() & 0x07));
						u8 const rm = 0x7F >> (ir.value() & 0x07);

						{
							u8* lb = binmap0_ + llo;
							u8* rb = binmap0_ + lro;

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

						u8 const lm = (u8)(0xFF00 >> (il.value() & 0x07));
						u8 const rm = 0x7F >> (ir.value() & 0x07);

						{
							u8* lb = binmap1_ + llo;
							u8* rb = binmap1_ + lro;

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
			g_memclr(binmap1_, binmap_size);
			g_memclr(binmap0_, binmap_size);
		}


		/**
		* Fill the binmap. Creates a new filled binmap. Size is given by the source root
		*/
		void binmap::fill()
		{
			u32 const binmap_size = (u32)(((binroot_->base_length() * 2) + 7) / 8);
			g_memset(binmap1_, 0xffffffff, binmap_size);
			g_memset(binmap0_, 0xffffffff, binmap_size);
		}

		/**
		* Get total size of the binmap
		*/
		uint_t binmap::total_size() const
		{
			u32 const binmap_size = (u32)(((binroot_->base_length() * 2) + 7) / 8);
			return sizeof(binmap) + binmap_size + binmap_size;
		}

		/**
		* Copy a range from one binmap to another binmap
		*/
		void copy(binmap& destination, const binmap& source, const bin_t& range)
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
