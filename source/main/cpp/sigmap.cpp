#include "xbase\x_target.h"
#include "xbase\x_integer.h"
#include "xbase\x_memory_std.h"
#include "xbinmaps\sigmap.h"

namespace xcore
{
	typedef		  sigmap::signature_t		xsig_t;
	typedef		  sigmap::iallocator		xallocator_t;
	typedef		  sigmap::sigcomb_f			xsigcomb_f;

	static inline void copy(xsig_t& _dst, xsig_t const& _src)
	{
		ASSERT(_dst.length == _src.length);
		ASSERT(_src.digest != NULL);
		ASSERT(_dst.digest != NULL);
		x_memcopy4((u32*)_dst.digest, (u32*)_src.digest, _dst.length/4);
	}

	static inline xsig_t dup(xallocator_t* _allocator, xsig_t const& _src_sig)
	{
		xsig_t dst_sig = _allocator->sig_allocate();
		copy(dst_sig, _src_sig);
		return dst_sig;
	}

	static inline void	allocate(xallocator_t* _allocator, sigmap::node_t*& _node)
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
		, is_open(false)
		, do_fold(false)
		, is_verified(false)
		, is_valid(false)
	{
	}

	xsigmap::~xsigmap()
	{
		close();
	}

	void			xsigmap::open(xsig_t const& _rootsig, u32 _max_bins, bool _fold)
	{
		close();

		is_open = true;
		do_fold = _fold;
		is_verified = false;
		is_valid = false;
		u32 max_bins2 = xcore::x_intu::ceilPower2(_max_bins);
		u32 layer = xcore::x_intu::ilog2(max_bins2);
		rootBin = bin_t(layer, 0);

		ASSERT(allocator != NULL);
		ASSERT(combiner != NULL);

		// compute the maximum number of nodes and hashes that this sigmap will need
		u32 m = ((_max_bins + 1) / 2) * 2 * 2;
		u32 max_required_nodes = m;
		while (m != 0)
		{

		};

		allocator->initialize(max_required_nodes, sizeof(sigmap::node_t), _max_bins, _rootsig.length);
		workSig = allocator->sig_allocate();
		statistics.incSig();

		rootSig = dup(allocator, _rootsig);
		statistics.incSig();

		allocate(allocator, rootNode);
		statistics.incNode();
	}

	void			xsigmap::close()
	{
		// Traverse the tree and deallocate nodes, leafs and hashes
		if (is_open)
		{
			const u32 max_stack_depth = 32;
			u32 stack_depth = max_stack_depth;
			sigmap::node_t* stack[max_stack_depth];

			// traverse to the far left to start the removal of nodes
			u32 r = traverse(rootBin, rootNode, bin_t(0,0), stack, stack_depth, ETRAVERSE_NORMAL);
			while (stack_depth < max_stack_depth)
			{
				ASSERT(stack_depth < max_stack_depth);
				sigmap::node_t* node = stack[stack_depth++];

				// traverse right and then as far left as possible
				if (node->is_node(sigmap::node_t::RIGHT))
				{
					if (node->has_node(sigmap::node_t::RIGHT))
					{
						sigmap::node_t* child = node->get_node(sigmap::node_t::RIGHT);
						ASSERT(stack_depth > 0);
						stack[--stack_depth] = child;
						while (child->has_node(sigmap::node_t::LEFT))
						{
							child = child->get_node(sigmap::node_t::LEFT);
							ASSERT(stack_depth > 0);
							stack[--stack_depth] = child;
						}
					}
				}

				// deallocate the signatures the node has for left and/or right
				sigmap::node_t::echild children[] = { sigmap::node_t::LEFT, sigmap::node_t::RIGHT };
				for (u32 i=0; i<2; ++i)
				{
					sigmap::node_t::echild c = children[i];
					if (node->has_sig(c))
					{
						xsig_t s(node->get_sig(c), rootSig.length);
						allocator->sig_deallocate(s);
						statistics.decSig();
					}
				}

				// remove node
				allocator->node_deallocate(node);
				statistics.decNode();
			}

			rootNode = NULL;

			statistics.decSig();
			allocator->sig_deallocate(rootSig);
			statistics.decSig();
			allocator->sig_deallocate(workSig);

			is_open = false;
		}
	}

	bool			xsigmap::verify()
	{
		if (is_open && !is_verified)
		{
			if (rootNode->has_sig(sigmap::node_t::LEFT) && rootNode->has_sig(sigmap::node_t::RIGHT))
			{
				is_verified = true;
				xsig_t lhs(rootNode->get_sig(sigmap::node_t::LEFT), rootSig.length);
				xsig_t rhs(rootNode->get_sig(sigmap::node_t::RIGHT), rootSig.length);
				combiner(lhs, rhs, &workSig);
				statistics.incCombs();

				is_valid = rootSig == workSig;

				close();
			}
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

		// first check if we are dealing with a base bin, if so
		// we find it and attach the signature to it as 'trusted'.
		const u32 max_stack_depth = 32;
		u32 stack_depth = max_stack_depth;
		sigmap::node_t* stack[max_stack_depth];

		u32 r = traverse(rootBin, rootNode, _bin, stack, stack_depth, ETRAVERSE_EXPAND);
		if (r > 0)
		{
			ASSERT(stack_depth < max_stack_depth);
			sigmap::node_t* node = stack[stack_depth++];
			sigmap::node_t::echild const echild = (_bin.is_left()) ? sigmap::node_t::LEFT : sigmap::node_t::RIGHT;
			if (!node->has_sig(echild))
			{
				xsig_t child_sig = dup(allocator, _sig);
				statistics.incSig();
				node->set_sig(echild, child_sig.digest);
				{
					// for every node:
					// - check if node has 2 signatures
					// - combine the 2 signatures
					// - determine if we are the left or right child of our parent
					// - move up to our parent and set the signature on the appropriate child
					while (_bin.parent() != rootBin)
					{
						// See if this node has 2 signatures, if so combine them, remove the node and
						// set the combined signature on the parent
						if (!node->has_sig(sigmap::node_t::LEFT) || !node->has_sig(sigmap::node_t::RIGHT))
							break;

						xsig_t lhs(node->get_sig(sigmap::node_t::LEFT), rootSig.length);
						xsig_t rhs(node->get_sig(sigmap::node_t::RIGHT), rootSig.length);
						combiner(lhs, rhs, &lhs);
						statistics.incCombs();

						// remove the node and one of the signatures, keep the other one for the parent
						allocator->node_deallocate(node);
						statistics.decNode();
						allocator->sig_deallocate(rhs);
						statistics.decSig();

						ASSERT(stack_depth < max_stack_depth);
						sigmap::node_t* parent = stack[stack_depth++];
						_bin.to_parent();

						// set the signature on the child of the parent that was occupied by child node
						sigmap::node_t::echild const c = (_bin.is_left()) ? sigmap::node_t::LEFT : sigmap::node_t::RIGHT;
						ASSERT(parent->get_node(c) == node);
						parent->set_sig(c, lhs.digest);

						node = parent;
					};
				}
				return 0;
			}
		}
		return -1;
	}

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



	u32				xsigmap::traverse(bin_t _node_bin, sigmap::node_t* _node, bin_t _tofind, sigmap::node_t** _stack, u32& _stack_depth, emode _traverse_mode)
	{
		s32 const depth = _stack_depth;

		ASSERT(_stack_depth > 0);
		_stack[--_stack_depth] = (_node);
		while (_node_bin != _tofind)
		{
			sigmap::node_t::echild _echild = sigmap::node_t::LEFT;
			if (_tofind < _node_bin)
			{
				_node_bin.to_left();
				_echild = sigmap::node_t::LEFT;
			}
			else if (_tofind > _node_bin)
			{
				_node_bin.to_right();
				_echild = sigmap::node_t::RIGHT;
			}

			if (_node_bin.layer() == 0)
				break;

			if (_node->has_node(_echild))
			{
				_node = _node->get_node(_echild);
			}
			else if (_node->has_sig(_echild))
			{
				// This child is a signature, meaning a folded subtree
				return 0;
			}
			else
			{
				if (_traverse_mode == ETRAVERSE_EXPAND)
				{
					sigmap::node_t* _node_child_ptr = allocator->node_allocate();
					statistics.incNode();
					_node_child_ptr->clear();
					_node->set_node(_echild, _node_child_ptr);
					_node = _node_child_ptr;
				}
				else
				{
					break;
				}
			}

			ASSERT(_stack_depth > 0);
			_stack[--_stack_depth] = (_node);
		}
		
		return depth - _stack_depth;
	}


}
