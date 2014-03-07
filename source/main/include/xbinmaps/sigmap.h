// x_sigmap.h - Core Signature Map
#ifndef __XSIGV_SIGMAP_H__
#define __XSIGV_SIGMAP_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "xbinmaps/bin.h"

namespace xcore
{
#ifdef TARGET_32BIT
	typedef		u64			iptr;
#else
	typedef		u32			iptr;
#endif

	namespace sigmap
	{	
		struct signature_t
		{
			inline			signature_t() : digest(0), length(0) {}
			inline			signature_t(xbyte* d, u32 l) : digest(d), length(l) {}
			xbyte*			digest;
			u32				length;
		};

		bool	are_signatures_equal (signature_t const& _a, signature_t const& _b);
		bool	are_signatures_not_equal (signature_t const& _a, signature_t const& _b);
		s32		compare_signatures (const signature_t& _a, const signature_t& _b);

		// note: @_out can be one of _left or _right, so the combiner needs to make sure that @_out is written to
		//       after the combined signature is computed.
		typedef void (*sigcomb_f)(signature_t const& _left, signature_t const& _right, signature_t& _out);

		class iallocator
		{
		public:
			virtual void*	allocate(xsize_t _size, u32 _alignment) = 0;
			virtual void	deallocate(void*) = 0;

			virtual void	allocate(signature_t&) = 0;
			virtual void	deallocate(signature_t&) = 0;
		};

		struct stats
		{
			inline			stats()				{ reset(); }

			void			reset()				{ numSigs=0; maxSigs=0; numCombs=0; }
			void			incSig()			{ ++numSigs; if (numSigs > maxSigs) maxSigs = numSigs; }
			void			decSig()			{ --numSigs; }
			void			incCombs()			{ ++numCombs; }

		private:
			u32				numSigs;
			u32				maxSigs;
			u32				numCombs;
		};
	}


	/**
	 * @group		xsigmap
	 * @brief		Signature Map implementation used for content validation
	 * @URL         Reference -> http://www.merkle.com/‎
	 * 
	 * @required    @_rootsig is the root signature
	 *				@_max_bins is the number of bins
	 *				@_sigcombiner can combine 2 signatures into 1
	 *				@_allocator allocator for allocating nodes and signatures
	 *
	 * @behavior	Submitting a signature is going to allocate nodes, leafs and signatures and if submitted
	 *              in the worst case order (010101....010101) then this data structure will use the most
	 *              amount of memory.
	 *				So the best is to submit signatures in a continues order (increasing or decreasing bin).
	 *              The signature map will collapse sub-trees when it can combine the signature. 
	 * 
	 * @example		You want to set a signature on it's bin :
	 *              
	 *                sigmap->submit(bin_t(sig_bin), sig);
	 *              
	 */
	class xsigmap
	{
	public:
						xsigmap(sigmap::iallocator* _allocator, u32 _signature_len, sigmap::sigcomb_f _sigcombiner);
						~xsigmap();

		void			open(sigmap::signature_t const& _rootsig, u32 _count);
		void			close();

		bool			verify();
		bool			valid() const;

		s32				submit(bin_t _bin, sigmap::signature_t const& _signature);	// return: 1=added, -1 if this was the last signature of a trusted sub-tree that failed to result in the trusted signature
		bool			build();

		s32				submit_branch(bin_t _bin, sigmap::signature_t const * _branch_signatures);
		s32				read_branch(bin_t _bin, sigmap::signature_t* _branch_signatures, u32 _max_branch_signatures) const;

	private:
		s32				get_signature_at(bin_t, sigmap::signature_t& _out_signature) const;
		s32				set_signature_at(bin_t, sigmap::signature_t const & _in_signature);

		sigmap::iallocator*		allocator;
		sigmap::stats			statistics;
		sigmap::sigcomb_f		sigComb;

		sigmap::signature_t		rootSig;
		sigmap::signature_t		workSig;

		u32						countSig;
		u32						submitSig;

		bin_t					rootBin;
		u32						layerToOffset[32];
		u32						layerToWidth[32];
		xbyte*					signatureDataArray;

		bool					is_open;
		bool					is_verified;
		bool					is_valid;
	};


	inline s32 xsigmap::get_signature_at(bin_t _bin, sigmap::signature_t& _out_signature) const
	{
		// do we contain this bin ?
		if (!rootBin.contains(_bin))
			return -2;	// out of range

		u32 const layer  = _bin.layer();
		u32 const layer_offset = (u32)_bin.layer_offset();

		u32 const signature_offset = layerToOffset[layer] + layer_offset;
		_out_signature = sigmap::signature_t(signatureDataArray + (signature_offset * rootSig.length), rootSig.length);
		return 0;
	}

}
#endif	// __XSIGV_SIGMAP_H__
