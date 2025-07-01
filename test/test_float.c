#include "hgl_test.h"

#include "hgl_int.h"
#include "hgl_float.h"

#include <math.h>

static inline bool float_eq(f64 a, f64 b, f64 epsilon)
{
    return (fabs(a - b) < epsilon);
}

TEST(test_f32_bits)
{
    union {
        f32 flt;
        u32 bits;
    } u;
    u.bits = 0xAAAABBBB;
    ASSERT(f32_to_bits(u.flt) == u.bits);
    ASSERT(f32_sign(u.flt) == 1);
    ASSERT(f32_mantissa(u.flt) == (0xAAAABBBB & 0x7FFFFF));
    ASSERT(f32_exponent(u.flt) == (0xAAAABBBB & 0x7F800000) >> 23);
}

TEST(test_f64_bits)
{
    union {
        f64 flt;
        u64 bits;
    } u;
    u.bits = 0xAAAABBBBCCCCDDDD;
    ASSERT(f64_to_bits(u.flt) == u.bits);
    ASSERT(f64_sign(u.flt) == 1);
    ASSERT(f64_mantissa(u.flt) == (0xAAAABBBBCCCCDDDD & 0x000FFFFFFFFFFFFF));
    ASSERT(f64_exponent(u.flt) == (0xAAAABBBBCCCCDDDD & 0x7FF0000000000000) >> 52);
}

TEST(test_fixed_point_radix)
{
    f64 f, e, v;
    int b, r;

    ASSERT(float_eq(fixed_radix_to_epsilon(8), 1.0/256.0, 0.00001));
    
    f = 1234.5678; b = 64; r = 32; e = 0.0001; 
    ASSERT(float_eq(f64_clamp_to_fixed_range_radix(f, b, r, false), f, 0.0001));
    ASSERT(float_eq(f, f64_from_fixed_radix(f64_to_fixed_radix(f, b, r), b, r, false), e));

    f = 123.5678; b = 16; r = 8; e = fixed_radix_to_epsilon(r) + 0.0001; 
    ASSERT(float_eq(f64_clamp_to_fixed_range_radix(f, b, r, false), f, 0.0001));
    ASSERT(float_eq(f, f64_from_fixed_radix(f64_to_fixed_radix(f, b, r), b, r, false), e));

    f = 123.5678; b = 16; r = 0; e = fixed_radix_to_epsilon(r) + 0.0001; 
    ASSERT(float_eq(f64_clamp_to_fixed_range_radix(f, b, r, false), f, 0.0001));
    ASSERT(float_eq(f, f64_from_fixed_radix(f64_to_fixed_radix(f, b, r), b, r, false), e));

    f = -123.5678; b = 16; r = 4; e = fixed_radix_to_epsilon(4) + 0.0001; 
    ASSERT(float_eq(f64_clamp_to_fixed_range_radix(f, b, r, true), f, 0.0001));
    ASSERT(float_eq(f, f64_from_fixed_radix(f64_to_fixed_radix(f, b, r), b, r, true), e));


    f = -123.5678; b = 16; r = 10; e = fixed_radix_to_epsilon(4) + 0.0001; 
    ASSERT(!float_eq(f64_clamp_to_fixed_range_radix(f, b, r, true), f, 0.1));
    ASSERT(!float_eq(f, f64_from_fixed_radix(f64_to_fixed_radix(f, b, r), b, r, true), e)); // should fail. -123.56 wont fit.
}

TEST(test_fixed_point_scale)
{
    f64 f, s;
    int b;

    f = 1234.5678; b = 64; s = 1.0/100.0; 
    ASSERT(float_eq(f64_clamp_to_fixed_range_scale(f, b, s, false), f, 0.0001));
    ASSERT(float_eq(f, f64_from_fixed_scale(f64_to_fixed_scale(f, b, s), b, s, false), s));

    f = 1234.5678; b = 16; s = 1.0/10.0; 
    ASSERT(float_eq(f64_clamp_to_fixed_range_scale(f, b, s, false), f, 0.0001));
    ASSERT(float_eq(f, f64_from_fixed_scale(f64_to_fixed_scale(f, b, s), b, s, false), s));

    f = 1234.5678; b = 16; s = 1.0/100.0; 
    ASSERT(!float_eq(f64_clamp_to_fixed_range_scale(f, b, s, false), f, 0.0001));
    ASSERT(!float_eq(f, f64_from_fixed_scale(f64_to_fixed_scale(f, b, s), b, s, false), s));
}

TEST(test_brain_float)
{
    f32 f;
    bf16 bf;

    f = 123.456f;
    ASSERT(float_eq(bf16_to_f32(bf16_from_f32(f)), f, 1.0));

    f = 12.456f;
    ASSERT(float_eq(bf16_to_f32(bf16_from_f32(f)), f, 0.1));

    f = 121234.456f;
    LOG("%f\n", (f64) bf16_to_f32(bf16_from_f32(f)));
    ASSERT(float_eq(bf16_to_f32(bf16_from_f32(f)), f, 1000));
}

TEST(test_half_precision)
{
    f32 f;
    f16 g;

    f = 123.456f;
    ASSERT(float_eq(f16_to_f32(f16_from_f32(f)), f, 1.0));

    f = 12.456f;
    ASSERT(float_eq(f16_to_f32(f16_from_f32(f)), f, 0.1));

    f = 121234.456f;
    LOG("%f\n", (f64) f16_to_f32(f16_from_f32(f)));
    ASSERT(float_eq(f16_to_f32(f16_from_f32(f)), f, 1000));
}
