#include "xbase\x_allocator.h"
#include "xbinmaps\sigmap.h"
#include "xunittest\xunittest.h"

using namespace xcore;


extern xcore::x_iallocator* gTestAllocator;

class my_sigv_allocator : public sigv::iallocator
{
	u32		sizeof_node;
	u32		sizeof_sig;
public:
	virtual void			initialize(u32 _sizeof_node, u32 _sizeof_sig)
	{
		sizeof_node = _sizeof_node;
		sizeof_sig  = _sizeof_sig;
	}

	virtual sigv::node_t*	node_allocate()
	{
		sigv::node_t* n = (sigv::node_t*)gTestAllocator->allocate(sizeof_node, 4);
		return n;
	}

	virtual void			node_deallocate(sigv::node_t* p)
	{
		gTestAllocator->deallocate(p);
	}

	virtual sigv::signature_t	sig_allocate()
	{
		sigv::signature_t s;
		s.length = sizeof_sig;
		s.digest = (xbyte*)gTestAllocator->allocate(s.length, 4);
		return s;
	}

	virtual void			sig_deallocate(sigv::signature_t& s)
	{
		gTestAllocator->deallocate(s.digest);
		s.digest = 0;
	}
};

static void	sSigCombiner(sigv::signature_t const& lhs, sigv::signature_t const& rhs, sigv::signature_t* result)
{

};

UNITTEST_SUITE_BEGIN(sigmap)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(construct)
		{
			my_sigv_allocator a;
			xsigmap sm(&a, sSigCombiner);
		}

		UNITTEST_TEST(open_close)
		{
			my_sigv_allocator a;
			xsigmap sm(&a, sSigCombiner);

			sigv::signature_t rootSignature = a.sig_allocate();
			sm.open(rootSignature, 512, true);
			sm.close();
			a.sig_deallocate(rootSignature);
		}

		UNITTEST_TEST(open_submit_close)
		{
			my_sigv_allocator a;
			xsigmap sm(&a, sSigCombiner);

			sigv::signature_t rootSignature = a.sig_allocate();
			sm.open(rootSignature, 512, true);

			// @TODO: submit a signature

			sm.close();
			a.sig_deallocate(rootSignature);
		}
	}
}
UNITTEST_SUITE_END
