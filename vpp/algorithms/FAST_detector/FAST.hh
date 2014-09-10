#ifndef VPP_FAST3_DETECTOR_HH_
# define VPP_FAST3_DETECTOR_HH_

# include "immintrin.h"
# include <omp.h>
# include <bitset>
# include <thread>
# include <vpp/vpp.hh>

namespace vpp
{

  namespace FAST_internals
  {
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

    inline bool fast9_check_code_4(uint8_t code8)
    {
      uint16_t code16 = code8;
      code16 |= code16 << 8;
      return (code16 & (code16 << 2));
    }

    template <typename V>
    V* shift_row(V* p, int diff)
    {
      return (V*)(((char*)p) + diff);
    }

    template <typename V>
    inline
    int fast_score(V* row1,
                   V* row2,
                   V* row3,
                   V* row4,
                   V* row5,
                   V* row6,
                   V* row7,
                   int c,
                   int th)
    {
      V v = row4[c];
      int sum_inf = 0;
      int sum_sup = 0;

      auto f = [&] (V a) -> int {
        int diff = v - a;
        if (diff < -th) sum_inf -= diff;
        else if (diff > th) sum_sup += diff;
      };

      f(row1[c - 1]);
      f(row1[c]);
      f(row1[c + 1]);

      f(row2[c + 2]);

      f(row3[c + 3]);
      f(row4[c + 3]);
      f(row5[c + 3]);

      f(row6[c + 2]);

      f(row7[c + 1]);
      f(row7[c]);
      f(row7[c - 1]);

      f(row6[c - 2]);

      f(row5[c - 3]);
      f(row4[c - 3]);
      f(row3[c - 3]);

      f(row2[c - 2]);

      return std::max(sum_sup, sum_inf);
    }


    template <typename N>
    inline
    int fast_score_(N& nbh,
                    int c,
                    int th)
    {
      // V v = row4[c];
      typedef typename N::value_type V;
      V v = nbh.template at<0, 0>();
      int sum_inf = 0;
      int sum_sup = 0;

      auto f = [&] (V a) -> int {
        int diff = v - a;
        if (diff < -th) sum_inf -= diff;
        else if (diff > th) sum_sup += diff;
      };

      // f(row1[c - 1]);
      // f(row1[c]);
      // f(row1[c + 1]);
      f(nbh.template at<-3, -1>());
      f(nbh.template at<-3,  0>());
      f(nbh.template at<-3,  1>());

      // f(row2[c + 2]);
      f(nbh.template at<-2,  2>());

      // f(row3[c + 3]);
      // f(row4[c + 3]);
      // f(row5[c + 3]);
      f(nbh.template at<-1, 3>());
      f(nbh.template at< 0, 3>());
      f(nbh.template at< 1, 3>());

      // f(row6[c + 2]);
      f(nbh.template at<2,  2>());

      // f(row7[c + 1]);
      // f(row7[c]);
      // f(row7[c - 1]);
      f(nbh.template at<3, +1>());
      f(nbh.template at<3,  0>());
      f(nbh.template at<3, -1>());

      // f(row6[c - 2]);
      f(nbh.template at< 2, -2>());

      // f(row5[c - 3]);
      // f(row4[c - 3]);
      // f(row3[c - 3]);
      f(nbh.template at< 1, -3>());
      f(nbh.template at< 0, -3>());
      f(nbh.template at<-1, -3>());

      // f(row2[c - 2]);
      f(nbh.template at<-2, -2>());

      return std::max(sum_sup, sum_inf);
    }

#ifdef __AVX2__
    struct fast9_simd
    {
      typedef __m256i V;
      enum { size = 32, size_in_bits = 256 };

      static V check(V x, V hi, V lo)
      {
        __m256i _1 = _mm256_set1_epi8(1);
        __m256i a = _mm256_min_epu8(_1, _mm256_subs_epu8(x, hi));
        __m256i b = _mm256_min_epu8(_1, _mm256_subs_epu8(lo, x));        
        return _mm256_slli_epi16(a, 4) | b;
      };

      static V u_subs(V a, V b) { return _mm256_subs_epu8(a, b); }
      static V u_adds(V a, V b) { return _mm256_adds_epu8(a, b); }
      static bool all_equal_zero(V a) {return _mm256_testz_si256(a, _mm256_set1_epi8(255)) == 1;}
      static V repeat(int v) { return _mm256_set1_epi8(v); }
      static V load(void* ptr) { return _mm256_load_si256((const __m256i*) (ptr)); }
      static V loadu(void* ptr) { return _mm256_loadu_si256((const __m256i*) (ptr)); }
      static void storeu(void* ptr, V x) { _mm256_storeu_si256((__m256i*) (ptr), x); }
    };
#else

#  ifdef __SSE4_1__
    struct fast9_simd // SSE4 version
    {
      typedef __m128i V;
      enum { size = 16, size_in_bits = 128 };

      static V check(V x, V hi, V lo)
      {
        V _1 = _mm_set1_epi8(1);
        V a = _mm_min_epu8(_1, _mm_subs_epu8(x, hi));
        V b = _mm_min_epu8(_1, _mm_subs_epu8(lo, x));
        return _mm_slli_epi16(a, 4) | b;
      };

      static V u_subs(V a, V b) { return _mm_subs_epu8(a, b); }
      static V u_adds(V a, V b) { return _mm_adds_epu8(a, b); }
      static bool all_equal_zero(V a) { return _mm_testz_si128(a, _mm_set1_epi8(255)) == 1; }
      static V repeat(int v) { return _mm_set1_epi8(v); }
      static V load(void* ptr) { return _mm_load_si128((const V*) (ptr)); }
      static V loadu(void* ptr) { return _mm_loadu_si128((const V*) (ptr)); }
      static void storeu(void* ptr, V x) { _mm_storeu_si128((V*) (ptr), x); }
    };

#  else

#    ifdef __ARM_NEON__
#    include <arm_neon.h>
    struct fast9_simd // ARM NEON version
    {
      typedef uint16x8_t V;
      enum { size = 16, size_in_bits = 128 };

      static V check(V x, V hi, V lo)
      {
        V _1 = _mm_set1_epi8(1);
        V a = vminq_u16(_1, _mm_subs_epu8(x, hi));
        V b = vminq_u16(_1, _mm_subs_epu8(lo, x));
        return vshlq_n_u16(a, 4) | b;
      };

      static V u_subs(V a, V b) { return vqsubq_u16(a, b); }
      static V u_adds(V a, V b) { return vqaddq_u16(a, b); }
      static bool all_equal_zero(V a) { return _mm_testz_si128(a, _mm_set1_epi8(255)) == 1; }
      static V repeat(int v) { return _mm_set1_epi8(v); }
      static V load(void* ptr) { return vld1q_u16((const uint16_t*) (ptr)); }
      static V loadu(void* ptr) { return vld1q_u16((const uint16_t*) (ptr)); }
      static void storeu(void* ptr, V x) { vst1q_u16((uint16_t*) (ptr), x); }
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
      static V load(void* ptr) { return *(const V*) (ptr); }
      static V loadu(void* ptr) { return *(const V*) (ptr); }
      static void storeu(void* ptr, V x) { (*(V*) (ptr)) = x; }
    };

#    endif

#  endif
#endif

    template <typename V>
    void fast_detector9_simd(image2d<V>& A, std::vector<vint2>& keypoints, int th)
    {
      int nc = A.ncols();
      int nr = A.nrows();
      int pitch = A.pitch();

#pragma omp parallel
      {
      std::vector<vint2> local;

#pragma omp for
      for (int r = 0; r < nr; r++)
      {
        V* a_row = &A(r, 0);
        V* a_row1 = shift_row(a_row, -3*pitch);
        V* a_row2 = shift_row(a_row, -2*pitch);
        V* a_row3 = shift_row(a_row, -1*pitch);
        V* a_row4 = a_row;
        V* a_row5 = shift_row(a_row, 1*pitch);
        V* a_row6 = shift_row(a_row, 2*pitch);
        V* a_row7 = shift_row(a_row, 3*pitch);

        typedef fast9_simd S;
        typedef typename S::V v;
        for (int c = 0; c < nc - S::size; c += S::size)
        {
          v hi,lo;
          v a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15;

          {
            v here = S::load(a_row4 + c);
            v th_ = S::repeat(th);
            hi = S::u_adds(here, th_);
            lo = S::u_subs(here, th_);

          }

          v possible;
          {
            v a = S::load(a_row1 + c);
            v b = S::load(a_row7 + c);

            a0 = S::check(a, hi, lo);
            a8 = S::check(b, hi, lo);

            possible = a0 | a8;
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
            for (int i = 0; i < S::size; i++)
              if (is_corner[i]) local.push_back(vint2{r, c + i});
          }
        }

      }

#pragma omp critical
      keypoints.insert(keypoints.end(), local.begin(), local.end());
    }

    }


    template <int fullcheck, typename V, typename U>
    void fast_detector9(image2d<V>& A, image2d<U>& B, int th)
    {
      int nc = A.ncols();
      int nr = A.nrows();
      int pitch = A.pitch();

      auto n = box_nbh2d<V, 7, 7>(A);
      pixel_wise(B, n) << [&] (U& b, auto& n)
      {
        V v = n.at(0, 0);

        auto f = [&] (V a) -> int { return ((a > v + th) << 1) ^ (a < v - th); };

        uint x  =
          (f(n.at(3, -1)) << 20) +
          (f(n.at(3,  0)) << 18) +
          (f(n.at(3, +1)) << 16) +

          (f(n.at( 2, -2)) << 22) +
          (f(n.at(2,  2)) << 14) +


          (f(n.at( 1, -3)) << 24) +
          (f(n.at( 1, 3)) << 12) +
          (f(n.at( 0, -3)) << 26) +
          (f(n.at( 0, 3)) << 10) +

          (f(n.at(-1, -3)) << 28) +
          (f(n.at(-1, 3)) << 8) +

          (f(n.at(-2, -2)) << 30) +
          (f(n.at(-2,  2)) << 6) +

          f(n.at(-3, -1)) +
          (f(n.at(-3,  0)) << 2) +
          (f(n.at(-3,  1)) << 4);
  
        b = fast9_check_code(x);

      };
    }


    template <typename V, typename U>
    void fast_detector9_scores(image2d<V>& A, image2d<U>& B, int th)
    {
      int nc = A.ncols();
      int nr = A.nrows();
      int pitch = A.pitch();

#pragma omp parallel for
      for (int r = 0; r < nr; r++)
      {
        V* a_row = &A(r, 0);
        V* a_row1 = shift_row(a_row, -3*pitch);
        V* a_row2 = shift_row(a_row, -2*pitch);
        V* a_row3 = shift_row(a_row, -1*pitch);
        V* a_row4 = a_row;
        V* a_row5 = shift_row(a_row, 1*pitch);
        V* a_row6 = shift_row(a_row, 2*pitch);
        V* a_row7 = shift_row(a_row, 3*pitch);

        auto* b_row = &B(r, 0);

        #pragma omp simd aligned(a_row1, a_row2, a_row3, a_row4, a_row5, a_row6, a_row7, b_row:8 * sizeof(int))
        for (int c = 0; c < nc; c++) if (b_row[c])
        {
          b_row[c] = fast_score(a_row1,
                                a_row2,
                                a_row3,
                                a_row4,
                                a_row5,
                                a_row6,
                                a_row7,
                                c, th);
        }
      }
    }

    template <typename V, typename U>
    void fast_detector9_check4(image2d<V>& A, image2d<U>& B, int th)
    {
      int nc = A.ncols();
      int nr = A.nrows();
      int pitch = A.pitch();

#pragma omp parallel for
      for (int r = 0; r < nr; r++)
      {
        V* a_row = &A(r, 0);
        V* a_row1 = shift_row(a_row, -3*pitch);
        V* a_row4 = a_row;
        V* a_row7 = shift_row(a_row, 3*pitch);

        auto* b_row = &B(r, 0);

#pragma omp simd aligned(a_row1, a_row4, a_row7, b_row:8 * sizeof(int))
        for (int c = 0; c < nc; c++)
        {
          V v = a_row4[c];

          auto f = [&] (V a) -> int { return ((a > v + th) << 1) | (a < v - th); };

          uint8_t x  =
            (f(a_row1[c])) |
            (f(a_row4[c - 3]) << 2) |
            (f(a_row7[c]) << 4) |
            (f(a_row4[c + 3]) << 6);

          b_row[c] = fast9_check_code_4(x);
        }
      }
    }

    template <typename V>
    void fast_detector9_local_maxima(image2d<V>& A)
    {
      int nc = A.ncols();
      int nr = A.nrows();
      int pitch = A.pitch();

#pragma omp parallel for
      for (int r = 0; r < nr; r++)
      {
        V* a_row = &A(r, 0);
        V* a_row1 = shift_row(a_row, -1*pitch);
        V* a_row2 = a_row;
        V* a_row3 = shift_row(a_row, 1*pitch);

#pragma omp simd aligned(a_row1, a_row2, a_row3:8 * sizeof(int))
        for (int c = 0; c < nc; c++)
          if (a_row2[c])
          {
            V max = 0;
            max = std::max(max, a_row1[c-1]);
            max = std::max(max, a_row1[c]);
            max = std::max(max, a_row1[c+1]);

            max = std::max(max, a_row2[c-1]);
            max = std::max(max, a_row2[c+1]);

            max = std::max(max, a_row3[c-1]);
            max = std::max(max, a_row3[c]);
            max = std::max(max, a_row3[c+1]);

            if (a_row2[c] < max)
              a_row2[c] = 0;
          }
      }
    }

  }

  void fast9_blockwise_maxima(image2d<int>& A, int block_size)
  {
    int nc = A.ncols();
    int nr = A.nrows();
    int pitch = A.pitch();

    #pragma omp parallel for
    for (int r = 0; r < nr; r += block_size)
    {
      int* rows[block_size];
      for (int i = 0; i < block_size; i++)
        if (r + i < nr)
          rows[i] = &A(r + i, 0);

      for (int c = 0; c < nc; c += block_size)
      {
        // Maximum search.
        vint2 pmax;
        int vmax = 0;
        for (int br = 0; br < block_size; br++)
        for (int bc = c; bc < c + block_size; bc++)
          if (r + br < nr and bc < nc)
          {
            int v = rows[br][bc];
            //std::cout << v << std::endl;
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

  inline void make_keypoint_vector(image2d<int>& img, std::vector<vint2>& keypoints)
  {
    int n_threads = omp_get_num_threads();
    int nc = img.ncols();
    int nr = img.nrows();
#pragma omp parallel
    {
      std::vector<vint2> local;
#pragma omp for
      for (int r = 0; r < nr; r++)
      {
        auto* row = &img(r, 0);
        for (int c = 0; c < nc; c++)
          if (row[c])
            local.push_back(vint2(r, c));
      }
#pragma omp critical
        keypoints.insert(keypoints.end(), local.begin(), local.end());
    }
  }

  template <typename V>
  std::vector<vint2> fast_detector9(image2d<V>& A,
                                    int th, bool local_maxima = false,
                                    bool low_density = false, image2d<int>* _scores = 0)
  {
    image2d<int> scores;
    if (_scores) scores = *_scores;
    else scores = image2d<int>(A.domain(), border(2));

    std::vector<vint2> res;

    if (low_density)
    {
      FAST_internals::fast_detector9_check4(A, scores, th);
      FAST_internals::fast_detector9_simd(A, res, th);
    }
    else
    {
      FAST_internals::fast_detector9_simd(A, res, th);
    }

    if (local_maxima)
    {
      FAST_internals::fast_detector9_scores(A, scores, th);
      FAST_internals::fast_detector9_local_maxima(scores);
    }
    //else if (_scores) FAST_internals::fast_detector9_scores(A, scores, th);

    //make_keypoint_vector(scores, res);
    return std::move(res);
  }

  template <typename V>
  std::vector<vint2> fast_detector9_blockwise_maxima(image2d<V>& A,
                                                     int th, int block_size,
                                                     bool low_density = false,
                                                     image2d<int>* _scores = 0)
  {
    image2d<int> scores;
    if (_scores) scores = *_scores;
    else scores = image2d<int>(A.domain(), border(block_size / 2));

    if (low_density)
    {
      FAST_internals::fast_detector9_check4(A, scores, th);
      FAST_internals::fast_detector9<false>(A, scores, th);
    }
    else
      FAST_internals::fast_detector9<true>(A, scores, th);

    FAST_internals::fast_detector9_scores(A, scores, th);
    fast9_blockwise_maxima(scores, block_size);

    std::vector<vint2> res;
    make_keypoint_vector(scores, res);
    return std::move(res);
  }

  template <typename V>
  void fast9_detect(image2d<V>& A, image2d<int>& keypoints, int th,
                    bool low_density = false)
  {
    if (low_density)
    {
      FAST_internals::fast_detector9_check4(A, keypoints, th);
      FAST_internals::fast_detector9<false>(A, keypoints, th);
    }
    else
      FAST_internals::fast_detector9<true>(A, keypoints, th);
  }

  template <typename V>
  void fast9_scores(image2d<V>& A, image2d<int>& keypoints, int th)
  {
    FAST_internals::fast_detector9_scores(A, keypoints, th);
  }

  inline void fast9_filter_localmaximas(image2d<int>& keypoints)
  {
    FAST_internals::fast_detector9_local_maxima(keypoints);
  }
}

#endif
