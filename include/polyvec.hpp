#pragma once
#include "bit_packing.hpp"
#include "poly.hpp"

// Utility functions for Dilithium Post-Quantum Digital Signature Algorithm
namespace dilithium_utils {

// Applies NTT on a vector ( of dimension k x 1 ) of degree-255 polynomials
template<const size_t k>
inline static void
polyvec_ntt(ff::ff_t* const __restrict vec)
{
  for (size_t i = 0; i < k; i++) {
    const size_t off = i * ntt::N;
    ntt::ntt(vec + off);
  }
}

// Applies iNTT on a vector ( of dimension k x 1 ) of degree-255 polynomials
template<const size_t k>
inline static void
polyvec_intt(ff::ff_t* const __restrict vec)
{
  for (size_t i = 0; i < k; i++) {
    const size_t off = i * ntt::N;
    ntt::intt(vec + off);
  }
}

// Compresses vector ( of dimension k x 1 ) of degree-255 polynomials by
// extracting out high and low order bits
template<const size_t k, const size_t d>
inline static void
polyvec_power2round(const ff::ff_t* const __restrict poly,
                    ff::ff_t* const __restrict poly_hi,
                    ff::ff_t* const __restrict poly_lo)
  requires(check_d(d))
{
  for (size_t i = 0; i < k; i++) {
    const size_t off = i * ntt::N;
    poly_power2round<d>(poly + off, poly_hi + off, poly_lo + off);
  }
}

// Compile-time check to ensure that operand matrices are having compatible
// dimension for matrix multiplication
static inline constexpr bool
check_matrix_dim(const size_t a_cols, const size_t b_rows)
{
  return !static_cast<bool>(a_cols ^ b_rows);
}

// Given two matrices ( in NTT domain ) of compatible dimension, where each
// matrix element is a degree-255 polynomial over Z_q | q = 2^23 -2^13 + 1, this
// routine attempts to multiply and compute resulting matrix
template<const size_t a_rows,
         const size_t a_cols,
         const size_t b_rows,
         const size_t b_cols>
static void
matrix_multiply(const ff::ff_t* const __restrict a,
                const ff::ff_t* const __restrict b,
                ff::ff_t* const __restrict c)
  requires(check_matrix_dim(a_cols, b_rows))
{
  ff::ff_t tmp[ntt::N]{};

  for (size_t i = 0; i < a_rows; i++) {
    for (size_t j = 0; j < b_cols; j++) {
      const size_t coff = (i * b_cols + j) * ntt::N;

      for (size_t k = 0; k < a_cols; k++) {
        const size_t aoff = (i * a_cols + k) * ntt::N;
        const size_t boff = (k * b_cols + j) * ntt::N;

        polymul(a + aoff, b + boff, tmp);

        for (size_t l = 0; l < ntt::N; l++) {
          c[coff + l] = c[coff + l] + tmp[l];
        }
      }
    }
  }
}

// Given a vector ( of dimension k x 1 ) of degree-255 polynomials, this
// routine adds it to another polynomial vector of same dimension s.t.
// destination vector is mutated.
template<const size_t k>
inline static void
polyvec_add_to(const ff::ff_t* const __restrict src,
               ff::ff_t* const __restrict dst)
{
  for (size_t i = 0; i < k; i++) {
    const size_t off = i * ntt::N;

    for (size_t l = 0; l < ntt::N; l++) {
      dst[off + l] = dst[off + l] + src[off + l];
    }
  }
}

// Given a vector ( of dimension k x 1 ) of degree-255 polynomials, this
// routine negates each coefficient.
template<const size_t k>
inline static void
polyvec_neg(ff::ff_t* const __restrict vec)
{
  for (size_t i = 0; i < k; i++) {
    const size_t off = i * ntt::N;

    for (size_t l = 0; l < ntt::N; l++) {
      vec[off + l] = -vec[off + l];
    }
  }
}

// Given a vector ( of dimension k x 1 ) of degree-255 polynomials s.t. each
// coefficient ??? [-x, x], this routine subtracts each coefficient from x so that
// coefficients now stay in [0, 2x].
template<const size_t k, const uint32_t x>
inline static void
polyvec_sub_from_x(ff::ff_t* const vec)
{
  for (size_t i = 0; i < k; i++) {
    const size_t off = i * ntt::N;
    poly_sub_from_x<x>(vec + off);
  }
}

// Given a vector ( of dimension k x 1 ) of degree-255 polynomials, this routine
// encodes each of those polynomials into 32 x sbw -bytes, writing to a
// (k x 32 x sbw) -bytes destination array
template<const size_t k, const size_t sbw>
inline static void
polyvec_encode(const ff::ff_t* const __restrict src,
               uint8_t* const __restrict dst)
{
  for (size_t i = 0; i < k; i++) {
    const size_t off0 = i * ntt::N;
    const size_t off1 = i * sbw * 32;

    encode<sbw>(src + off0, dst + off1);
  }
}

// Given a byte array of length (k x 32 x sbw) -bytes, this routine decodes them
// into k degree-255 polynomials, writing them to a column vector of dimension
// k x 1
template<const size_t k, const size_t sbw>
inline static void
polyvec_decode(const uint8_t* const __restrict src,
               ff::ff_t* const __restrict dst)
{
  for (size_t i = 0; i < k; i++) {
    const size_t off0 = i * sbw * 32;
    const size_t off1 = i * ntt::N;

    decode<sbw>(src + off0, dst + off1);
  }
}

// Given a vector ( of dimension k x 1 ) of degree-255 polynomials, this routine
// extracts out high order bits from each coefficient
template<const size_t k, const uint32_t alpha>
inline static void
polyvec_highbits(const ff::ff_t* const __restrict src,
                 ff::ff_t* const __restrict dst)
{
  for (size_t i = 0; i < k; i++) {
    const size_t off = i * ntt::N;
    poly_highbits<alpha>(src + off, dst + off);
  }
}

// Given a vector ( of dimension k x 1 ) of degree-255 polynomials, this routine
// extracts out low order bits from each coefficient, while not mutating operand
template<const size_t k, const uint32_t alpha>
inline static void
polyvec_lowbits(const ff::ff_t* const __restrict src,
                ff::ff_t* const __restrict dst)
{
  for (size_t i = 0; i < k; i++) {
    const size_t off = i * ntt::N;
    poly_lowbits<alpha>(src + off, dst + off);
  }
}

// Given a vector ( of dimension k x 1 ) of degree-255 polynomials and one
// multiplier polynomial, this routine performs k pointwise polynomial
// multiplications when each of these polynomials are in their NTT
// representation, while not mutating operand polynomials.
template<const size_t k>
inline static void
polyvec_mul_poly(const ff::ff_t* const __restrict poly,
                 const ff::ff_t* const __restrict src_vec,
                 ff::ff_t* const __restrict dst_vec)
{
  for (size_t i = 0; i < k; i++) {
    const size_t off = i * ntt::N;
    polymul(poly, src_vec + off, dst_vec + off);
  }
}

// Computes infinity norm of a vector ( of dimension k x 1 ) of degree-255
// polynomials
//
// See point `Sizes of elements` in section 2.1 of Dilithium specification
// https://pq-crystals.org/dilithium/data/dilithium-specification-round3-20210208.pdf
template<const size_t k>
inline static ff::ff_t
polyvec_infinity_norm(const ff::ff_t* const __restrict vec)
{
  ff::ff_t res{ 0u };

  for (size_t i = 0; i < k; i++) {
    const size_t off = i * ntt::N;
    res = std::max(res, poly_infinity_norm(vec + off));
  }

  return res;
}

// Given two vector ( of dimension k x 1 ) of degree-255 polynomials, this
// routine computes hint bit for each coefficient, using `make_hint` routine.
template<const size_t k, const uint32_t alpha>
inline static void
polyvec_make_hint(const ff::ff_t* const __restrict polya,
                  const ff::ff_t* const __restrict polyb,
                  ff::ff_t* const __restrict polyc)
{
  for (size_t i = 0; i < k; i++) {
    const size_t off = i * ntt::N;
    poly_make_hint<alpha>(polya + off, polyb + off, polyc + off);
  }
}

// Recovers high order bits of a vector of degree-255 polynomials (  i.e. r + z
// ) s.t. hint bits ( say h ) and another polynomial vector ( say r ) are
// provided.
template<const size_t k, const uint32_t alpha>
inline static void
polyvec_use_hint(const ff::ff_t* const __restrict polyh,
                 const ff::ff_t* const __restrict polyr,
                 ff::ff_t* const __restrict polyrz)
{
  for (size_t i = 0; i < k; i++) {
    const size_t off = i * ntt::N;
    poly_use_hint<alpha>(polyh + off, polyr + off, polyrz + off);
  }
}

// Given a vector ( of dimension k x 1 ) of degree-255 polynomials, this routine
// counts number of coefficients having value 1.
template<const size_t k>
inline static size_t
polyvec_count_1s(const ff::ff_t* const __restrict vec)
{
  size_t cnt = 0;

  for (size_t i = 0; i < k; i++) {
    const size_t off = i * ntt::N;
    cnt += count_1s(vec + off);
  }

  return cnt;
}

// Given a vector ( of dimension k x 1 ) of degree-255 polynomials, this routine
// shifts each coefficient leftwards by d bits
template<const size_t k, const size_t d>
inline static void
polyvec_shl(ff::ff_t* const __restrict vec)
{
  for (size_t i = 0; i < k; i++) {
    const size_t off = i * ntt::N;
    poly_shl<d>(vec + off);
  }
}

}
