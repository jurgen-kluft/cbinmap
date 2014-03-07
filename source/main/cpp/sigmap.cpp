#include "xbase\x_target.h"
#include "xbase\x_integer.h"
#include "xbase\x_memory_std.h"
#include "xbinmaps\sigmap.h"

namespace xcore
{
	typedef		  sigmap::signature_t		xsig_t;
	typedef		  sigmap::iallocator		xallocator_t;
	typedef		  sigmap::sigcomb_f			xsigcomb_f;

	namespace sigmap
	{
		static inline bool is_zero(xsig_t const& _src)
		{
			u32 const* src = (u32 const*)_src.digest;
			u32 const* end = (u32 const*)(_src.digest + _src.length);
			bool is_empty = *src++ == 0;
			while (is_empty && src<end)
			{
				is_empty = (0 == *src++);
			}
			return is_empty;
		}

		static inline void copy(xsig_t& _dst, xsig_t const& _src)
		{
			ASSERT(_src.length == _dst.length);
			ASSERT(_src.digest != NULL);
			ASSERT(_dst.digest != NULL);
			x_memcopy4((u32*)_dst.digest, (u32 const*)_src.digest, _src.length / 4);
		}

		bool	are_signatures_equal (signature_t const& _a, signature_t const& _b)
		{
			ASSERT(_a.length == _b.length);
			ASSERT(_a.digest != NULL);
			ASSERT(_b.digest != NULL);

			// This is a special case where we assume the signature is aligned on 4 bytes
			u32 const* lhs = (u32 const*)_a.digest;
			u32 const* rhs = (u32 const*)_b.digest;
			u32 const* end = (u32 const*)(_b.digest + _b.length);
			bool equal = (*lhs++ == *rhs++);
			while (equal && rhs<end)
			{
				equal = (*lhs++ == *rhs++);
			}
			return equal;
		}

		bool	are_signatures_not_equal (signature_t const& _a, signature_t const& _b)
		{
			ASSERT(_a.length == _b.length);
			ASSERT(_a.digest != NULL);
			ASSERT(_b.digest != NULL);

			// This is a special case where we assume the signature is aligned on 4 bytes
			u32 const* lhs = (u32 const*)_a.digest;
			u32 const* rhs = (u32 const*)_b.digest;
			u32 const* end = (u32 const*)(_b.digest + _b.length);
			bool equal = (*lhs++ == *rhs++);
			while (equal && rhs<end)
			{
				equal = (*lhs++ == *rhs++);
			}
			return equal;
		}

		s32		compare_signatures (const signature_t& _a, const signature_t& _b)
		{
			ASSERT(_a.length == _b.length);
			ASSERT(_a.digest != NULL);
			ASSERT(_b.digest != NULL);

			// This is a special case where we assume the signature is aligned on 4 bytes
			u8 const* lhs = (u8 const*)(_a.digest);
			u8 const* end = (u8 const*)(_a.digest + _a.length);
			u8 const* rhs = (u8 const*)(_b.digest);
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
	}

	xsigmap::xsigmap(xallocator_t* _allocator, u32 _signature_len, xsigcomb_f _sig_combiner)
		: allocator(_allocator)
		, sigComb(_sig_combiner)
		, signatureDataArray(0)
		, is_open(false)
		, is_verified(false)
		, is_valid(false)
	{
	}

	xsigmap::~xsigmap()
	{
		close();
	}

	void			xsigmap::open(xsig_t const& _rootsig, u32 _count)
	{
		close();

		is_open = true;
		is_verified = false;
		is_valid = false;

		rootBin = bin_t(0, _count - 1);
		while (rootBin.layer_offset() > 0)
		{
			rootBin.to_parent();
		}
		ASSERT(rootBin.contains(bin_t(0, _count - 1)));

		ASSERT((_rootsig.length & 0x3) == 0);	// signatures should be a multiple of 4 bytes
		ASSERT(allocator != NULL);
		ASSERT(sigComb != NULL);

		bin_t iterBin = bin_t(0, _count - 1);
		u32 offset = 0;
		while (true)
		{
			u32 const width = (((u32)iterBin.layer_offset() + 1) + 1) & 0xfffffffe;
			u32 const layer = iterBin.layer();
			layerToWidth[layer] = width;
			layerToOffset[layer] = offset;
			iterBin.to_parent();
			offset += width;
			if (iterBin == rootBin)
			{
				u32 const rlayer = iterBin.layer();
				layerToWidth[rlayer] = 1;
				layerToOffset[rlayer] = offset;
				offset += 1;
				break;
			}
		}
		u32 const size = offset * _rootsig.length;

		countSig = _count;
		submitSig = 0;

		signatureDataArray = (xbyte*)allocator->allocate(size, sizeof(void*));
		x_memzero(signatureDataArray, size);

		allocator->allocate(workSig);
		statistics.incSig();

		rootSig = xsig_t(NULL, _rootsig.length);
		get_signature_at(rootBin, rootSig);
		copy(rootSig, _rootsig);
	}

	void			xsigmap::close()
	{
		// Traverse the tree and deallocate nodes, leafs and hashes
		if (is_open)
		{
			allocator->deallocate(signatureDataArray);
			signatureDataArray = NULL;

			statistics.decSig();
			allocator->deallocate(workSig);

			is_open = false;
		}
	}

	bool			xsigmap::verify()
	{
		if (is_open && !is_verified)
		{
			// @TODO: This is not part of the final implementation
			s32 max_layer = rootBin.layer()-1;
			for (s32 l=1; l<=max_layer; ++l)
			{
				s32 w = layerToWidth[l];
				for (s32 o=0; o<w; ++o)
				{
					xsig_t psig;
					get_signature_at(bin_t(l, o), psig);
					xsig_t lsig, rsig;
					get_signature_at(bin_t(l-1, (2*o)+0), lsig);
					get_signature_at(bin_t(l-1, (2*o)+1), rsig);

					sigComb(lsig, rsig, psig);
				}
			}	

			xsig_t lsig, rsig;
			get_signature_at(bin_t(max_layer, 0), lsig);
			get_signature_at(bin_t(max_layer, 1), rsig);
			sigComb(lsig, rsig, workSig);

			is_verified = true;
			is_valid = are_signatures_equal(rootSig, workSig);

			close();
		}
		return is_valid;
	}

	bool			xsigmap::valid() const
	{
		return is_valid;
	}

	s32				xsigmap::submit(bin_t _bin, xsig_t const& _sig)
	{
		// do we contain this bin ?
		if (!rootBin.contains(_bin))
			return -2;	// out of range

		xsig_t s;
		get_signature_at(_bin, s);
		copy(s, _sig);

		return 1;
	}

	/*
	 Build the signature tree from the base level up until the root
	*/
	bool			xsigmap::build()
	{
		bool is_complete = true;
		s32 w = layerToWidth[0];
		for (s32 o=0; is_complete && o<w; ++o)
		{
			xsig_t sig;
			get_signature_at(bin_t(0, o), sig);
			is_complete = is_complete || !is_zero(sig);
		}

		if (!is_complete)
			return false;

		s32 max_layer = rootBin.layer();
		for (s32 l=1; l<=max_layer; ++l)
		{
			w = layerToWidth[l];
			for (s32 o=0; o<w; ++o)
			{
				xsig_t psig;
				get_signature_at(bin_t(l, o), psig);
				xsig_t lsig, rsig;
				get_signature_at(bin_t(l-1, (2*o)+0), lsig);
				get_signature_at(bin_t(l-1, (2*o)+1), rsig);
				sigComb(lsig, rsig, psig);
			}
		}
		is_valid = true;
		return is_valid;
	}

	//
	// Verify if the signature branch results in the root signature.
	//
	// The signature branch is saved by first writing the signature
	// of the node together with the signature of its sibling.
	// Then when navigating up save the parent and its sibling etc..
	//
	s32 xsigmap::submit_branch(bin_t _bin, xsig_t const * _branch_signatures)
	{
		// do we contain this bin ?
		if (!rootBin.contains(_bin))
			return -2;	// out of range

		// First step: Determine if this branch resolves to the root signature
		xsig_t const* sit = _branch_signatures;
		xsig_t const* lhs = sit++;
		xsig_t const* rhs = sit++;

		bin_t iter = _bin;
		while (true)
		{
			sigComb(*lhs, *rhs, workSig);
			iter.to_parent();
			if (iter == rootBin)
			{
				break;
			}
			else if (iter.is_left())
			{
				lhs = &workSig;
				rhs = sit++;
			}
			else //if (_bin.is_right())
			{
				lhs = sit++;
				rhs = &workSig;
			}
		}

		if (are_signatures_not_equal(rootSig, workSig))
			return -3;

		// The branch is valid, proceed

		iter = _bin;
		sit = _branch_signatures;

		xsig_t base_sig;
		get_signature_at(iter, base_sig);
		xsig_t base_sibling_sig;
		get_signature_at(iter.sibling(), base_sibling_sig);

		// Increase the signature submit count whenever we are adding a signature in layer 0
		if (is_zero(base_sig))
			submitSig++;
		if (is_zero(base_sibling_sig))
			submitSig++;
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
			if (iter == rootBin)
				break;
			set_signature_at(iter.sibling(), *sit++);
		}

		return -1;
	}

	s32 xsigmap::read_branch(bin_t _bin, xsig_t* _branch_signatures, u32 _max_branch_signatures) const
	{
		// do we contain this bin ?
		if (!rootBin.contains(_bin))
			return -1;	// out of range

		// is there enough space in the destination to write the full branch?
		if (_max_branch_signatures <= rootBin.layer())
			return -2;

		xsig_t sig;
		get_signature_at(_bin, sig);
		if (is_zero(sig))
			return -3;

		sigmap::signature_t* dst = _branch_signatures;
		if (_bin != rootBin)
		{
			bin_t iter = _bin;
			if (iter.is_right())
				iter.to_sibling();

			get_signature_at(iter.sibling(), sig);
			copy(*dst++, sig);

			sigmap::signature_t* end = _branch_signatures + _max_branch_signatures;
			do
			{
				get_signature_at(iter.sibling(), sig);
				copy(*dst++, sig);
				iter.to_parent();
			} while (iter != rootBin);
		}

		copy(*dst++, rootSig);

		// return the number of signatures that are part of the branch
		const s32 n = dst - _branch_signatures;
		return n;
	}

	s32 xsigmap::set_signature_at(bin_t _bin, xsig_t const& _in_signature)
	{
		// do we contain this bin ?
		if (!rootBin.contains(_bin))
			return -2;	// out of range

		u32 const layer  = _bin.layer();
		u32 const layer_offset = (u32)_bin.layer_offset();

		u32 const signature_offset = layerToOffset[layer] + layer_offset;
		xsig_t s = xsig_t(signatureDataArray + signature_offset, rootSig.length);

		copy(s, _in_signature);

		return 0;
	}


}
