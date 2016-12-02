int main()
{

  optical_flow(i1, i2,
               _keypoints = kps,
               _flow = [] (int i, vfloat2 f, float distance) { ...});

  optical_flow(i1, i2,
               _flow = [] (vint2 p, vfloat2 f) { ... });

  epipolar_flow(i1, i2,
                _flow = [] (vint2 p, vfloat2 f) { ... });
  
}
