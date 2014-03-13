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
	namespace sigmap
	{	
		typedef		xbyte		byte;

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

		class iallocator
		{
		public:
			virtual void*	allocate(xsize_t _size, u32 _alignment) = 0;
			virtual void	deallocate(void*) = 0;
		};

		/**
		 * @group		xcore::sigmap
		 * @brief		Signature Map implementation used for content validation
		 * @URL         Reference -> http://www.merkle.com/‎
		 * 
		 * @required    @_rootsig is the root signature
		 *				@_max_bins is the number of bins
		 *				@_sigcombiner can combine 2 signatures into 1
		 *				@_allocator allocator for allocating nodes and signatures
		 *
		 * @behavior	The signature map will allocate once for the full tree 
		 * 
		 * @example		You want to set a signature on it's bin :
		 *              
		 *                sigmap->submit(bin_t(sig_bin), sig);
		 *              
		*/

		class branch
		{
		public:
			inline					branch(signature_t* _array, u32 _length) : length_(_length), size_(0), array_(_array) {}

			inline u32				length() const					{ return length_; }
			inline u32				size() const					{ return size_; }

			s32						push(signature_t const&);		// return 1 if OK, 0 if not pushed
			signature_t const*		operator[](s32 index) const;

		protected:
			u32						length_;
			u32						size_;
			signature_t				*array_;
		};

		class data
		{
		public:
			bin_t					get_root() const				{ return root_; }
			u32						get_siglen() const				{ return siglen_; }
			byte*					get_data() const				{ return data_; }

			static u32				size_for(bin_t _root, u32 _siglen);

		protected:
			inline					data() : root_(bin_t::NONE), siglen_(0), data_(0) {}
			inline					data(bin_t _root, u32 _siglen, xbyte* _data) : root_(_root), siglen_(_siglen), data_(_data) {}

			bin_t					root_;
			u32						siglen_;
			byte*					data_;
		};

		class user_data : public data
		{
		public:
			inline					user_data(bin_t _root, u32 _signature_length, xbyte* _data) : data(_root, _signature_length, _data) {}
		};

		class cmap
		{
		public:
									cmap();
									cmap(data& _data);

			s32						read_branch(bin_t _bin, branch& _out_branch) const;
			s32						get_sig_at(bin_t, signature_t& _out_signature) const;

		protected:
			bin_t*					root_bin_;
			signature_t				root_sig_;
			signature_t				work_sig_;

			xbyte*					data_;
		};

		class map : public cmap
		{
		public:
									map();
									map(data& _data, signature_t const& _rootsig, combine_f _sigcombiner);

			s32						submit_branch(bin_t _bin, branch const& _branch);

		protected:
			s32						set_sig_at(bin_t, signature_t const& _in_signature);

			combine_f				combine_f_;

			u32						total_count_sig_;
			u32						added_count_sig_;

		public:

			class builder
			{
			public:
										builder(data& _data, signature_t const& _rootsig, combine_f _sigcombiner);

				bool					valid() const;

				s32						submit(bin_t _bin, signature_t const& _signature);	// return: 1=added, -1 if this was the last signature of a trusted sub-tree that failed to result in the trusted signature
				
				bool					build();
				bool					build_and_verify(signature_t const& _root_signature);

			protected:
				s32						get_sig_at(bin_t _bin, signature_t& _out_signature) const;

				combine_f				combine_f_;

				u32						total_count_sig_;
				u32						added_count_sig_;

				bin_t*					root_bin_;
				signature_t				root_sig_;
				signature_t				work_sig_;

				xbyte*					data_;
			};
		};


		inline s32 cmap::get_sig_at(bin_t _bin, signature_t& _out_signature) const
		{
			// do we contain this bin ?
			if (!root_bin_->contains(_bin))
				return -2;	// out of range

			_out_signature = signature_t(data_ + (_bin.value() * root_sig_.length_), root_sig_.length_);
			return 0;
		}

		inline s32 map::set_sig_at(bin_t _bin, signature_t const& _in_signature)
		{
			// do we contain this bin ?
			if (!root_bin_->contains(_bin))
				return -2;	// out of range

			u32 const signature_offset = (u32)_bin.value() * root_sig_.length_;
			signature_t s = signature_t(data_ + signature_offset, root_sig_.length_);
			u32 const* src = (u32 const*)_in_signature.digest_;
			u32      * dst = (u32      *)s.digest_;
			for (s32 i=root_sig_.length_; i>=0; i-=4)
				*dst++ = *src++;
			return 0;
		}

		inline s32 map::builder::get_sig_at(bin_t _bin, signature_t& _out_signature) const
		{
			// do we contain this bin ?
			if (!root_bin_->contains(_bin))
				return -2;	// out of range

			_out_signature = signature_t(data_ + (_bin.value() * root_sig_.length_), root_sig_.length_);
			return 0;
		}

	}

}
#endif	// __XSIGV_SIGMAP_H__
