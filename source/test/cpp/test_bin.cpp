#include "xbinmaps\bin.h"
#include "xbinmaps\utils.h"
#include "xunittest\xunittest.h"

using namespace xcore;

UNITTEST_SUITE_BEGIN(bin)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(_search)
		{
			bin_t b;
			CHECK_TRUE(b.is_none());

			bin_t v1(0x3f);
			int v1l = v1.layer();
			int v1o = (int)v1.layer_offset();

			u32 a = 1;
			a = 0 - a;

			bin_t v2(5, 0);
			CHECK_EQUAL(0x1F, v2.value());
		}

		UNITTEST_TEST(InitGet) 
		{
			CHECK_EQUAL(0x1,bin_t(1,0).value());
			CHECK_EQUAL(0xB,bin_t(2,1).value());
			CHECK_EQUAL(0x2,bin_t(2,1).layer());
			CHECK_EQUAL(34,bin_t(34,2345).layer());
			CHECK_EQUAL(0x7ffffffffULL,bin_t(34,2345).layer_bits());
			CHECK_EQUAL(1,bin_t(2,1).layer_offset());
			CHECK_EQUAL(2345,bin_t(34,2345).layer_offset());
			CHECK_EQUAL((1<<1) - 1,bin_t(0,123).layer_bits());
			CHECK_EQUAL((1<<17) - 1,bin_t(16,123).layer_bits());
		}

		UNITTEST_TEST(Base1) 
		{
			bin_t b(2,1);
			s32 o1 = (s32)b.base_offset();
			b.to_left();
			b.to_left();
			CHECK_EQUAL(0, b.layer());
			CHECK_EQUAL(o1, b.layer_offset());
		}

		UNITTEST_TEST(Base2) 
		{
			bin_t b(5,9);
			s32 o1 = (s32)b.base_offset();
			b.to_left(); // 5->4
			b.to_left(); // 4->3
			b.to_left(); // 3->2
			b.to_left(); // 2->1
			b.to_left(); // 1->0
			CHECK_EQUAL(0, b.layer());
			CHECK_EQUAL(o1, b.layer_offset());
		}

		UNITTEST_TEST(Navigation)
		{
			bin_t mid(4,18);
			CHECK_EQUAL(bin_t(5,9).value(),mid.parent().value());
			CHECK_EQUAL(bin_t(3,36).value(),mid.left().value());
			CHECK_EQUAL(bin_t(3,37).value(),mid.right().value());
			CHECK_EQUAL(bin_t(5,9).value(),bin_t(4,19).parent().value());
			bin_t up32(30,1);
			CHECK_EQUAL(bin_t(31,0).value(),up32.parent().value());
		}

		UNITTEST_TEST(Overflows)
		{
			CHECK_FALSE(bin_t::NONE.contains(bin_t(0,1)));
			CHECK_TRUE(bin_t::ALL.contains(bin_t(0,1)));
			CHECK_EQUAL(0,bin_t::NONE.base_length());
			CHECK_EQUAL(bin_t::NONE.value(),bin_t::NONE.twisted(123).value());
		}

		UNITTEST_TEST(Advanced) 
		{
			CHECK_EQUAL(4,bin_t(2,3).base_length());
			CHECK_FALSE(bin_t(1,1234).is_base());
			CHECK_TRUE(bin_t(0,12345).is_base());
			CHECK_EQUAL(bin_t(0,2).value(),bin_t(1,1).base_left().value());
			bin_t peaks[64];
			int peak_count = gen_peaks(7,peaks);
			CHECK_EQUAL(3,peak_count);
			CHECK_EQUAL(bin_t(2,0).value(),peaks[0].value());
			CHECK_EQUAL(bin_t(1,2).value(),peaks[1].value());
			CHECK_EQUAL(bin_t(0,6).value(),peaks[2].value());

		}

		UNITTEST_TEST(Bits) 
		{
			bin_t all = bin_t::ALL, none = bin_t::NONE, big = bin_t(40,18);
			u32 a32 = bin_toUInt32(all), n32 = bin_toUInt32(none), b32 = bin_toUInt32(big);
			CHECK_EQUAL(0x7fffffff,a32);
			CHECK_EQUAL(0xffffffff,n32);
			CHECK_EQUAL(bin_t::NONE.value(),bin_fromUInt32(b32).value());
		}
	}
}
UNITTEST_SUITE_END
