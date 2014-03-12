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
	namespace xsig
	{	
		struct signature_t
		{
			inline			signature_t() : digest_(0), length_(0) {}
			inline			signature_t(xbyte* d, u32 l) : digest_(d), length_(l) {}
			xbyte*			digest_;
			u32				length_;
		};

		bool	are_signatures_equal (signature_t const& _a, signature_t const& _b);
		bool	are_signatures_not_equal (signature_t const& _a, signature_t const& _b);
		s32		compare_signatures (const signature_t& _a, const signature_t& _b);

		// note: @_out can be one of _left or _right, so the combiner needs to make sure that @_out is written to
		//       after the combined signature is computed.
		typedef void (*combine_f)(signature_t const& _left, signature_t const& _right, signature_t& _out);

		class jallocator
		{
		public:
			virtual void*	allocate(xsize_t _size, u32 _alignment) = 0;
			virtual void	deallocate(void*) = 0;
		};

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
		class map
		{
		public:
									map(jallocator* _allocator);
									~map();

			void					open(signature_t const& _rootsig, combine_f _sigcombiner, u32 _count);
			void					close();

			s32						submit_branch(bin_t _bin, signature_t const * _branch_signatures);
			s32						read_branch(bin_t _bin, signature_t* _branch_signatures, u32 _max_branch_signatures) const;

		private:

			struct imp
			{
				void					init(jallocator* _allocator, combine_f _sigcombiner);
				void					exit();

				void					open(signature_t const& _rootsig, u32 _count);
				void					close();

				s32						submit_branch(bin_t _bin, signature_t const * _branch_signatures);
				s32						read_branch(bin_t _bin, signature_t* _branch_signatures, u32 _max_branch_signatures) const;

				s32						get_signature_at(bin_t, signature_t& _out_signature) const;
				s32						set_signature_at(bin_t, signature_t const & _in_signature);

				jallocator*				allocator_;
				combine_f				combine_f_;

				enum estate { EMPTY = 0, OPEN = 1, BUILDING = 2, FINAL = 3, CLOSED = 4 };
				estate					state_;

				signature_t				root_signature_;
				signature_t				work_signature_;

				bin_t					root_bin_;

				u32						count_signature_;
				u32						submit_signature_;

				xbyte*					signature_data_array_;
			};
			jallocator*				allocator_;
			imp*					imp_;

		public:

			class builder
			{
			public:
										builder(jallocator* _allocator);

				bool					valid() const;

				void					open(signature_t const& _rootsig, combine_f _sigcombiner, u32 _count);
				void					close();

				s32						submit(bin_t _bin, signature_t const& _signature);	// return: 1=added, -1 if this was the last signature of a trusted sub-tree that failed to result in the trusted signature
				
				bool					build();
				bool					build_and_verify(signature_t const& _root_signature);

			protected:
				friend map;
				jallocator*				allocator_;
				imp*					imp_;
			};

									map(builder& _builder);
		};


		inline s32 map::imp::get_signature_at(bin_t _bin, signature_t& _out_signature) const
		{
			// do we contain this bin ?
			if (!root_bin_.contains(_bin))
				return -2;	// out of range

			_out_signature = signature_t(signature_data_array_ + (_bin.value() * root_signature_.length_), root_signature_.length_);
			return 0;
		}

	}

}
#endif	// __XSIGV_SIGMAP_H__
