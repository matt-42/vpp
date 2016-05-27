
namespace vpp
{
  template <typename V>
  std::vector<vint2> miel_detector_blockwise(const image2d<V>& I,
                                             int th,
                                             int bs)
  {
    std::vector<vint2> kps;

//     int nr = I.nrows();
//     int nc = I.ncols();

//     #pragma omp parallel for
//     for (int br = 0; br < nr; br += bs)
//     for (int bc = 0; bc < nc; bc += bs)
//     {

//       int has_keypoints = 0;      
// //#pragma omp simd reduction(|:has_keypoints)
//       for (int r = br; r < br + bs; r++)
//       {
//         for (int c = br; c < bc + bs; c++)
//           has_keypoints |= std::abs(2 * I[r][c] - I[r][c + 3] - I[r][c - 3]) > th;
//       }

//       if (has_keypoints)
//       {
//         #pragma omp barier
//         kps.push_back(vint2(br, bc));
//       }
//     }

    image2d<int> s(I.domain());
    
    auto N = const_box_nbh2d<V, 7, 7>(I);
    pixel_wise(N, I.domain()) | [&] (auto n, vint2 p)
    {
      int v2 = 2 * n(0,0);
      int saillance = std::abs(v2 - n(0, - 3) - n(0, + 3));
      // saillance = std::min(saillance, std::abs(v2 - n(r + 3, c + 1) - n(r - 3, c - 1)));
      // saillance = std::min(saillance, std::abs(v2 - n(r + 3, c - 1) - n(r - 3, c + 1)));
      // saillance = std::min(saillance, std::abs(v2 - n(r + 2, c + 2) - n(r - 2, c - 2)));
      // saillance = std::min(saillance, std::abs(v2 - n(r + 2, c - 2) - n(r - 2, c + 2)));
      // saillance = std::min(saillance, std::abs(v2 - n(r + 1, c + 3) - n(r - 1, c - 3)));
      // saillance = std::min(saillance, std::abs(v2 - n(r + 1, c - 3) - n(r - 1, c + 3)));
      if (saillance > th)
        kps.push_back(p);
      //s(p) = saillance;
//       if (saillance > th)
//       {
// #pragma omp barier
//         kps.push_back(vint2(0, 0));
//       }      
    };
//     block_wise(vint2(bs, bs), input, input.domain()) | [&] (const image2d<V>& B,
//                                                             box2d block_box)
//     {
//       //image2d<unsigned short> kps_img(B.nrows(), B.ncols());
//       int nr = B.nrows();
//       int nc = B.ncols();
//       int has_keypoints = 0;
//       for (int r = 0; r < nr; r++)
//       {
// //#pragma omp simd reduction(has_keypoints:|)
//         for (int c = 0; c < nc; c++)
//           has_keypoints |= std::abs(2 * B[r][c] - B[r][c + 3] - B[r][c - 3]) > th;
//       }

// //       if (has_keypoints)
// //       {
// //         for (int r = 0; r < nr; r++)
// //         {
// // #pragma omp simd
// //           for (int c = 0; c < nc; c++)
// //           {
// //             auto v2 = 2 * B[r][c];
            
// //             int saillance = std::abs(v2 - B[r + 3][c] - B[r - 3][c]);
// //             saillance = std::min(saillance, std::abs(v2 - B[r + 3][c + 1] - B[r - 3][c - 1]));
// //             saillance = std::min(saillance, std::abs(v2 - B[r + 3][c - 1] - B[r - 3][c + 1]));
// //             saillance = std::min(saillance, std::abs(v2 - B[r + 2][c + 2] - B[r - 2][c - 2]));
// //             saillance = std::min(saillance, std::abs(v2 - B[r + 2][c - 2] - B[r - 2][c + 2]));
// //             saillance = std::min(saillance, std::abs(v2 - B[r + 1][c + 3] - B[r - 1][c - 3]));
// //             saillance = std::min(saillance, std::abs(v2 - B[r + 1][c - 3] - B[r - 1][c + 3]));
// //             kps_img[r][c] = saillance;
            
// //           }
// //         }

// //         V vmax = kps_img[0][0];
// //         vint2 p(0, 0);
// //         for (int r = 0; r < nr; r++)
// //           for (int c = 0; c < nc; c++)
// //           {
// //             if (kps_img[r][c] > vmax)
// //             {
// //               vmax = kps_img[r][c];
// //               p = vint2(r, c);
// //             }

// // //             if (kps_img[r][c] > th)
// // //             {
// // // //#pragma omp barier
// // // //              kps.push_back(block_box.p1() + vint2(r, c));
// // //             }
// //           }

// //         if (vmax > th)
// //         {
// // #pragma omp barier
// //           kps.push_back(block_box.p1() + p);
// //         }
//       // }
//     };

    return kps;
  }
}
