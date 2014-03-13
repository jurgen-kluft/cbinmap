#include "xbase\x_target.h"
#include "xbase\x_integer.h"
#include "xbase\x_memory_std.h"
#include "xbinmaps\sigmap.h"

namespace xcore
{
	namespace sigmap
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

		s32 branch::push(signature_t const& _sig)
		{
			if (size_ < length_)
			{
				copy(array_[size_], _sig);
				size_++;
				return 1;
			}
			else
			{
				return 0;
			}
		}

		signature_t const* branch::operator[](s32 _index) const
		{
			if (_index < (s32)size_)
			{
				return &array_[_index];
			}
			else
			{
				return NULL;
			}
		}

		u32	data::size_for(bin_t _root, u32 _siglen)
		{
			u32 const data_size = (u32)((_root.base_length() * 2 * _siglen) + sizeof(bin_t));
			return data_size;
		}

		cmap::cmap()
			: root_bin_(0)
			, data_(0)
		{
		}

		cmap::cmap(data& _data)
			: root_bin_(0)
			, data_(0)
		{
			root_bin_ = (bin_t*)_data.get_data();
			*root_bin_ = _data.get_root();

			work_sig_ = signature_t(NULL, _data.get_siglen());
			work_sig_.digest_ = _data.get_data() + sizeof(bin_t);

			data_ = _data.get_data() + sizeof(bin_t) + _data.get_siglen();
			root_sig_ = signature_t(NULL, _data.get_siglen());
			get_sig_at(*root_bin_, root_sig_);
		}

		s32 cmap::read_branch(bin_t _bin, branch& _branch) const
		{
			// do we contain this bin ?
			if (!root_bin_->contains(_bin))
				return -1;	// out of range

			// is there enough space in the destination to write the full branch?
			if ((_branch.length() - _branch.size()) <= (u32)root_bin_->layer())
				return -2;

			signature_t sig;
			get_sig_at(_bin, sig);
			if (is_zero(sig))
				return -3;

			if (_bin != *root_bin_)
			{
				bin_t iter = _bin;
				if (iter.is_right())
					iter.to_sibling();

				get_sig_at(iter.sibling(), sig);
				_branch.push(sig);
				do
				{
					get_sig_at(iter.sibling(), sig);
					_branch.push(sig);
					iter.to_parent();
				} while (iter != *root_bin_);
			}
			_branch.push(root_sig_);

			// return the number of signatures that are part of the branch
			return _branch.size();
		}

		map::map()
			: cmap()
		{
		}

		map::map(data& _data, signature_t const& _rootsig, combine_f _sigcombiner)
			: cmap(_data)
			, combine_f_(_sigcombiner)
		{
			copy(root_sig_, _rootsig);
		}


		//
		// Verify if the signature branch results in the root signature.
		//
		// The signature branch is saved by first writing the signature
		// of the node together with the signature of its sibling.
		// Then when navigating up save the parent and its sibling etc..
		//
		s32 map::submit_branch(bin_t _bin, branch const& _branch)
		{
			// do we contain this bin ?
			if (!root_bin_->contains(_bin))
				return -2;	// out of range

			// First step: Determine if this branch resolves to the root signature
			s32 i = 0;
			signature_t const* lhs = _branch[i++];
			signature_t const* rhs = _branch[i++];

			bin_t iter = _bin;
			while (true)
			{
				combine_f_(*lhs, *rhs, work_sig_);
				iter.to_parent();
				if (iter == *root_bin_)
				{
					break;
				}
				else if (iter.is_left())
				{
					lhs = &work_sig_;
					rhs = _branch[i++];
				}
				else //if (_bin.is_right())
				{
					lhs = _branch[i++];
					rhs = &work_sig_;
				}
			}

			if (are_signatures_not_equal(root_sig_, work_sig_))
				return -3;

			// The branch is valid, proceed

			iter = _bin;
			
			i = 0;

			signature_t base_sig;
			get_sig_at(iter, base_sig);
			signature_t base_sibling_sig;
			get_sig_at(iter.sibling(), base_sibling_sig);

			// Increase the signature submit count whenever we are adding a signature in layer 0
			if (is_zero(base_sig))
				added_count_sig_++;

			if (is_zero(base_sibling_sig))
				added_count_sig_++;

			if (iter.is_left())
			{
				set_sig_at(iter, *_branch[i++]);
				set_sig_at(iter.sibling(), *_branch[i++]);
			}
			else
			{
				set_sig_at(iter.sibling(), *_branch[i++]);
				set_sig_at(iter, *_branch[i++]);
			}

			while (true)
			{
				iter.to_parent();
				if (iter == *root_bin_)
					break;
				set_sig_at(iter.sibling(), *_branch[i++]);
			}

			return -1;
		}


		map::builder::builder(data& _data, signature_t const& _rootsig, combine_f _sigcombiner)
			: combine_f_(_sigcombiner)
			, root_bin_(0)
			, data_(0)		
		{
			root_bin_ = (bin_t*)_data.get_data();
			*root_bin_ = _data.get_root();

			work_sig_ = signature_t(NULL, _data.get_siglen());
			work_sig_.digest_ = _data.get_data() + sizeof(bin_t);

			data_ = _data.get_data() + sizeof(bin_t) + _data.get_siglen();
			root_sig_ = signature_t(NULL, _data.get_siglen());
			get_sig_at(*root_bin_, root_sig_);

			u32 const data_size = data::size_for(*root_bin_, _data.get_siglen()) - sizeof(bin_t);
			x_memzero(_data.get_data() + sizeof(bin_t), data_size);
		}

		s32				map::builder::submit(bin_t _bin, signature_t const& _sig)
		{
			// do we contain this bin ?
			if (!root_bin_->contains(_bin))
				return -2;	// out of range

			signature_t s;
			get_sig_at(_bin, s);
			copy(s, _sig);

			return 1;
		}

		/*
		 Build the signature tree from the base level up until the root
		*/
		bool			map::builder::build()
		{
			bool is_complete = true;
			u32 w = (u32)root_bin_->base_length();
			for (u32 o=0; is_complete && o<w; ++o)
			{
				signature_t sig;
				get_sig_at(bin_t(0, o), sig);
				is_complete = is_complete || !is_zero(sig);
			}

			if (!is_complete)
				return false;

			bin_t ib(root_bin_->base_right());
			u32 layer = 0;
			while (ib != *root_bin_)
			{
				++layer;
				ib.to_parent();

				w = (u32)ib.layer_offset() + 1;
				for (u32 o=0; o<w; ++o)
				{
					signature_t psig;
					get_sig_at(bin_t(layer, o), psig);
					signature_t lsig, rsig;
					get_sig_at(bin_t(layer-1, (2*o)+0), lsig);
					get_sig_at(bin_t(layer-1, (2*o)+1), rsig);
					combine_f_(lsig, rsig, psig);
				}
			}

			return true;
		}

		/*
		 Build the signature tree from the base level up until the root and then
		 validate the root against the given root signature
		*/
		bool			map::builder::build_and_verify(signature_t const& _root_signature)
		{
			if (!build())
				return false;

			return are_signatures_equal(_root_signature, root_sig_);
		}
	}
}
