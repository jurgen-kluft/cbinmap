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
	 *              in the worst case random order (010101....010101) then this data structure will
	 *              use the most amount of memory.
	 *				So the aim is to submit signatures in a continues order (increasing or decreasing bin).
	 *              The signature map will collapse sub-trees when it can combine the signature. The user
	 *              can submit a trusted signature into the map with @submit_sig().
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
		s32				failed() const;												// return: number of trusted sub-trees signatures that failed

		s32				submit(bin_t _bin, sigv::signature_t const& _signature);	// return: 1=added, -1 if this was the last signature of a trusted sub-tree that failed to result in the trusted signature
		bool			submit_branch(bin_t _bin, sigv::signature_t* const* _branch_signatures);

		s32				find_unsigned(bin_t& _bin) const;							// return: 0=did not find bin, 1=item was found
		s32				find_unsigned(bin_t* _bins, u32 _max) const;				// return: 0=did not find any bin, >0=number of items found

	private:
		sigv::iallocator*	allocator;
		sigv::sigcomb_f		combiner;

		struct trace
		{
			inline			trace() : node_ptr(0) {}
			inline			trace(sigv::node_t* ptr) : node_ptr(ptr) {}
			sigv::node_t*	node_ptr;
		};
		u32				traverse(sigv::node_t*& _node_ptr, bin_t _node_bin, bin_t _tofind, trace* _stack, u32& _stack_depth);

		struct stats
		{
			inline				stats() : numNodes(0), maxNodes(0), numSigs(0), maxSigs(0), numCombs(0) {}
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
		bool				fold;
		bool				verified;

	};


}
#endif	// __XSIGV_SIGMAP_H__
