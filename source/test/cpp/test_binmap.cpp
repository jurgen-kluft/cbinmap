#include "xbase/x_debug.h"
#include "xbase/x_memory_std.h"
#include "xbinmaps/binmap.h"
#include "xbinmaps/bin.h"
#include "xbinmaps/utils.h"
#include "xunittest/xunittest.h"

using namespace xcore;

extern xcore::x_iallocator* gTestAllocator;


UNITTEST_SUITE_BEGIN(binmap)
{
	UNITTEST_FIXTURE(main)
	{
		xbyte* data1 = NULL;
		xbyte* data2 = NULL;
		u32 data_size = 0;

		UNITTEST_FIXTURE_SETUP() 
		{
			data_size = binmaps::data::size_for(bin_t::to_root(1<<24));
			data1 = (xbyte*)gTestAllocator->allocate(data_size, sizeof(void*));
			data2 = (xbyte*)gTestAllocator->allocate(data_size, sizeof(void*));
		}
		UNITTEST_FIXTURE_TEARDOWN() 
		{
			gTestAllocator->deallocate(data1);
			gTestAllocator->deallocate(data2);
		}

		void clear_data()
		{
			x_memzero(data1, data_size);
			x_memzero(data2, data_size);
		}

		UNITTEST_TEST(SetGet) 
		{
			clear_data();

			u32 const n = 16;
			binmaps::binmap bs(binmaps::user_data(bin_t::to_root(n), data1));

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
			clear_data();

			u32 const n = 16;
			binmaps::binmap bs(binmaps::user_data(bin_t::to_root(n), data1));

			for(int i=0; i<256; i++)
			{
				if (i&1) 
				{
					bs.set(bin_t(0,i));
				}
				else 
				{
					bs.reset(bin_t(0,i));
				}
			}
			CHECK_FALSE(bs.is_empty());
			bs.clear();
			CHECK_TRUE(bs.is_empty());
		}

		UNITTEST_TEST(Chess) 
		{
			clear_data();

			u32 const n = 32;
			binmaps::binmap chess16(binmaps::user_data(bin_t::to_root(n), data1));

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
			clear_data();

			const int TOPLAYR = 24;
			u32 const n = 1 << TOPLAYR;
			binmaps::binmap staircase(binmaps::user_data(bin_t::to_root(n), data1));

			for(int i=0;i<TOPLAYR;i++)
				staircase.set(bin_t(i,1));
	
			CHECK_FALSE(staircase.is_filled(bin_t(TOPLAYR,0)));
			CHECK_FALSE(staircase.is_empty(bin_t(TOPLAYR,0)));

			staircase.set(bin_t(0,0));
			CHECK_TRUE(staircase.is_filled(bin_t(TOPLAYR,0)));
		}

		UNITTEST_TEST(Hole)
		{
			clear_data();

			u32 const n = 1 << 8;
			binmaps::binmap hole(binmaps::user_data(bin_t::to_root(n), data1));

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
			clear_data();

			u32 const n = 1 << 5;
			binmaps::binmap hole(binmaps::user_data(bin_t::to_root(n), data1));

			hole.set(bin_t(4,0));
			hole.reset(bin_t(1,1));
			hole.reset(bin_t(0,7));

			bin_t f = hole.find_empty().base_left();
			CHECK_EQUAL(bin_t(0,2).value(),f.value());
		}

		UNITTEST_TEST(Alloc) 
		{
			clear_data();

			u32 const n = 64;
			binmaps::binmap b(binmaps::user_data(bin_t::to_root(n), data1));

			b.set(bin_t(1,0));
			b.set(bin_t(1,1));
			b.reset(bin_t(1,0));
			b.reset(bin_t(1,1));
		}

		UNITTEST_TEST(FindFiltered) 
		{
			clear_data();

			u32 const n = 64;
			binmaps::binmap data(binmaps::user_data(bin_t::to_root(n), data1));
			binmaps::binmap filter(binmaps::user_data(bin_t::to_root(n), data2));

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
			bin_t x = binmaps::find_complement(data, filter, bin_t(4,0), 0);
			CHECK_EQUAL(bin_t(0,12).value(),x.value());
		}

		UNITTEST_TEST(Cover)
		{
			clear_data();

			u32 const n = 64;
			binmaps::binmap b(binmaps::user_data(bin_t::to_root(n), data1));

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
			clear_data();

			u32 const n = 1024;
			binmaps::binmap data(binmaps::user_data(bin_t::to_root(n), data1));
			binmaps::binmap filter(binmaps::user_data(bin_t::to_root(n), data2));

			for(int i=0; i<n; i+=2)
				data.set(bin_t(0,i));
			for(int j=0; j<n; j+=2)
				filter.set(bin_t(0,j));
			data.reset(bin_t(0,500));
			CHECK_EQUAL(bin_t(0,500).value(),binmaps::find_complement(data, filter, bin_t(10,0), 0).base_left().value());
			data.set(bin_t(0,500));
			CHECK_EQUAL(bin_t::NONE.value(),binmaps::find_complement(data, filter, bin_t(10,0), 0).base_left().value());
		}
	
		UNITTEST_TEST(CopyRange) 
		{
			clear_data();

			binmaps::binmap data(binmaps::user_data(bin_t::to_root(64), data1));
			binmaps::binmap add(binmaps::user_data(bin_t::to_root(4096), data2));

			data.set(bin_t(2,0));
			data.set(bin_t(2,2));
			data.set(bin_t(1,7));
			add.set(bin_t(2,1));
			add.set(bin_t(1,4));
			add.set(bin_t(0,13));
			add.set(bin_t(5,118));
			binmaps::copy(data, add, bin_t(3,0));
			CHECK_FALSE(data.is_empty(bin_t(3,0)));
			CHECK_FALSE(data.is_filled(bin_t(3,0)));
			CHECK_TRUE(data.is_empty(bin_t(2,0)));
			CHECK_TRUE(data.is_filled(bin_t(2,1)));
			CHECK_TRUE(data.is_empty(bin_t(1,6)));
			CHECK_TRUE(data.is_filled(bin_t(1,7)));
		}

		UNITTEST_TEST(SeqLength) 
		{
			clear_data();

			u32 const n = 32;
			binmaps::binmap b(binmaps::user_data(bin_t::to_root(n), data1));

			b.set(bin_t(3,0));
			b.set(bin_t(1,4));
			b.set(bin_t(0,10));
			b.set(bin_t(3,2));
			CHECK_EQUAL(11,b.find_empty().base_offset());
		}

		UNITTEST_TEST(EmptyFilled) 
		{
			clear_data();

			// 1112 3312  2111 ....
			u32 const n = 64;
			binmaps::binmap b(binmaps::user_data(bin_t::to_root(n), data1));
	
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
