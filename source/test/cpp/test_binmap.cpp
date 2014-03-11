#include "xbase\x_debug.h"
#include "xbinmaps\binmap.h"
#include "xbinmaps\bin.h"
#include "xbinmaps\utils.h"
#include "xunittest\xunittest.h"

using namespace xcore;


extern xcore::x_iallocator* gTestAllocator;

class my_cell_allocator : public binmap_t::allocator
{
public:
	my_cell_allocator()
	{
	}

	virtual void*			allocate(xsize_t size, u32 align)
	{
		void* p = gTestAllocator->allocate(size, align);
		return p;
	}

	virtual void			deallocate(void* p)
	{
		gTestAllocator->deallocate(p);
	}
};


UNITTEST_SUITE_BEGIN(binmap)
{
	UNITTEST_FIXTURE(main)
	{
		static my_cell_allocator ma;
		static binmap_t::allocator* a = NULL;

		UNITTEST_FIXTURE_SETUP() 
		{
			a = &ma;
		}
		UNITTEST_FIXTURE_TEARDOWN() 
		{
			a = NULL;
		}

		UNITTEST_TEST(SetGet) 
		{
			binmap_t bs;
			bs.init(16, a);
			bin_t b3(1,0), b2(0,1), b4(0,2), b6(1,1), b7(2,0);
			bs.set(b3);
			//bs.dump("set done");
			CHECK_TRUE(bs.is_filled(b3));
			//bs.dump("set checkd");
			CHECK_TRUE(bs.is_filled(b2));
			//bs.dump("get b2 done");
			CHECK_TRUE(bs.is_filled(b3));
			//bs.dump("get b3 done");
			CHECK_TRUE(bs.is_empty(b4));
			CHECK_TRUE(bs.is_empty(b6));
			CHECK_FALSE(bs.is_filled(b7));
			CHECK_FALSE(bs.is_empty(b7));
			CHECK_TRUE(bs.is_filled(b3));
			bs.set(bin_t(1,1));
			CHECK_TRUE(bs.is_filled(bin_t(2,0)));
		}

		UNITTEST_TEST(Clear) 
		{
			binmap_t m;
			m.init(256, a);

			for(int i=0; i<256; i++)
			{
				if (i&1) 
				{
					m.set(bin_t(0,i));
				}
				else 
				{
					m.reset(bin_t(0,i));
				}
			}
			CHECK_FALSE(m.is_empty());
			m.clear();
			CHECK_TRUE(m.is_empty());
		}

		UNITTEST_TEST(Chess) 
		{
			binmap_t chess16;
			chess16.init(32, a);

			for(int i=0; i<16; i++)
			{
				if (i&1) 
				{
					chess16.set(bin_t(0,i));
				}
				else 
				{
					chess16.reset(bin_t(0,i));
				}
			}

			for(int i=0; i<16; i++)
			{
				if (i&1) 
				{
					CHECK_TRUE(chess16.is_filled(bin_t(0,i)));
				} 
				else 
				{
					CHECK_TRUE(chess16.is_empty(bin_t(0,i)));
				}
			}
			CHECK_FALSE(chess16.is_empty(bin_t(4,0)));
			for(int i=0; i<16; i+=2)
				chess16.set(bin_t(0,i));
			CHECK_TRUE(chess16.is_filled(bin_t(4,0)));
			CHECK_TRUE(chess16.is_filled(bin_t(2,3)));

			chess16.set(bin_t(4,1));
			CHECK_TRUE(chess16.is_filled(bin_t(5,0)));
		}

		UNITTEST_TEST(Staircase) 
		{
			const int TOPLAYR = 24;
			binmap_t staircase;
			staircase.init(1 << 25, a);

			for(int i=0;i<TOPLAYR;i++)
				staircase.set(bin_t(i,1));
	
			CHECK_FALSE(staircase.is_filled(bin_t(TOPLAYR,0)));
			CHECK_FALSE(staircase.is_empty(bin_t(TOPLAYR,0)));

			staircase.set(bin_t(0,0));
			CHECK_TRUE(staircase.is_filled(bin_t(TOPLAYR,0)));
		}

		UNITTEST_TEST(Hole)
		{
			binmap_t hole;
			hole.init(bin_t(8,0), a);

			hole.set(bin_t(8,0));
			CHECK_TRUE(hole.is_filled());

			CHECK_TRUE(hole.is_filled(bin_t(6,0)));
			CHECK_TRUE(hole.is_filled(bin_t(6,3)));
			CHECK_TRUE(hole.is_filled(bin_t(8,0)));

			CHECK_FALSE(hole.is_empty(bin_t(6,0)));
			CHECK_FALSE(hole.is_empty(bin_t(6,3)));
			CHECK_FALSE(hole.is_empty(bin_t(8,0)));
			CHECK_FALSE(hole.is_empty(bin_t(6,1)));

			hole.reset(bin_t(6,1));

			CHECK_TRUE (hole.is_filled(bin_t(6,0)));
			CHECK_TRUE (hole.is_filled(bin_t(6,3)));
			CHECK_FALSE(hole.is_filled(bin_t(8,0)));

			CHECK_TRUE (hole.is_empty(bin_t(6,1)));
			CHECK_FALSE(hole.is_empty(bin_t(6,0)));
			CHECK_FALSE(hole.is_empty(bin_t(6,3)));
			CHECK_FALSE(hole.is_empty(bin_t(8,0)));
			CHECK_TRUE (hole.is_empty(bin_t(6,1)));

			hole.reset(bin_t(6,2));

			CHECK_TRUE (hole.is_filled(bin_t(6,0)));
			CHECK_TRUE (hole.is_filled(bin_t(6,3)));
			CHECK_FALSE(hole.is_filled(bin_t(8,0)));

			CHECK_TRUE (hole.is_empty(bin_t(6,2)));
			CHECK_FALSE(hole.is_empty(bin_t(6,0)));
			CHECK_FALSE(hole.is_empty(bin_t(6,3)));
			CHECK_FALSE(hole.is_empty(bin_t(8,0)));
			CHECK_TRUE (hole.is_empty(bin_t(6,1)));


	
		}

		UNITTEST_TEST(Find)
		{
			binmap_t hole;
			hole.init(1 << 5, a);

			hole.set(bin_t(4,0));
			hole.reset(bin_t(1,1));
			hole.reset(bin_t(0,7));

			bin_t f = hole.find_empty().base_left();
			CHECK_EQUAL(bin_t(0,2).value(),f.value());
		}

		UNITTEST_TEST(Alloc) 
		{
			binmap_t b;
			b.init(64, a);

			b.set(bin_t(1,0));
			b.set(bin_t(1,1));
			b.reset(bin_t(1,0));
			b.reset(bin_t(1,1));
		}

		UNITTEST_TEST(FindFiltered) 
		{
			binmap_t data, filter;
			data.init(64, a);
			filter.init(64, a);

			data.set(bin_t(2,0));
			data.set(bin_t(2,2));
			data.set(bin_t(1,7));
			filter.set(bin_t(4,0));
			filter.reset(bin_t(2,1));
			filter.reset(bin_t(1,4));
			filter.reset(bin_t(0,13));
	
			CHECK_TRUE(data.is_filled(bin_t(0, 8)));
			CHECK_TRUE(data.is_filled(bin_t(0, 9)));
			CHECK_TRUE(data.is_filled(bin_t(0,10)));
			CHECK_TRUE(data.is_filled(bin_t(0,11)));
			CHECK_TRUE(data.is_empty (bin_t(0,12)));
			CHECK_TRUE(data.is_empty (bin_t(0,13)));
			CHECK_TRUE(data.is_filled(bin_t(0,14)));
			CHECK_TRUE(data.is_filled(bin_t(0,15)));

			CHECK_TRUE(filter.is_empty (bin_t(0, 8)));
			CHECK_TRUE(filter.is_empty (bin_t(0, 9)));
			CHECK_TRUE(filter.is_filled(bin_t(0,10)));
			CHECK_TRUE(filter.is_filled(bin_t(0,11)));

			// Find first additional bin in @filter
			bin_t x = binmap_t::find_complement(data, filter, bin_t(4,0), 0);
			CHECK_EQUAL(bin_t(0,12).value(),x.value());
		}

		UNITTEST_TEST(Cover)
		{
			binmap_t b;
			b.init(64, a);

			b.set(bin_t(2,0));
			b.set(bin_t(4,1));
			CHECK_EQUAL(bin_t(4,1).value(),b.cover(bin_t(0,30)).value());
			CHECK_EQUAL(bin_t(2,0).value(),b.cover(bin_t(0, 3)).value());
			CHECK_EQUAL(bin_t(2,0).value(),b.cover(bin_t(2, 0)).value());
			//binmap_t c;
			//CHECK_EQUAL(bin64_t::ALL,b.cover(bin64_t(0,30)));
		}

		UNITTEST_TEST(FindFiltered2) 
		{
			const s32 n = 1024;
			binmap_t data, filter;
			data.init(bin_t(0,n), a);
			filter.init(bin_t(0,n), a);

			for(int i=0; i<n; i+=2)
				data.set(bin_t(0,i));
			for(int j=0; j<n; j+=2)
				filter.set(bin_t(0,j));
			data.reset(bin_t(0,500));
			CHECK_EQUAL(bin_t(0,500).value(),binmap_t::find_complement(data, filter, bin_t(10,0), 0).base_left().value());
			data.set(bin_t(0,500));
			CHECK_EQUAL(bin_t::NONE.value(),binmap_t::find_complement(data, filter, bin_t(10,0), 0).base_left().value());
		}
	
		UNITTEST_TEST(CopyRange) 
		{
			binmap_t data, add;
			data.init(64, a);
			add.init(4096, a);

			data.set(bin_t(2,0));
			data.set(bin_t(2,2));
			data.set(bin_t(1,7));
			add.set(bin_t(2,1));
			add.set(bin_t(1,4));
			add.set(bin_t(0,13));
			add.set(bin_t(5,118));
			binmap_t::copy(data, add, bin_t(3,0));
			CHECK_FALSE(data.is_empty(bin_t(3,0)));
			CHECK_FALSE(data.is_filled(bin_t(3,0)));
			CHECK_TRUE(data.is_empty(bin_t(2,0)));
			CHECK_TRUE(data.is_filled(bin_t(2,1)));
			CHECK_TRUE(data.is_empty(bin_t(1,6)));
			CHECK_TRUE(data.is_filled(bin_t(1,7)));
		}

		UNITTEST_TEST(SeqLength) 
		{
			binmap_t b;
			b.init(32, a);

			b.set(bin_t(3,0));
			b.set(bin_t(1,4));
			b.set(bin_t(0,10));
			b.set(bin_t(3,2));
			CHECK_EQUAL(11,b.find_empty().base_offset());
		}

		UNITTEST_TEST(EmptyFilled) 
		{
			// 1112 3312  2111 ....
			binmap_t b;
			b.init(64, a);
	
			CHECK_TRUE(b.is_empty(bin_t::ALL));
	
			b.set(bin_t(1,0));
			b.set(bin_t(0,2));
			b.set(bin_t(0,6));
			b.set(bin_t(1,5));
			b.set(bin_t(0,9));
	
			CHECK_FALSE(b.is_empty(bin_t::ALL));
	
			CHECK_TRUE(b.is_empty(bin_t(2,3)));
			CHECK_FALSE(b.is_filled(bin_t(2,3)));
			//CHECK_TRUE(b.is_solid(bin_t(2,3),binmap_t::MIXED));
			CHECK_TRUE(b.is_filled(bin_t(1,0)));
			CHECK_TRUE(b.is_filled(bin_t(1,5)));
			CHECK_FALSE(b.is_filled(bin_t(1,3)));
	
			b.set(bin_t(0,3));
			b.set(bin_t(0,7));
			b.set(bin_t(0,8));
	
			CHECK_TRUE(b.is_filled(bin_t(2,0)));
			CHECK_TRUE(b.is_filled(bin_t(2,2)));
			CHECK_FALSE(b.is_filled(bin_t(2,1)));

			b.set(bin_t(1,2));
			CHECK_TRUE(b.is_filled(bin_t(2,1)));
		}
	}
}
UNITTEST_SUITE_END
