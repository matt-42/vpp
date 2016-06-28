#ifndef VPP_CORE_KEYPOINT_TRAJECTORY_HH_
# define VPP_CORE_KEYPOINT_TRAJECTORY_HH_

# include <iostream>
# include <deque>
# include <vpp/core/vector.hh>

namespace vpp
{

  struct keypoint_trajectory
  {
    keypoint_trajectory() : alive_(true) {}
    keypoint_trajectory(int frame_cpt)
      : start_frame_(frame_cpt),
        alive_(true)
    {
    }

    void die()
    {
      alive_ = false;
    }

    bool alive()
    {
      return alive_;
    }

    vfloat2 position() const
    {
      assert(size() > 0);
      return history_.front();
    }

    int size() const
    {
      return history_.size();
    }

    vfloat2 position_at_frame(int frame_cpt)
    {
      assert(frame_cpt < (start_frame_ + history_.size()));
      assert(frame_cpt >= (start_frame_));
      int i = history_.size() - 1 - (frame_cpt - start_frame_);
      return history_[i];
    }

    void move_to(vfloat2 p)
    {
      history_.push_front(p);
    }

    void pop_oldest_position()
    {
      history_.pop_back();
    }
    
    vfloat2 operator[](unsigned i) const { return history_[i]; }

    const std::deque<vfloat2>& positions() const { return history_; }

    int start_frame() const { return start_frame_; }
    int end_frame() const { return start_frame_ + history_.size() - 1; }

  private:
    int start_frame_;
    bool alive_;
    std::deque<vfloat2> history_;
  };

};

#endif
