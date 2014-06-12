#ifndef VPP_CORE_KEYPOINT_TRAJECTORY_HH_
# define VPP_CORE_KEYPOINT_TRAJECTORY_HH_

# include <deque>
# include <vpp/core/vector.hh>

namespace vpp
{

  struct keypoint_trajectory
  {
    keypoint_trajectory() {}
    keypoint_trajectory(vfloat2 pos) { history.push_back(pos); }

    void die()
    {
      history.clear();
    }

    vfloat2 position() const
    {
      assert(age() > 0);
      return history.front();
    }

    int size() const
    {
      return history.size();
    }

    void move_to(vfloat2 p)
    {
      history.push_front(p);
    }

    vfloat2 operator[](unsigned i) const { return history[i]; }

  private:
    std::deque<vfloat2> history;
  };

};

#endif
