#include "xbase\x_target.h"
#include "xbase\x_integer.h"
#include "xbase\x_memory_std.h"
#include "xbinmaps\sigmap.h"

namespace xcore
{
	namespace xsig
	{
		static inline bool is_zero(signature_t const& _src)
		{
			u32 const* src = (u32 const*)_src.digest_;
			u32 const* end = (u32 const*)(_src.digest_ + _src.length_);
			bool is_empty = *src++ == 0;
			while (is_empty && src<end)
			{
				is_empty = (0 == *src++);
			}
			return is_empty;
		}

		static inline void copy(signature_t& _dst, signature_t const& _src)
		{
			ASSERT(_src.length_ == _dst.length_);
			ASSERT(_src.digest_ != NULL);
			ASSERT(_dst.digest_ != NULL);
			x_memcopy4((u32*)_dst.digest_, (u32 const*)_src.digest_, _src.length_ / 4);
		}

		bool	are_signatures_equal (signature_t const& _a, signature_t const& _b)
		{
			ASSERT(_a.length_ == _b.length_);
			ASSERT(_a.digest_ != NULL);
			ASSERT(_b.digest_ != NULL);

			// This is a special case where we assume the signature is aligned on 4 bytes
			u32 const* lhs = (u32 const*)_a.digest_;
			u32 const* rhs = (u32 const*)_b.digest_;
			u32 const* end = (u32 const*)(_b.digest_ + _b.length_);
			bool equal = (*lhs++ == *rhs++);
			while (equal && rhs<end)
			{
				equal = (*lhs++ == *rhs++);
			}
			return equal;
		}

		bool	are_signatures_not_equal (signature_t const& _a, signature_t const& _b)
		{
			ASSERT(_a.length_ == _b.length_);
			ASSERT(_a.digest_ != NULL);
			ASSERT(_b.digest_ != NULL);

			// This is a special case where we assume the signature is aligned on 4 bytes
			u32 const* lhs = (u32 const*)_a.digest_;
			u32 const* rhs = (u32 const*)_b.digest_;
			u32 const* end = (u32 const*)(_b.digest_ + _b.length_);
			bool equal = (*lhs++ == *rhs++);
			while (equal && rhs<end)
			{
				equal = (*lhs++ == *rhs++);
			}
			return equal;
		}

		s32		compare_signatures (const signature_t& _a, const signature_t& _b)
		{
			ASSERT(_a.length_ == _b.length_);
			ASSERT(_a.digest_ != NULL);
			ASSERT(_b.digest_ != NULL);

			// This is a special case where we assume the signature is aligned on 4 bytes
			u8 const* lhs = (u8 const*)(_a.digest_);
			u8 const* end = (u8 const*)(_a.digest_ + _a.length_);
			u8 const* rhs = (u8 const*)(_b.digest_);
			while (lhs < end)
			{
				if (*lhs < *rhs)
					return -1;
				else if (*lhs > *rhs)
					return 1;
				++lhs;
				++rhs;
			}
			return 0;
		}

		map::map(jallocator* _allocator)
			: allocator_(_allocator)
			, imp_(NULL)
		{
		}

		map::map(builder& _builder)
			: allocator_(_builder.allocator_)
			, imp_(_builder.imp_)
		{
		}

		map::~map()
		{
			close();
		}

		void			map::open(signature_t const& _rootsig, combine_f _sigcombiner, u32 _count)
		{
			imp_ = (imp*)allocator_->allocate(sizeof(imp), sizeof(void*));
			imp_->init(allocator_, _sigcombiner);
			imp_->open(_rootsig, _count);
		}

		void			map::close()
		{
			if (imp_!=NULL)
			{
				imp_->close();
				allocator_->deallocate(imp_);
				imp_ = NULL;
			}
		}

		s32				map::submit_branch(bin_t _bin, signature_t const * _branch_signatures)
		{
			return imp_->submit_branch(_bin, _branch_signatures);
		}

		s32				map::read_branch(bin_t _bin, signature_t* _branch_signatures, u32 _max_branch_signatures) const
		{
			return imp_->read_branch(_bin, _branch_signatures, _max_branch_signatures);
		}


		void map::imp::init(jallocator* _allocator, combine_f _sig_combiner)
		{
			allocator_ = _allocator;
			combine_f_ = _sig_combiner;
			state_ = EMPTY;
			root_signature_ = signature_t();
			work_signature_ = signature_t();
			root_bin_ = bin_t::NONE;
			root_layer = 0;
			signature_data_array_ = 0;
		}

		void map::imp::exit()
		{
			if (signature_data_array_!=NULL)
			{
				allocator_->deallocate(signature_data_array_);
				signature_data_array_ = NULL;
				root_signature_ = signature_t();
			}

			if (work_signature_.digest_!=NULL)
			{
				allocator_->deallocate(work_signature_);
			}

			root_bin_ = bin_t::NONE;
			state_    = EMPTY;

			count_signature_ = 0;
			submit_signature_ = 0;
		}

		void			map::imp::open(signature_t const& _root_signature_, u32 _count)
		{
			ASSERT(state_ == EMPTY || state_ == CLOSED);

			state_ = OPEN;

			root_bin_ = bin_t(0, _count - 1);
			while (root_bin_.layer_offset() > 0)
				root_bin_.to_parent();
			ASSERT(root_bin_.contains(bin_t(0, _count - 1)));

			ASSERT((_root_signature_.length_ & 0x3) == 0);	// signatures should be a multiple of 4 bytes
			ASSERT(allocator_ != NULL);
			ASSERT(combine_f_ != NULL);

			u32 const size = (u32)((root_bin_.base_length() * 2) - 1) * _root_signature_.length_;

			count_signature_ = _count;
			submit_signature_ = 0;

			signature_data_array_ = (xbyte*)allocator_->allocate(size, sizeof(void*));
			x_memzero(signature_data_array_, size);

			allocator_->allocate(work_signature_);

			root_signature_ = signature_t(NULL, _root_signature_.length_);
			get_signature_at(root_bin_, root_signature_);
			copy(root_signature_, _root_signature_);
		}

		void			map::imp::close()
		{
			// Traverse the tree and deallocate nodes, leafs and hashes
			if (state_ == OPEN || state_ == FINAL)
			{
				exit();
				state_ = CLOSED;
			}
		}

		map::builder::builder(jallocator* _allocator)
			: allocator_(_allocator)
			, imp_(NULL)
		{

		}

		bool			map::builder::valid() const
		{
			return imp_->state_ == imp::FINAL;
		}

		void			map::builder::open(signature_t const& _rootsig, combine_f _sigcombiner, u32 _count)
		{
			ASSERT(imp_ == NULL);
			imp_ = (imp*)allocator_->allocate(sizeof(imp), sizeof(void*));
			imp_->init(allocator_, _sigcombiner);
			imp_->open(_rootsig, _count);
		}

		void			map::builder::close()
		{
			if (imp_!=NULL)
			{
				imp_->close();
				allocator_->deallocate(imp_);
				imp_ = NULL;
			}
		}

		s32				map::builder::submit(bin_t _bin, signature_t const& _sig)
		{
			// do we contain this bin ?
			if (!imp_->root_bin_.contains(_bin))
				return -2;	// out of range

			signature_t s;
			imp_->get_signature_at(_bin, s);
			copy(s, _sig);

			return 1;
		}

		/*
		 Build the signature tree from the base level up until the root
		*/
		bool			map::builder::build()
		{
			bool is_complete = true;
			u32 w = (u32)imp_->root_bin_.base_length();
			for (u32 o=0; is_complete && o<w; ++o)
			{
				signature_t sig;
				imp_->get_signature_at(bin_t(0, o), sig);
				is_complete = is_complete || !is_zero(sig);
			}

			if (!is_complete)
				return false;

			bin_t ib(imp_->root_bin_.base_right());
			u32 layer = 0;
			while (ib != imp_->root_bin_)
			{
				++layer;
				ib.to_parent();

				w = (u32)ib.layer_offset() + 1;
				for (u32 o=0; o<w; ++o)
				{
					signature_t psig;
					imp_->get_signature_at(bin_t(layer, o), psig);
					signature_t lsig, rsig;
					imp_->get_signature_at(bin_t(layer-1, (2*o)+0), lsig);
					imp_->get_signature_at(bin_t(layer-1, (2*o)+1), rsig);
					imp_->combine_f_(lsig, rsig, psig);
				}
			}
			imp_->state_ = imp::FINAL;
			return true;
		}

		//
		// Verify if the signature branch results in the root signature.
		//
		// The signature branch is saved by first writing the signature
		// of the node together with the signature of its sibling.
		// Then when navigating up save the parent and its sibling etc..
		//
		s32 map::imp::submit_branch(bin_t _bin, signature_t const * _branch_signatures)
		{
			// do we contain this bin ?
			if (!root_bin_.contains(_bin))
				return -2;	// out of range

			// First step: Determine if this branch resolves to the root signature
			signature_t const* sit = _branch_signatures;
			signature_t const* lhs = sit++;
			signature_t const* rhs = sit++;

			bin_t iter = _bin;
			while (true)
			{
				combine_f_(*lhs, *rhs, work_signature_);
				iter.to_parent();
				if (iter == root_bin_)
				{
					break;
				}
				else if (iter.is_left())
				{
					lhs = &work_signature_;
					rhs = sit++;
				}
				else //if (_bin.is_right())
				{
					lhs = sit++;
					rhs = &work_signature_;
				}
			}

			if (are_signatures_not_equal(root_signature_, work_signature_))
				return -3;

			// The branch is valid, proceed

			iter = _bin;
			sit = _branch_signatures;

			signature_t base_sig;
			get_signature_at(iter, base_sig);
			signature_t base_sibling_sig;
			get_signature_at(iter.sibling(), base_sibling_sig);

			// Increase the signature submit count whenever we are adding a signature in layer 0
			if (is_zero(base_sig))
				submit_signature_++;
			if (is_zero(base_sibling_sig))
				submit_signature_++;
			if (iter.is_left())
			{
				set_signature_at(iter, *sit++);
				set_signature_at(iter.sibling(), *sit++);
			}
			else
			{
				set_signature_at(iter.sibling(), *sit++);
				set_signature_at(iter, *sit++);
			}

			while (true)
			{
				iter.to_parent();
				if (iter == root_bin_)
					break;
				set_signature_at(iter.sibling(), *sit++);
			}

			return -1;
		}

		s32 map::imp::read_branch(bin_t _bin, signature_t* _branch_signatures, u32 _max_branch_signatures) const
		{
			// do we contain this bin ?
			if (!root_bin_.contains(_bin))
				return -1;	// out of range

			// is there enough space in the destination to write the full branch?
			if (_max_branch_signatures <= (u32)root_bin_.layer())
				return -2;

			signature_t sig;
			get_signature_at(_bin, sig);
			if (is_zero(sig))
				return -3;

			signature_t* dst = _branch_signatures;
			if (_bin != root_bin_)
			{
				bin_t iter = _bin;
				if (iter.is_right())
					iter.to_sibling();

				get_signature_at(iter.sibling(), sig);
				copy(*dst++, sig);

				signature_t* end = _branch_signatures + _max_branch_signatures;
				do
				{
					get_signature_at(iter.sibling(), sig);
					copy(*dst++, sig);
					iter.to_parent();
				} while (iter != root_bin_);
			}

			copy(*dst++, root_signature_);

			// return the number of signatures that are part of the branch
			const s32 n = dst - _branch_signatures;
			return n;
		}

		s32 map::imp::set_signature_at(bin_t _bin, signature_t const& _in_signature)
		{
			// do we contain this bin ?
			if (!root_bin_.contains(_bin))
				return -2;	// out of range

			u32 const signature_offset = (u32)_bin.value() * root_signature_.length_;
			signature_t s = signature_t(signature_data_array_ + signature_offset, root_signature_.length_);

			copy(s, _in_signature);

			return 0;
		}

	}
}
