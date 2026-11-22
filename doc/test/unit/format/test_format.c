#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "core/format.h"

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

static void check_str(const char* got, const char* want)
{
	if (strcmp(got, want) != 0)
	{
		printf("FAIL str: got [%s] want [%s]\n", got, want);
		failures++;
	}
}

static void test_dtos(void)
{
	char buf[32];
	s8_t i8 = -128;
	s16_t i16 = -32768;
	s32_t i32 = (s32_t)0x80000000;
	s64_t i64 = (s64_t)0x8000000000000000ULL;
	s32_t zero = 0;
	s32_t one = 1;
	CHECK(format_dtos(buf, sizeof(buf), &i8, 1));
	check_str(buf, "-128");
	CHECK(format_dtos(buf, sizeof(buf), &i16, 2));
	check_str(buf, "-32768");
	CHECK(format_dtos(buf, sizeof(buf), &i32, 4));
	check_str(buf, "-2147483648");
	CHECK(format_dtos(buf, sizeof(buf), &i64, 8));
	check_str(buf, "-9223372036854775808");
	CHECK(format_dtos(buf, sizeof(buf), &zero, 4));
	check_str(buf, "0");
	CHECK(format_dtos(buf, sizeof(buf), &one, 4));
	check_str(buf, "1");
	CHECK(!format_dtos(buf, 2, &i32, 4));
	CHECK(!format_dtos(buf, sizeof(buf), &i32, 3));
	CHECK(!format_dtos(0, sizeof(buf), &i32, 4));
	CHECK(!format_dtos(buf, sizeof(buf), 0, 4));
}

static void test_utos(void)
{
	char buf[32];
	u8_t u8 = 255;
	u16_t u16 = 65535;
	u32_t u32 = 0xFFFFFFFFu;
	u64_t u64 = 0xFFFFFFFFFFFFFFFFULL;
	u32_t zero = 0;
	CHECK(format_utos(buf, sizeof(buf), &u8, 1));
	check_str(buf, "255");
	CHECK(format_utos(buf, sizeof(buf), &u16, 2));
	check_str(buf, "65535");
	CHECK(format_utos(buf, sizeof(buf), &u32, 4));
	check_str(buf, "4294967295");
	CHECK(format_utos(buf, sizeof(buf), &u64, 8));
	check_str(buf, "18446744073709551615");
	CHECK(format_utos(buf, sizeof(buf), &zero, 4));
	check_str(buf, "0");
	CHECK(!format_utos(buf, 2, &u32, 4));
}

static void test_mtos(void)
{
	char buf[16];
	unsigned char bytes[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
	unsigned char one = 0x0A;
	CHECK(format_mtos(buf, sizeof(buf), bytes, 4));
	check_str(buf, "deadbeef");
	CHECK(format_mtos(buf, sizeof(buf), &one, 1));
	check_str(buf, "0a");
	CHECK(!format_mtos(buf, 8, bytes, 4));
	CHECK(!format_mtos(buf, sizeof(buf), bytes, 0));
}

static void test_ftos(void)
{
	char buf[64];
	f32_t f;
	f64_t d;
	f = 0.0f;
	CHECK(format_ftos(buf, sizeof(buf), &f, 4));
	check_str(buf, "0");
	f = -0.0f;
	CHECK(format_ftos(buf, sizeof(buf), &f, 4));
	check_str(buf, "-0");
	f = 0.1f;
	CHECK(format_ftos(buf, sizeof(buf), &f, 4));
	check_str(buf, "0.1");
	CHECK((float)strtod(buf, 0) == f);
	f = 1.5f;
	CHECK(format_ftos(buf, sizeof(buf), &f, 4));
	check_str(buf, "1.5");
	f = FLT_MAX;
	CHECK(format_ftos(buf, sizeof(buf), &f, 4));
	CHECK((float)strtod(buf, 0) == f);
	f = FLT_MIN;
	CHECK(format_ftos(buf, sizeof(buf), &f, 4));
	CHECK((float)strtod(buf, 0) == f);

	d = 0.1;
	CHECK(format_ftos(buf, sizeof(buf), &d, 8));
	check_str(buf, "0.1");
	CHECK(strtod(buf, 0) == d);
	d = 1e20;
	CHECK(format_ftos(buf, sizeof(buf), &d, 8));
	CHECK(strtod(buf, 0) == d);
	d = 1e-20;
	CHECK(format_ftos(buf, sizeof(buf), &d, 8));
	CHECK(strtod(buf, 0) == d);
	d = DBL_MAX;
	CHECK(format_ftos(buf, sizeof(buf), &d, 8));
	CHECK(strtod(buf, 0) == d);
	d = DBL_MIN;
	CHECK(format_ftos(buf, sizeof(buf), &d, 8));
	CHECK(strtod(buf, 0) == d);

	d = NAN;
	CHECK(format_ftos(buf, sizeof(buf), &d, 8));
	check_str(buf, "nan");
	d = INFINITY;
	CHECK(format_ftos(buf, sizeof(buf), &d, 8));
	check_str(buf, "inf");
	d = -INFINITY;
	CHECK(format_ftos(buf, sizeof(buf), &d, 8));
	check_str(buf, "-inf");

	f = 1.0f;
	CHECK(!format_ftos(buf, 1, &f, 4));
	CHECK(!format_ftos(buf, sizeof(buf), &f, 2));
	CHECK(!format_ftos(0, sizeof(buf), &f, 4));
}

int main(void)
{
	test_dtos();
	test_utos();
	test_mtos();
	test_ftos();
	if (failures)
	{
		printf("format tests: FAIL (%d)\n", failures);
		return 1;
	}
	printf("format tests: PASS\n");
	return 0;
}