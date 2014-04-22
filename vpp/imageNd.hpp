#ifndef VPP_IMAGENd_HPP__
# define VPP_IMAGENd_HPP__

# include <vpp/imageNd.hpp>

namespace vpp
{

  template <typename V, unsigned N>
  imageNd<V, N>::imageNd(int dims[N], int border)
  {
    allocate(dims, border);
  }

  imageNd(const domainNd<N>& domain_, int border = 0);


  template <typename V, unsigned N>
  imageNd<V, N>::imageNd(imageNd<V, N>&& other)
  {
    memcpy(this, other, sizeof(*this));

    buffer_ = 
  }

  template <typename V, unsigned N>
  imageNd<V, N>::~imageNd()
  {
    assert(buffer_);
    delete[] buffer;
  }

  template <typename V, unsigned N>
  void imageNd<V, N>::allocate(int dims[N], int border)
  {
    int size = 1;
    for (int i = 0; i < N; i++) size *= (dims[i] + border * 2);
    buffer_ = new V[size];

    vintX<N> b = vintX<N>::One();
    begin_ = buffer_ + border * coords_to_index(b);
  }

  template <typename V, unsigned N>
  int imageNd<V, N>::coords_to_index(const vintX<N>& p) const
  {
    int idx = p[2];
    int ds = 1;
    for (int i = 1; i < N; i++)
    {
      ds *= domain_[N - i];
      idx += ds * p[i];
    }
    return idx;
  }

  template <typename V, unsigned N>
  V& operator()(const vintX<N>& p)
  {
    return begin_[coords_to_index(p)];
  }

  template <typename V, unsigned N>
  const V& operator()(const vintX<N>& p) const
  {
    return begin_[coords_to_index(p)];
  }

  template <typename V, unsigned N>
  imageNd<V, N> clone(imageNd& img)
  {
    imageNd<V, N> n(img.domain(), img.border());
    copy(img, n);
    return n;
  }

};

#endif
