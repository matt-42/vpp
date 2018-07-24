#pragma once

#ifdef __ARM_NEON__
#  include <arm_neon.h>
#endif

#if defined(__SSE4_1__) || defined(__AVX2__)
# include "immintrin.h"
#endif

#include <bitset>
#include <thread>
#include <vpp/vpp.hh>
#include <stdint.h>
#include <float.h>

namespace vpp
{
  
  namespace FAST_internals
  {

    typedef unsigned int uint;
    
    inline bool fast9_check_code(uint code32)
    {
      // Thanks Arkanosis (https://github.com/Arkanosis) for the idea.
      uint64_t code48 = code32;
      code48 |= code48 << 32;
      code48 &= code48 << 8;
      code48 &= code48 << 4;
      code48 &= code48 << 2;
      return (code48 & (code48 << 2));
    }

    template <typename N>
    inline
    int fast9_score(N nbh,
                    int th)
    {
      typedef decltype(nbh(0, 0)) V;
      V v = nbh(0, 0);
      int sum_inf = 0;
      int sum_sup = 0;

      auto f = [&] (V a) {
        int diff = v - a;
        if (diff < -th) sum_inf -= diff;
        else if (diff > th) sum_sup += diff;
      };

      f(nbh(-3, -1));
      f(nbh(-3,  0));
      f(nbh(-3,  1));

      f(nbh(-2,  2));

      f(nbh(-1, 3));
      f(nbh( 0, 3));
      f(nbh( 1, 3));

      f(nbh(2,  2));

      f(nbh(3, +1));
      f(nbh(3,  0));
      f(nbh(3, -1));

      f(nbh( 2, -2));

      f(nbh( 1, -3));
      f(nbh( 0, -3));
      f(nbh(-1, -3));

      f(nbh(-2, -2));

      return std::max(sum_sup, sum_inf);
    }

    template <typename N>
    bool is_fast9_keypoint(const N& n, int th)
    {
      typedef typename N::value_type V;

      V v = n(0, 0);
      
      auto f = [&] (V a) -> int { return ((a > v + th) << 1) ^ (a < v - th); };

      uint x  =
        (f(n(3, -1)) << 20) +
        (f(n(3,  0)) << 18) +
        (f(n(3, +1)) << 16) +

        (f(n( 2, -2)) << 22) +
        (f(n(2,  2)) << 14) +

        (f(n( 1, -3)) << 24) +
        (f(n( 1, 3)) << 12) +
        (f(n( 0, -3)) << 26) +
        (f(n( 0, 3)) << 10) +

        (f(n(-1, -3)) << 28) +
        (f(n(-1, 3)) << 8) +

        (f(n(-2, -2)) << 30) +
        (f(n(-2,  2)) << 6) +

        f(n(-3, -1)) +
        (f(n(-3,  0)) << 2) +
        (f(n(-3,  1)) << 4);
  
      return fast9_check_code(x);
    }

#ifdef __AVX2__
    struct fast9_simd
    {
      typedef __m256i V;
      enum { size = 32, size_in_bits = 256 };

      static V check(V x, V hi, V lo)
      {
        __m256i _1 = ::_mm256_set1_epi8(1);
        __m256i a = ::_mm256_min_epu8(_1, ::_mm256_subs_epu8(x, hi));
        __m256i b = ::_mm256_min_epu8(_1, _mm256_subs_epu8(lo, x));        
        return ::_mm256_slli_epi16(a, 4) | b;
      };

      static V u_subs(V a, V b) { return ::_mm256_subs_epu8(a, b); }
      static V u_adds(V a, V b) { return ::_mm256_adds_epu8(a, b); }
      static bool all_equal_zero(V a) {return ::_mm256_testz_si256(a, ::_mm256_set1_epi8(255)) == 1;}
      static V repeat(int v) { return ::_mm256_set1_epi8(v); }
      static V load(const void* ptr) { return ::_mm256_load_si256((const __m256i*) (ptr)); }
      static V loadu(const void* ptr) { return ::_mm256_loadu_si256((const __m256i*) (ptr)); }
      static void storeu(void* ptr, V x) { ::_mm256_storeu_si256((__m256i*) (ptr), x); }
    };
#else

#  ifdef __SSE4_1__
    struct fast9_simd // SSE4 version
    {
      typedef __m128i V;
      enum { size = 16, size_in_bits = 128 };

      static V check(V x, V hi, V lo)
      {
        V _1 = ::_mm_set1_epi8(1);
        V a = ::_mm_min_epu8(_1, ::_mm_subs_epu8(x, hi));
        V b = ::_mm_min_epu8(_1, ::_mm_subs_epu8(lo, x));
        return ::_mm_slli_epi16(a, 4) | b;
      };

      static V u_subs(V a, V b) { return ::_mm_subs_epu8(a, b); }
      static V u_adds(V a, V b) { return ::_mm_adds_epu8(a, b); }
      static bool all_equal_zero(V a) { return ::_mm_testz_si128(a, ::_mm_set1_epi8(255)) == 1; }
      static V repeat(int v) { return ::_mm_set1_epi8(v); }
      static V load(const void* ptr) { return ::_mm_load_si128((const V*) (ptr)); }
      static V loadu(const void* ptr) { return ::_mm_loadu_si128((const V*) (ptr)); }
      static void storeu(void* ptr, V x) { ::_mm_storeu_si128((V*) (ptr), x); }
    };

#  else

#    ifdef __ARM_NEON
#    include <arm_neon.h>

    struct fast9_simd // ARM NEON version
    {
      typedef uint8x16_t V;
      enum { size = 16, size_in_bits = 128 };

      static V check(V x, V hi, V lo)
      {
        V _1 = vdupq_n_u8(1);
        V a = vminq_u8(_1, vqsubq_u8(x, hi));
        V b = vminq_u8(_1, vqsubq_u8(lo, x));
        return vshlq_n_u8(a, 4) | b;
      };

      static V u_subs(V a, V b) { return vqsubq_u8(a, b); }
      static V u_adds(V a, V b) { return vqaddq_u8(a, b); }
      static bool all_equal_zero(V a) {

#ifdef __aarch64__
        V tmp;
        int res;
        asm("CMTST     %[tmp].16B, %[in].16B, %[in].16B\n\t"
            "UQADD   %[tmp].16B, %[tmp].16B, %[tmp].16B\n\t"
            "MRS        %[out],FPSR"
            : [out] "=r" (res), [tmp] "=w" (tmp)
            : [in] "w" (a)
          );

        return !(res & (1 << 27));
#else
        V tmp;
        int res;
        asm("VTST.8     %q[tmp], %q[in], %q[in]\n\t"
            "VQADD.u8   %q[tmp], %q[tmp]\n\t"
            "VMRS        %[out],FPSCR"
            : [out] "=r" (res), [tmp] "=w" (tmp)
            : [in] "w" (a)
          );

        return !(res & (1 << 27));
#endif
      }
      static V repeat(int v) { return vdupq_n_u8(v); }
      static V load(const void* ptr) {

#ifdef __aarch64__
        V res;
        const uint8_t *aligned_ptr = (const uint8_t*)__builtin_assume_aligned(ptr, 128);
        return vld1q_u8(aligned_ptr);
#else
        V res;
        asm("VLD1.8 {%q[out]}, [%[in]:128]"
            : [out] "=w" (res)
            : [in]"r" (ptr)
          );
        return res;
#endif
      }
      static V loadu(const void* ptr) { return vld1q_u8((const uint8_t*) (ptr)); }
      static void storeu(void* ptr, V x) { vst1q_u8((uint8_t*) (ptr), x); }
    };

#    else

    struct fast9_simd // Fallback version
    {
      typedef unsigned char V;
      enum { size = 1, size_in_bits = 8 };

      static V check(V x, V hi, V lo) { return ((x > hi) << 4) | (x < lo); };
      static V u_subs(V a, V b) { return a > b ? a - b : 0; }
      static V u_adds(V a, V b) 
      {
        int s = a + b;
        return s > 255 ? 255 : s;
      }
      static bool all_equal_zero(V a) { return a == 0; }
      static V repeat(int v) { return v; }
      static V load(const void* ptr) { return *(const V*) (ptr); }
      static V loadu(const void* ptr) { return *(const V*) (ptr); }
      static void storeu(void* ptr, V x) { (*(V*) (ptr)) = x; }
    };

#    endif

#  endif
#endif

    template <typename V, typename M>
    void fast_detector9_simd(const image2d<V>& A, std::vector<vint2>& keypoints, int th,
                             const M& mask)
    {
      int nc = A.ncols();
      int nr = A.nrows();
      int pitch = A.pitch();

      auto shift_row = [] (V* ptr, int o) { return (V*)(((char*)ptr) + o); };

      #pragma omp parallel
      {

#define LSB_MAX_SIZE (2000)
        vint2 local_static_buffer[LSB_MAX_SIZE];
        int local_static_buffer_size = 0;

        auto export_keypoints = [&] ()
        {
          int last_idx;
#pragma omp critical          
          keypoints.insert(keypoints.end(), local_static_buffer, local_static_buffer + local_static_buffer_size);
          local_static_buffer_size = 0;
        };
        
        auto add_keypoint = [&] (vint2 p)
        {
          if (local_static_buffer_size >= LSB_MAX_SIZE)
            export_keypoints();

          local_static_buffer[local_static_buffer_size] = p;
          local_static_buffer_size++;
        };
        
#pragma omp for
      for (int r = 0; r < nr; r++)
      {
        V* a_row = const_cast<V*>(&A(r, 0));
        V* a_row1 = shift_row(a_row, -3*pitch);
        V* a_row2 = shift_row(a_row, -2*pitch);
        V* a_row3 = shift_row(a_row, -1*pitch);
        V* a_row4 = a_row;
        V* a_row5 = shift_row(a_row, 1*pitch);
        V* a_row6 = shift_row(a_row, 2*pitch);
        V* a_row7 = shift_row(a_row, 3*pitch);

        const unsigned char* m_row = mask.has_data() ? &mask(r, 0) : 0;
        typedef fast9_simd S;

        typedef typename S::V v;
        int c;
        for (c = 0; c < nc; c += S::size)
        {
          v hi,lo;
          v a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15;

          v possible;
          if (m_row)
          {
            possible = S::load(m_row + c);
            if (S::all_equal_zero(possible))
              continue;
          }
          else
            possible = S::repeat(255);

          {
            v here = S::load(a_row4 + c);
            v th_ = S::repeat(th);
            hi = S::u_adds(here, th_);
            lo = S::u_subs(here, th_);
          }

          {
            v a = S::load(a_row1 + c);
            v b = S::load(a_row7 + c);

            a0 = S::check(a, hi, lo);
            a8 = S::check(b, hi, lo);

            possible &= a0 | a8;
            if (S::all_equal_zero(possible))
              continue;

          }

          {
            v a = S::loadu(a_row1 + c - 1);
            v b = S::loadu(a_row1 + c + 1);

            a15 = S::check(a, hi, lo);
            a1 = S::check(b, hi, lo);
            possible &= a8 | (a15 & a1);

            if (S::all_equal_zero(possible))
              continue;
          }

          {
            v a = S::loadu(a_row7 + c - 1);
            v b = S::loadu(a_row7 + c + 1);

            a9 = S::check(a, hi, lo);
            a7 = S::check(b, hi, lo);

            possible &= a9 | (a0 & a1);
            possible &= a7 | (a15 & a0);

            if (S::all_equal_zero(possible))
              continue;
          }


          {
            v a = S::loadu(a_row1 + c - 3);
            v b = S::loadu(a_row1 + c + 3);

            a12 = S::check(a, hi, lo);
            a4 = S::check(b, hi, lo);

            possible &= a12 | (a4 & (a1 | a7));
            possible &= a4 | (a12 & (a9 | a15));

            if (S::all_equal_zero(possible))
              continue;
          }


          {
            v a = S::loadu(a_row2 + c - 2);
            v b = S::loadu(a_row6 + c + 2);

            a14 = S::check(a, hi, lo);
            a6 = S::check(b, hi, lo);

            {
              v a6_7 = a6 & a7;
              possible &= a14 | (a6_7 & (a4 | (a8 & a9)));
              possible &= a1 | (a6_7) | a12;
            }
            {
              v a14_15 = a14 & a15;
              possible &= a6 | (a14_15 & (a12 | (a0 & a1)));
              possible &= a9 | (a14_15) | a4;
            }

            if (S::all_equal_zero(possible))
              continue;
          }


          {
            v a = S::loadu(a_row6 + c - 2);
            v b = S::loadu(a_row2 + c + 2);

            a10 = S::check(a, hi, lo);
            a2 = S::check(b, hi, lo);

            {
              v a1_2 = a1 & a2;
              possible &= a10 | (a1_2 & ((a0 & a15) | a4));
              possible &= a12 | (a1_2) | (a6 & a7);
            }
            {
              v a9_10 = a9 & a10;
              possible &= a2 | (a9_10 & ((a7 & a8) | a12));
              possible &= a4 | (a9_10) | (a14 & a15);
            }
            possible &= a8 | a14 | a2;
            possible &= a0 | a10 | a6;

            if (S::all_equal_zero(possible))
              continue;
          }

          {
            v a = S::loadu(a_row3 + c - 3);
            v b = S::loadu(a_row5 + c + 3);

            a13 = S::check(a, hi, lo);
            a5 = S::check(b, hi, lo);

            v a15_0 = a15 & a0;
            v a7_8 = a7 & a8;
            {
              v a12_13 = a12 & a13;
              possible &= a5 | (a12_13 & a14 & ((a15_0) | a10));
              possible &= a7 | (a1 & a2) | (a12_13);
              possible &= a2 | (a12_13) | (a7_8);
            }
            {
              v a4_5 = a4 & a5;
              v a9_10 = a9 & a10;
              possible &= a13 | (a4_5 & a6 & ((a7_8) | a2));
              possible &= a15 | (a4_5) | (a9_10);
              possible &= a10 | (a4_5) | (a15_0);
              possible &= a15 | (a9_10) | (a4_5);
            }
            possible &= a8 | (a13 & a14) | a2;
            possible &= a0 | (a5 & a6) | a10;

            if (S::all_equal_zero(possible))
              continue;
          }


          {
            v a = S::loadu(a_row5 + c - 3);
            v b = S::loadu(a_row3 + c + 3);

            a11 = S::check(a, hi, lo);
            a3 = S::check(b, hi, lo);

            {
              v a2_3 = a2 & a3;
              possible &= a11 | (a2_3 & a4 & ((a0 & a1) | (a5 & a6)));
              possible &= a13 | (a7 & a8) | (a2_3);
              possible &= a8 | (a2_3) | (a13 & a14);
            }
            {
              v a11_12 = a11 & a12;
              possible &= a3 | (a10 & a11_12 & ((a8 & a9) | (a13 & a14)));
              possible &= a1 | (a11_12) | (a6 & a7);
              possible &= a6 | (a0 & a1) | (a11_12);
            }
            {
              v a3_4 = a3 & a4;
              possible &= a9 | (a3_4) | (a14 & a15);
              possible &= a14 | (a8 & a9) | (a3_4);
            }
            {
              v a10_11 = a10 & a11;
              possible &= a5 | (a15 & a0) | (a10_11);
              possible &= a0 | (a10_11) | (a5 & a6);
            }

            if (S::all_equal_zero(possible))
              continue;
          }

          {
            unsigned char is_corner[S::size];
            S::storeu(is_corner, possible);
            // if (is_corner[i] and (c + i) < nc) add_keypoint(vint2{r, c + i});
            for (int i = 0; i < S::size; i++)
              if (is_corner[i] and (c + i) < nc) add_keypoint(vint2{r, c + i});
            
          }
        }

      }

      export_keypoints();
    }

    }

    
    template <typename V, typename U>
    void fast_detector9(const image2d<V>& A, image2d<U>& B, int th)
    {
      int nc = A.ncols();
      int nr = A.nrows();
      int pitch = A.pitch();

      pixel_wise(B, relative_access(A)) | [&] (U& b, auto n)
      {
        V v = n(0, 0);

        auto f = [&] (V a) -> int { return ((a > v + th) << 1) ^ (a < v - th); };

        uint x  =
          (f(n(3, -1)) << 20) +
          (f(n(3,  0)) << 18) +
          (f(n(3, +1)) << 16) +

          (f(n( 2, -2)) << 22) +
          (f(n(2,  2)) << 14) +


          (f(n( 1, -3)) << 24) +
          (f(n( 1, 3)) << 12) +
          (f(n( 0, -3)) << 26) +
          (f(n( 0, 3)) << 10) +

          (f(n(-1, -3)) << 28) +
          (f(n(-1, 3)) << 8) +

          (f(n(-2, -2)) << 30) +
          (f(n(-2,  2)) << 6) +

          f(n(-3, -1)) +
          (f(n(-3,  0)) << 2) +
          (f(n(-3,  1)) << 4);
  
        b = fast9_check_code(x);

      };
    }

  }

  template <typename V>
  void local_maxima_filter(const image2d<V>& A, int nbh_size) // At the moment nbh_size is ignored.
  {
    pixel_wise(A, relative_access(A)) | [] (V& a, auto nn)
    {
      V v = a;
      int is_max = 1;
      is_max &= a > nn(-1, -1);
      is_max &= a > nn(-1, 0);
      is_max &= a > nn(-1, 1);

      is_max &= a > nn(0, -1);
      is_max &= a > nn(0, 1);

      is_max &= a > nn(1, -1);
      is_max &= a > nn(1, 0);
      is_max &= a > nn(1, 1);

      if (!is_max) a = zero<V>();
    };
  }

  template <typename V>
  void blockwise_maxima_filter(const image2d<V>& A, int block_size)
  {
    int nc = A.ncols();
    int nr = A.nrows();
    int pitch = A.pitch();

    #pragma omp parallel for
    for (int r = 0; r < nr; r += block_size)
    {
      V* rows[block_size];
      for (int i = 0; i < block_size; i++)
        if (r + i < nr)
          rows[i] = &A(r + i, 0);

      for (int c = 0; c < nc; c += block_size)
      {
        // Maximum search.
        vint2 pmax;
        V vmax = 0;
        for (int br = 0; br < block_size; br++)
        for (int bc = c; bc < c + block_size; bc++)
          if (r + br < nr and bc < nc)
          {
            V v = rows[br][bc];
            rows[br][bc] = 0;
            if (v > vmax)
            {
              vmax = v;
              pmax = vint2(br, bc);
            }
          }

        if (vmax > 0)
          rows[pmax[0]][pmax[1]] = vmax;
      }
    }
  }

  template <typename V, typename F>
  inline std::vector<vint2> compact_coordinates_if(const image2d<V>& img, F f)
  {
    int nc = img.ncols();
    int nr = img.nrows();
    std::vector<vint2> points;

#pragma omp parallel
    {
      std::vector<vint2> local;
#pragma omp for
      for (int r = 0; r < nr; r++)
      {
        auto* row = &img(r, 0);
        for (int c = 0; c < nc; c++)
          if (f(row[c]))
            local.push_back(vint2(r, c));
      }
#pragma omp critical
      points.insert(points.end(), local.begin(), local.end());
    }

    return std::move(points);
  }


  template <typename V, typename KPS>
  void fast9_scores(const image2d<V>& A,
                    int th,
                    const KPS& keypoints,
                    std::vector<int>& scores)
  {
    scores.resize(keypoints.size());
#pragma omp parallel for
    for (int i = 0; i < keypoints.size(); i++)
      scores[i] = FAST_internals::fast9_score(relative_accessor(A, keypoints[i]), th);
  }

  template <typename V>
  int fast9_score(const image2d<V>& A,
                  int th,
                  vint2 p)
  {
    return FAST_internals::fast9_score(relative_accessor(A, p), th);
  }

  template <typename V>
  std::vector<vint2> fast_detector9(const image2d<V>& A,
                                    int th,
                                    const image2d<unsigned char>& mask,
                                    std::vector<int>* scores)
  {
    std::vector<vint2> kps;
    FAST_internals::fast_detector9_simd(A, kps, th, mask);
    if (scores)
      fast9_scores(A, th, kps, *scores);
    return std::move(kps);
  }

  template <typename V, typename F, typename M>
  auto fast_detector9_maxima(const image2d<V>& A,
                             int th,
                             const M& mask,
                             std::vector<int>* scores,
                             F maxima_filter)
  {
    std::vector<vint2> kps;
    FAST_internals::fast_detector9_simd(A, kps, th, mask);

    image2d<unsigned char> scores_img(A.domain(), _border = 1);
    fill_with_border(scores_img, 0);

    #pragma omp parallel for
    for (int i = 0; i < kps.size(); i++)
    {
      auto p = kps[i];
      int s = FAST_internals::fast9_score(relative_accessor(A, p), th);
      scores_img(p) = s / 16;
    }

    kps = maxima_filter(scores_img, kps);

    if (scores)
    {
      scores->resize(kps.size());
#pragma omp parallel for
      for (int i = 0; i < kps.size(); i++)
        (*scores)[i] = scores_img(kps[i]);
    }

    return std::move(kps);
  }


  template <typename V, typename F>
  auto fast_detector9_maxima2(const image2d<V>& A,
                              int th,
                              const image2d<int>& mask,
                              std::vector<int>* scores,
                              F maxima_filter)
  {
    std::vector<vint2> kps;
    FAST_internals::fast_detector9_simd(A, kps, th, mask);

    image2d<unsigned int> scores_img(A.domain(), _border = 1);
    fill_with_border(scores_img, 0);

    #pragma omp parallel for simd
    for (int i = 0; i < kps.size(); i++)
    {
      auto p = kps[i];
      int s = FAST_internals::fast9_score(relative_accessor(A, p), th);
      scores_img(p) = s;
    }

    auto kps2 = maxima_filter(scores_img, kps);

    if (scores)
    {
      scores->resize(kps2.size());
#pragma omp parallel for
      for (int i = 0; i < kps2.size(); i++)
        (*scores)[i] = scores_img(kps2[i].template segment<2>(0));
    }

    return std::move(kps2);
  }
  
  template <typename V>
  std::vector<vint2> fast_detector9_blockwise_maxima(const image2d<V>& A,
                                                     int th,
                                                     int block_size,
                                                     const image2d<unsigned char>& mask,
                                                     std::vector<int>* scores)
  {
    return fast_detector9_maxima(A, th, mask, scores,
                                 [block_size] (auto& S, auto& kps)
                                 {
                                   int nc = S.ncols();
                                   int nr = S.nrows();
                                   int pitch = S.pitch();
                                   std::vector<vint2> lms;

                                   #pragma omp parallel
                                   {
                                     std::vector<vint2> local;
                                     #pragma omp for
                                     for (int r = 0; r < nr; r += block_size)
                                     {
                                       unsigned char* rows[block_size];
                                       for (int i = 0; i < block_size; i++)
                                         if (r + i < nr)
                                           rows[i] = &S(r + i, 0);

                                       for (int c = 0; c < nc; c += block_size)
                                       {
                                         // Maximum search.
                                         vint2 pmax;
                                         unsigned int vmax = 0;
                                         for (int br = 0; br < block_size; br++)
                                           for (int bc = c; bc < c + block_size; bc++)
                                             if (r + br < nr and bc < nc)
                                             {
                                               unsigned int v = rows[br][bc];
                                               if (v > vmax)
                                               {
                                                 vmax = v;
                                                 pmax = vint2(br, bc);
                                               }
                                             }

                                         if (vmax > 0)
                                           local.push_back(vint2(r, 0) + pmax);
                                       }
                                     }
                                     #pragma omp critical
                                     lms.insert(lms.end(), local.begin(), local.end());
                                   }
                                   return lms;
                                   // Cleaner but slower..
                                   // blockwise_maxima_filter(S, block_size);
                                   // return compact_coordinates_if(S, [] (unsigned int& s) { return s != 0; });
                                 });
  }

  template <typename V>
  std::vector<vint3> fast_detector9_blockwise_rank(const image2d<V>& A,
                                                   int th,
                                                   int block_size,
                                                   int max_point_per_block,
                                                   const image2d<unsigned char>& mask,
                                                   std::vector<int>* scores)
  {
    return fast_detector9_maxima2(A, th, mask, scores,
                                 [=] (auto& S, auto& kps)
                                 {
                                   int nc = S.ncols();
                                   int nr = S.nrows();
                                   int pitch = S.pitch();
                                   std::vector<vint3> lms;

                                   #pragma omp parallel
                                   {
                                     std::vector<vint3> local;
                                     #pragma omp for
                                     for (int r = 0; r < nr; r += block_size)
                                     {
                                       unsigned int* rows[block_size];
                                       for (int i = 0; i < block_size; i++)
                                         if (r + i < nr)
                                           rows[i] = &S(r + i, 0);

                                       for (int c = 0; c < nc; c += block_size)
                                       {
                                         // Maximum search.
                                         vint2 pmax;
                                         unsigned int vmax = 0;
                                         std::pair<vint2, unsigned int> init{vint2{0,0}, 0};
                                         std::vector<std::pair<vint2, unsigned int>> pts(max_point_per_block, init);
                                         for (int br = 0; br < block_size; br++)
                                           for (int bc = c; bc < c + block_size; bc++)
                                             if (r + br < nr and bc < nc)
                                             {

                                               unsigned int v = rows[br][bc];
                                               if (v > 0)
                                               {
                                                 vint2 p(r + br, bc);
                                                 auto nn = relative_accessor(S, p);
                                                 unsigned int a = nn(0, 0);
                                                 int is_max = 1;
                                                 is_max &= a > nn(-1, -1);
                                                 is_max &= a > nn(-1, 0);
                                                 is_max &= a > nn(-1, 1);
        
                                                 is_max &= a > nn(0, -1);
                                                 is_max &= a > nn(0, 1);
        
                                                 is_max &= a > nn(1, -1);
                                                 is_max &= a > nn(1, 0);
                                                 is_max &= a > nn(1, 1);
        
                                                 if (is_max)
                                                 {
                                                   for (int k = 0; k < max_point_per_block; k++)
                                                     if (pts[k].second < v)
                                                     {
                                                       pts[k].second = v;
                                                       pts[k].first = vint2(br, bc);
                                                       break;
                                                     }
                                                 }
                                               }
                                             }
                                         std::sort(pts.begin(), pts.end(), [] (auto& a, auto& b) {
                                             return a.second > b.second; });

                                         for (int k = 0; k < max_point_per_block; k++)
                                           if (pts[k].second > 0)
                                             local.push_back(vint3(r + pts[k].first[0], pts[k].first[1], k));
                                       }
                                     }
                                     #pragma omp critical
                                     lms.insert(lms.end(), local.begin(), local.end());
                                   }
                                   return std::move(lms);
                                   // Cleaner but slower..
                                   // blockwise_maxima_filter(S, block_size);
                                   // return compact_coordinates_if(S, [] (unsigned int& s) { return s != 0; });
                                 });
  }
  
  template <typename V>
  std::vector<vint2> fast_detector9_local_maxima(const image2d<V>& A,
                                                 int th,
                                                 const image2d<unsigned char>& mask,
                                                 std::vector<int>* scores)
  {
    return fast_detector9_maxima(A, th, mask,
                                 scores,
                                 [] (auto& img, auto& kps)
                                 {
                                   std::vector<vint2> lms;
                                   #pragma omp parallel
                                   {
                                     std::vector<vint2> local;
                                     #pragma omp for
                                     for (int i = 0; i < kps.size(); i++)
                                     {
                                       auto p = kps[i];
                                       auto nn = relative_accessor(img, p);
                                       unsigned int a = nn(0, 0);
                                       int is_max = 1;
                                       is_max &= a > nn(-1, -1);
                                       is_max &= a > nn(-1, 0);
                                       is_max &= a > nn(-1, 1);
        
                                       is_max &= a > nn(0, -1);
                                       is_max &= a > nn(0, 1);
        
                                       is_max &= a > nn(1, -1);
                                       is_max &= a > nn(1, 0);
                                       is_max &= a > nn(1, 1);
        
                                       if (is_max)
                                         local.push_back(p);
                                     }
                                     #pragma omp critical
                                     lms.insert(lms.end(), local.begin(), local.end());
                                   }
                                   return lms;
                                 });
  }

  template <typename V, typename... OPTS>
  std::vector<vint2> fast9(const image2d<V>& A,
                           int th,
                           OPTS... opts_)
  {
    auto opts = iod::D(opts_...);

    if (A.border() < 3)
      throw std::runtime_error("Image need a border of 3px at least for the FAST detector");
    
    image2d<unsigned char> mask = opts.get(_mask, image2d<unsigned char>());
    std::vector<int>* scores = opts.get(_scores, nullptr);
    int block_size = opts.get(_block_size, 10);
    int max_points_per_block = opts.get(_max_points_per_block, 5);

    if (opts.has(_local_maxima))
      return fast_detector9_local_maxima(A, th, mask, scores);
    else if (opts.has(_blockwise))
      return fast_detector9_blockwise_maxima(A, th, block_size, mask, scores);
    // else if (opts.has(_blockwise_rank))
    //   return fast_detector9_blockwise_rank(A, th, opts.block_size,
    //                                        opts.max_points_per_block,
    //                                        opts.mask, opts.scores);
    else
      return fast_detector9(A, th, mask, scores);
  }
  
}
