#pragma once

namespace vpp
{

  // Parameters:
  //   optimize = [] (vint2 p, vfloat2 flow) { ... return make_pair(flow, distance); }
  //     Optimize the \flow at point \p.
  //
  //   criteria = [] (vint2 p, vint2 n) { return true/false; }
  //     Tell if the flow should propagate from p to n.
  //
  //   flow = [] (vint2 p, vfloat2 flow) { }
  //     Register the new flow for point p.

  void flow_propagation(image2d<vint2>& flow,
                        image2d<int>& distance,
                        D distance)
  {
    
  }
}
