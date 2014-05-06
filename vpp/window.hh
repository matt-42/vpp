#ifndef VPP_WINDOW_HPP__
# define VPP_WINDOW_HPP__

# include <vpp/imageNd.hh>

namespace vpp
{

  template <typename I>
  class window_iterator
  {
  public:
    typedef typename I::value_type V;
    window_iterator(V* p, const std::vector<int>& offsets, int i)
      : offsets_(offsets),
        p_(p),
        cur_((V*)(((char*)p) + offsets_[i])),
        i_(i)
    {
    }

    inline window_iterator<I>& operator++()
    {
      i_++;
      cur_ = (V*)((char*)(p_) + offsets_[i_]);
    }

    V& operator*() { return *cur_; }
    const V& operator*() const { return *cur_; }

    int i() const { return i_; }

  private:
    const std::vector<int>& offsets_;
    V* p_;
    V* cur_;
    int i_;
  };

  template <typename I>
  bool operator==(const window_iterator<I>& a, const window_iterator<I>& b)
  {
    return a.i() == b.i();
  }

  template <typename I>
  bool operator!=(const window_iterator<I>& a, const window_iterator<I>& b)
  {
    return !(a == b);
  }

  template <typename I>
  class window_range
  {
  public:
    typedef typename I::coord_type coord_type;
    typedef typename I::value_type value_type;
    typedef typename I::iterator I_iterator;

    typedef window_iterator<I> iterator;

    window_range(value_type* p, const std::vector<int>& offsets)
      : p_(p),
        offsets_(offsets)
    {
    }

    iterator begin()
    {
      return window_iterator<I>(p_, offsets_, 0);
    }

    iterator end()
    {
      return window_iterator<I>(p_, offsets_, offsets_.size() - 1);
    }

  private:
    value_type* p_;
    const std::vector<int>& offsets_;
  };

  template <typename I>
  class window
  {
  public:
    typedef typename I::coord_type coord_type;
    typedef typename I::value_type value_type;
    typedef typename I::iterator I_iterator;
    window(const I& img, const std::initializer_list<coord_type>& cds)
    {
      for (auto p : cds) 
        offsets_.push_back(img.offset_of(p));
      offsets_.push_back(0);
    }

    window_range<I> operator()(value_type& p) const
    {
      return window_range<I>(&p, offsets_);
    }

  private:
    std::vector<int> offsets_;
  };


  template <typename I>
  window<I> make_window(const I& img, const std::initializer_list<typename I::coord_type>& cds)
  {
    return window<I>(img, cds);
  }

};

#endif
