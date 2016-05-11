#ifndef VPP_PROFILER_HH_
# define VPP_PROFILER_HH_

# include <cassert>
# include <iomanip>
# include <iostream>
# include <ctime>
# include <map>
# include <vector>
# include <algorithm>
# include <stack>
# include <string>

namespace vpp
{
  struct profiler_node
  {
    inline profiler_node() : duration(0), ncalls(0) {}

    unsigned long long duration;
    unsigned long long ncalls;
    std::map<std::string, profiler_node> childs;
  };

  struct profiler
  {
  public:
    profiler()
    {
      stack_.push(&root_);
    }

    inline void begin(const std::string& name);
    inline void end(const std::string& name);

    inline const profiler_node& root() const;
  private:
    inline unsigned long long current_time() const;

    profiler_node root_;

    std::stack<std::string> scope_;
    std::stack<profiler_node*> stack_;
    std::stack<long long> timers_;

  };

  unsigned long long
  profiler::current_time() const
  {
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    //clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
    //clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
  }

  void profiler::begin(const std::string& name)
  {
    scope_.push(name);
    timers_.push(current_time());
    stack_.push(&(*stack_.top()).childs[name]);
  }

  void profiler::end(const std::string& name)
  {
    assert(stack_.size());
    assert(name == scope_.top());
    stack_.top()->duration += current_time() - timers_.top();
    stack_.top()->ncalls++;

    stack_.pop();
    scope_.pop();
    timers_.pop();
  }

  const profiler_node&
  profiler::root() const
  {
    return root_;
  }

  typedef std::pair<std::string, const profiler_node*> node_pair;

  struct profiler_node_cmp {
    bool operator()(const node_pair &lhs, const node_pair &rhs) {
      return lhs.second->duration > rhs.second->duration;
    }
  };


  inline void print(std::ostream& os, const std::string& path,
                    const profiler_node& n, const std::string& indent,
                    unsigned long long total_time, unsigned long long node_time)
  {
    unsigned childs_sum = 0;
    for (std::map<std::string, profiler_node>::const_iterator it = n.childs.begin();
         it != n.childs.end(); it++)
      childs_sum += it->second.duration;

    os << std::setw(50) << std::setfill(' ') << std::left << (indent + path + ":") << std::fixed << std::setprecision(3)
       << std::setw(15) << std::setfill(' ') << std::left << n.ncalls
       << std::setw(15) << std::setfill(' ') << std::left << (n.duration) / (n.ncalls * 1000.)
       << std::setw(15) << std::setfill(' ') << std::left << (n.duration) / 1000.
       << std::setw(15) << std::setfill(' ') << std::left << (100.*n.duration/node_time)
       << std::setw(15) << std::setfill(' ') << std::left << (100.*n.duration/total_time)
       << std::setw(15) << std::setfill(' ') << std::left << (100.*(n.duration-childs_sum)/total_time);


    if (n.childs.size())
    {
      std::vector<node_pair> vec;
      for (std::map<std::string, profiler_node>::const_iterator it = n.childs.begin();
          it != n.childs.end(); it++)
        vec.push_back(node_pair(it->first, &(it->second)));

      std::sort(vec.begin(), vec.end(), profiler_node_cmp());

      for(std::vector<node_pair>::const_iterator it = vec.begin();
          it != vec.end(); it++)
      {
        os << std::endl;
        print(os, it->first, *(it->second), indent + "|    ", total_time, n.duration);
      }
    }
  }

  inline std::ostream& operator<<(std::ostream& os, const profiler& p)
  {
    os << std::left << std::setw(49) << "Sections"
       << std::left << std::setw(15) << "| #calls"
       << std::left << std::setw(15) << "| ms/calls"
       << std::left << std::setw(15) << "| time"
       << std::left << std::setw(15) << "| %P"
       << std::left << std::setw(15) << "| %T"
       << std::left << std::setw(15) << "| %I"
       << std::endl
       << std::left << std::setw(140) << std::setfill('-') << ""
       << std::endl;

    unsigned long long root_total_time = 0;
    for(std::map<std::string, profiler_node>::const_iterator it = p.root().childs.begin();
        it != p.root().childs.end(); it++)
      root_total_time += it->second.duration;

    for(std::map<std::string, profiler_node>::const_iterator it = p.root().childs.begin();
        it != p.root().childs.end(); it++)
    {
      if (it != p.root().childs.begin())
        os << std::endl;
      print(os, it->first, it->second, "", root_total_time, root_total_time);
    }

    std::cout << std::endl << "%P = 100*node_time/parent_node_time" << std::endl
              << "%T = 100*node_time/total_recorded_time" << std::endl
              << "%I = 100*(node_duration-childs_time)/total_recorded_time";
      
    return os;
  }

}

#endif
