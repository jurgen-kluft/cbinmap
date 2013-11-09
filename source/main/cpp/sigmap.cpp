#include "xbase\x_target.h"
#include "xbase\x_integer.h"
#include "xbase\x_memory_std.h"
#include "xbinmaps\sigmap.h"

namespace xcore
{
	typedef		sigv::signature_t		xsig_t;
	typedef		sigv::node_t			xnode_t;
	typedef		sigv::iallocator		xallocator_t;
	typedef		sigv::sigcomb_f			xsigcomb_f;

#ifdef TARGET_32BIT
	typedef		u64						iptr;
#else
	typedef		u32						iptr;
#endif

	//
	// Verify if the signature branch results in the root signature.
	//
	// The signature branch is saved by writing first the mirror child 
	// of the node and then the mirror children of the parents when 
	// navigating up.
	//
	bool xsigmap::submit_branch(bin_t _bin, xsig_t* const* _branch_signatures)
	{
		s32 i = 0;

		// First step: Determine if this branch resolves to the root signature
		xsig_t const* lhs = _branch_signatures[i++];
		xsig_t const* rhs = _branch_signatures[i++];
		ASSERT(rootSig.length == lhs->length);
		ASSERT(rootSig.length == rhs->length);

		while (_bin != rootBin)
		{
			if (_bin.is_left())
				combiner(*rhs, *lhs, &workSig);
			else
				combiner(*lhs, *rhs, &workSig);

			lhs = _branch_signatures[i++];
			ASSERT(rootSig.length == lhs->length);
			rhs = &workSig;
			_bin.to_parent();
		}

		for (u32 i=0; i<rootSig.length; ++i)
		{
			if (rootSig.digest[i] != workSig.digest[i])
				return false;
		}

		// Ok, the branch is validated, now insert it into our tree
		// @TODO

		return true;
	}

	namespace sigv
	{
		// sizeof(node_t) == 8 | 16
		struct node_t
		{
			void			clear()								{ pchild[0]=0; pchild[1]=0; }

			enum echild {LEFT=0, RIGHT=1};
			bool			has_child(echild _child) const		{ return pchild[_child] != 0; }
			bool			is_node(echild _child) const		{ return ((iptr)(pchild[1<<_child])&1) == 0; }
			void			set_node(echild _child, node_t* n)	{ ASSERT(n!=NULL); (pchild[1<<_child]) = n; }
			node_t*			get_node(echild _child) const		{ return pchild[1<<_child]; }
			void			set_sig(echild _child, xbyte* s)	{ ASSERT(s!=NULL); psig[1<<_child] = (xbyte*)((iptr)s | 1); }
			xbyte*			get_sig(echild _child) const		{ return (xbyte*)((iptr)(psig[1<<_child]) & 1); }

		private:
			union
			{
				node_t*		pchild[2];
				xbyte*			psig[2];
			};
		};
	}

	static inline void sCopyHash(xsig_t& _dst, xsig_t const& _src)
	{
		ASSERT(_dst.length == _src.length);
		ASSERT(_src.digest != NULL);
		ASSERT(_dst.digest != NULL);
		x_memcopy4((u32*)_dst.digest, (u32*)_src.digest, _dst.length);
	}

	static inline void sDuplicateHash(xallocator_t* _allocator, xsig_t const& _src_sig, xsig_t& _dst_sig)
	{
		_dst_sig = _allocator->sig_allocate();
		sCopyHash(_dst_sig, _src_sig);
	}

	static inline void	sAllocNode(xallocator_t* _allocator, xnode_t* _node)
	{
		_node = _allocator->node_allocate();
		_node->clear();
	}

	static inline bool operator == (xsig_t const& _a, xsig_t const& _b)
	{
		ASSERT(_a.length == _b.length);
		ASSERT(_a.digest != NULL);
		ASSERT(_b.digest != NULL);

		// This is a special case where we assume the signature is aligned on 4 bytes
		u32 const* lhs = (u32 const*)_a.digest;
		u32 const* rhs = (u32 const*)_b.digest;
		for (u32 i=0; i<_a.length; i+=4)
		{
			if (*lhs++ != *rhs++)
				return false;
		}
		return true;
	}


	xsigmap::xsigmap(xallocator_t* _allocator, xsigcomb_f _sig_combiner)
		: allocator(_allocator)
		, combiner(_sig_combiner)
		, rootNode(0)
	{
	}

	xsigmap::~xsigmap()
	{
		close();
	}

	void			xsigmap::open(xsig_t const& _rootsig, u32 _max_bins, bool _fold)
	{
		close();

		fold = _fold;
		verified = false;
		rootBin = bin_t(xcore::x_intu::ilog2(_max_bins), 0);

		ASSERT(allocator != NULL);
		ASSERT(combiner != NULL);

		allocator->initialize(sizeof(xnode_t), _rootsig.length);
		workSig = allocator->sig_allocate();

		sDuplicateHash(allocator, _rootsig, rootSig);
		sAllocNode(allocator, (xnode_t*)rootNode);
	}

	void			xsigmap::close()
	{
		// Traverse the tree and deallocate nodes, leafs and hashes
	}

	bool			xsigmap::verify()
	{
		return verified;
	}

	s32				xsigmap::submit(bin_t _bin, xsig_t const& _sig)
	{
		// do we contain this bin ?
		if (rootBin.contains(_bin))
			return -2;	// out of range

		// first check if we are dealing with a base bin, if so
		// we find it and attach the signature to it as 'trusted'.
		const u32 max_stack_depth = 64;
		u32 stack_depth = max_stack_depth;
		trace stack[max_stack_depth];

		xnode_t* iter = rootNode;
		traverse(iter, rootBin, _bin, stack, stack_depth);
		
		// content signatures are always layer 0 signatures
		// here we also know that there are actually 3 layers below
		// layer 0 since we pack 8 signatures into the last leaf

		return 0;
	}

	s32				xsigmap::find_unsigned(bin_t& _bin) const
	{

		return 0;
	}

	s32				xsigmap::find_unsigned(bin_t* _bins, u32 _count) const
	{

		return 0;
	}


	u32				xsigmap::traverse(xnode_t*& _node, bin_t _node_bin, bin_t _tofind, trace* _stack, u32& _stack_depth)
	{
		s32 depth = _stack_depth;
		while (_node_bin != _tofind)
		{
			ASSERT(_stack_depth > 0);
			_stack[--_stack_depth] = trace(_node);

			xnode_t::echild _echild = xnode_t::LEFT;
			if (_node_bin < _tofind)
			{
				_node_bin.left();
				_echild = xnode_t::LEFT;
			}
			else if (_node_bin > _tofind)
			{
				_node_bin.right();
				_echild = xnode_t::RIGHT;
			}

			// It can be that the child is null, if so we have to create one and assign it.
			if (_node->has_child(_echild))
			{
				if (_node->is_node(_echild))
				{
					_node = _node->get_node(_echild);
				}
				else
				{
					// This child is a signature, meaning a folded subtree
					return 0;
				}
			}
			else
			{
				xnode_t* _node_child_ptr = allocator->node_allocate();
				_node_child_ptr->clear();
				_node->set_node(_echild, _node_child_ptr);
				_node = _node_child_ptr;
			}
		}
		
		ASSERT(_stack_depth > 0);
		_stack[--_stack_depth] = trace(_node);

		return _stack_depth - depth;
	}


}
