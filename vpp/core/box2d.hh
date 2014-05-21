#ifndef VPP_BOX2D_HH__
# define VPP_BOX2D_HH__

namespace vpp
{

  class box2d
  {
  public:

    box2d(vint2 a, vint2 b);

  private:

    vint2 first_;
    vint2 last_;
  };


};

#endif
