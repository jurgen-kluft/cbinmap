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
	namespace sigv
	{
		struct node_t;

		struct signature_t
		{
			inline			signature_t() : digest(0), length(0) {}
			inline			signature_t(xbyte* d, u32 l) : digest(d), length(l) {}
			xbyte*			digest;
			u32				length;
		};

		// note: @_out can be one of _left or _right, so the combiner needs to make sure that @_out is written to
		//       after the combined signature is computed.
		typedef void (*sigcomb_f)(signature_t const& _left, signature_t const& _right, signature_t* _out);

		class iallocator
		{
		public:
			virtual void		initialize(u32 _sizeof_node, u32 _sizeof_hash) = 0;

			virtual node_t*		node_allocate() = 0;
			virtual void		node_deallocate(node_t*) = 0;

			virtual signature_t	sig_allocate() = 0;
			virtual void		sig_deallocate(signature_t&) = 0;
		};
	}


	/**
	 * @group		xsigv
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
						xsigmap(sigv::iallocator* _allocator, sigv::sigcomb_f _sigcombiner);
						~xsigmap();

		void			open(sigv::signature_t const& _rootsig, u32 _max_bins, bool _fold=true);
		void			close();

		bool			verify();
		bool			valid() const;

		s32				submit(bin_t _bin, sigv::signature_t const& _signature);	// return: 1=added, -1 if this was the last signature of a trusted sub-tree that failed to result in the trusted signature
		bool			submit_branch(bin_t _bin, sigv::signature_t* const* _branch_signatures);

		bool			read_branch(bin_t _bin, sigv::signature_t* _branch_signatures, u32 _max_branch_signatures);

	private:
		sigv::iallocator*	allocator;
		sigv::sigcomb_f		combiner;

		enum emode { ETRAVERSE_NORMAL, ETRAVERSE_EXPAND };
		u32				traverse(bin_t _node_bin, sigv::node_t* _node_ptr, bin_t _tofind, sigv::node_t** _stack, u32& _stack_depth, emode _traverse_mode=ETRAVERSE_NORMAL);

		struct stats
		{
			inline				stats()				{ reset(); }

			void				reset()				{ numNodes=0; maxNodes=0; numSigs=0; maxSigs=0; numCombs=0; }
			void				incNode()			{ ++numNodes; if (numNodes > maxNodes) maxNodes = numNodes; }
			void				decNode()			{ --numNodes; }
			void				incSig()			{ ++numSigs; if (numSigs > maxSigs) maxSigs = numSigs; }
			void				decSig()			{ --numSigs; }
			void				incCombs()			{ ++numCombs; }

		private:
			u32					numNodes;
			u32					maxNodes;
			u32					numSigs;
			u32					maxSigs;
			u32					numCombs;
		};
		stats				statistics;

		sigv::signature_t	workSig;

		bin_t				rootBin;
		sigv::signature_t	rootSig;
		sigv::node_t*		rootNode;
		bool				is_open;
		bool				do_fold;
		bool				is_verified;
		bool				is_valid;
	};


}
#endif	// __XSIGV_SIGMAP_H__
