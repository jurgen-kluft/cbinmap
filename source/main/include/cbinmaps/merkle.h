// x_sigmap.h - Core Signature Map
#ifndef __CSIGV_SIGMAP_H__
#define __CSIGV_SIGMAP_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "cbinmaps/bin.h"

namespace ncore
{
	namespace merkle
	{	
		typedef		u8		byte;

		struct hash_t
		{
			inline			hash_t() : digest_(0), length_(0) {}
			inline			hash_t(u8* d, u32 l) : digest_(d), length_(l) {}
			u8*			digest_;
			u32				length_;
		};

		bool	are_equal  (hash_t const& _a, hash_t const& _b);
		bool	are_nequal (hash_t const& _a, hash_t const& _b);
		s32		compare    (const hash_t& _a, const hash_t& _b);

		// note: @_out can be one of _left or _right, so the combiner needs to make sure that @_out is written to
		//       after the combined signature is computed.
		typedef void (*combine_f)(hash_t const& _left, hash_t const& _right, hash_t& _out);

		/**
		 * @group		ncore::merkle
		 * @brief		Merkle-Tree implementation used for content validation
		 * @URL         Reference -> http://www.merkle.com/‎
		 * 
		 * @required    @_rootsig is the root signature
		 *				@_max_bins is the number of bins
		 *				@_sigcombiner can combine 2 signatures into 1
		 *				@_allocator allocator for allocating nodes and signatures
		 *
		 * @behavior	The merkle tree will allocate once for the full tree 
		 * 
		 * @example		You want to set a signature on it's bin :
		 *              
		 *                sigmap->submit(bin_t(sig_bin), sig);
		 *              
		*/

		struct branch_t
		{
		public:
			inline					branch_t(hash_t* _array, u32 _length) : length_(_length), size_(0), array_(_array) {}

			inline u32				length() const					{ return length_; }
			inline u32				size() const					{ return size_; }

			s32						push(hash_t const&);		// return 1 if OK, 0 if not pushed
			hash_t const*			operator[](s32 index) const;

		protected:
			u32						length_;
			u32						size_;
			hash_t					*array_;
		};

		struct data_t
		{
		public:
			inline					data_t(bin_t _root, u32 _siglen, u8* _data) : root_(_root), siglen_(_siglen), data_(_data) {}

			bin_t					get_root() const				{ return root_; }
			u32						get_siglen() const				{ return siglen_; }
			byte*					get_data() const				{ return data_; }

			static u32				size_for(bin_t _root, u32 _siglen);

		protected:
			inline					data_t() : root_(bin_t::NONE), siglen_(0), data_(0) {}

			bin_t					root_;
			u32						siglen_;
			byte*					data_;
		};

		class ctree
		{
		public:
									ctree();
									ctree(data_t& _data);

			s32						read(bin_t _bin, branch_t& _out_branch) const;
			s32						read(bin_t _bin, hash_t& _out_signature) const;

		protected:
			bin_t*					root_bin_;
			hash_t					root_sig_;
			hash_t					work_sig_;

			u8*					data_;
		};



		class tree : public ctree
		{
		public:
									tree();
									tree(data_t& _data, hash_t const& _rootsig, combine_f _sigcombiner);

			s32						write(bin_t _bin, branch_t const& _branch);

		protected:
			s32						write(bin_t _bin, hash_t const& _in_signature);

			combine_f				combine_f_;

			u32						total_count_sig_;
			u32						added_count_sig_;

		public:

			class builder
			{
			public:
										builder(data_t& _data, hash_t const& _rootsig, combine_f _sigcombiner);

				bool					valid() const;

				s32						write(bin_t _bin, hash_t const& _signature);	// return: 1=added, -1 if this was the last signature of a trusted sub-tree that failed to result in the trusted signature
				
				bool					build();
				bool					build_and_verify(hash_t const& _root_signature);

			protected:
				s32						read(bin_t _bin, hash_t& _out_signature) const;

				combine_f				combine_f_;

				u32						total_count_sig_;
				u32						added_count_sig_;

				bin_t*					root_bin_;
				hash_t					root_sig_;
				hash_t					work_sig_;

				u8*					data_;
			};
		};

		inline s32 ctree::read(bin_t _bin, hash_t& _out_signature) const
		{
			// do we contain this bin ?
			if (!root_bin_->contains(_bin))
				return -2;	// out of range

			_out_signature = hash_t(data_ + (_bin.value() * root_sig_.length_), root_sig_.length_);
			return 0;
		}

		inline s32 tree::write(bin_t _bin, hash_t const& _in_signature)
		{
			// do we contain this bin ?
			if (!root_bin_->contains(_bin))
				return -2;	// out of range

			u32 const signature_offset = (u32)_bin.value() * root_sig_.length_;
			hash_t s = hash_t(data_ + signature_offset, root_sig_.length_);
			u32 const* src = (u32 const*)_in_signature.digest_;
			u32      * dst = (u32      *)s.digest_;
			for (s32 i=root_sig_.length_; i>=0; i-=4)
				*dst++ = *src++;
			return 0;
		}

		inline s32 tree::builder::read(bin_t _bin, hash_t& _out_signature) const
		{
			// do we contain this bin ?
			if (!root_bin_->contains(_bin))
				return -2;	// out of range

			_out_signature = hash_t(data_ + (_bin.value() * root_sig_.length_), root_sig_.length_);
			return 0;
		}

	}

}
#endif	// __CSIGV_SIGMAP_H__
