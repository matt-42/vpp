#ifndef VPP_WINDOW_HPP__
# define VPP_WINDOW_HPP__

# include <vpp/imageNd.hh>

namespace vpp
{

  template <typename V>
  struct window_runner
  {
    inline window_runner(const std::vector<int>& offsets, V& pix)
      : offsets_(offsets),
        pix_((char*)&pix)
    {
    }

    template <typename F>
    inline void operator<(F f)
    {
      for (auto off : offsets_)
        f(*(V*)(pix_ + off));
    }

    const std::vector<int>& offsets_;
    char* pix_;
  };

  template <typename I>
  class window
  {
  public:
    typedef typename I::coord_type coord_type;
    typedef typename I::value_type value_type;
    typedef typename I::iterator I_iterator;
    window(const I& img, const std::vector<coord_type>& cds)
    {
      for (auto p : cds)
        offsets_.push_back(img.offset_of(p));
    }

    inline window_runner<value_type> operator()(value_type& p) const
    {
      return window_runner<value_type>(offsets_, p);
    }

  private:
    std::vector<int> offsets_;
  };

  template <typename I>
  window<I> make_window(const I& img, const std::vector<typename I::coord_type>& cds)
  {
    return window<I>(img, cds);
  }

};

#endif
