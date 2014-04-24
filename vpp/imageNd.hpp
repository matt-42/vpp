#ifndef VPP_IMAGENd_HPP__
# define VPP_IMAGENd_HPP__

# include <vpp/imageNd.hh>

namespace vpp
{

  template <typename V, unsigned N>
  imageNd<V, N>::imageNd(int* dims, int border)
  {
    allocate(dims, border);
  }

  template <typename V, unsigned N>
  imageNd<V, N>::imageNd(const boxNd<N>& domain_, int border)
  {
    int dims[N];

    for (unsigned i = 0; i < N; i++)
      dims[i] = domain_.size(i);

    allocate(dims, border);
  }

  template <typename V, unsigned N>
  imageNd<V, N>::~imageNd()
  {
    if (data_)
      delete[] data_;
    data_ = 0;
    begin_ = 0;
  }

  template <typename V, unsigned N>
  void imageNd<V, N>::allocate(int* dims, int border)
  {
    border_ = border;

    int size = 1;
    for (int i = 0; i < N; i++)
      size *= (dims[i] + border * 2);
    data_ = new V[size];
    data_end_ = &(data_[size]);

    vint<N> b = vint<N>::Ones();
    begin_ = &(data_[border * coords_to_index(b)]);

    domain_.p1() = vint<N>::Zero();
    for (unsigned i = 0; i < N; i++)
      domain_.p2()[i] = dims[i] - 1;
  }

  template <typename V, unsigned N>
  int imageNd<V, N>::coords_to_index(const vint<N>& p) const
  {
    int idx = p[N-1];
    int ds = 1;
    for (int i = N - 2; i >= 0; i--)
    {
      ds *= domain_.size(i + 1);
      idx += ds * p[i];
    }
    return idx;
  }

  template <typename V, unsigned N>
  V&
  imageNd<V, N>::operator()(const vint<N>& p)
  {
    assert(domain_.has(p));
    assert(&begin_[coords_to_index(p)] < data_end_);
    return begin_[coords_to_index(p)];
  }

  template <typename V, unsigned N>
  const V&
  imageNd<V, N>::operator()(const vint<N>& p) const
  {
    assert(domain_.has(p));
    assert(&begin_[coords_to_index(p)] < data_end_);
    return begin_[coords_to_index(p)];
  }


  template <typename V, unsigned N>
  V*
  imageNd<V, N>::address_of(const vint<N>& p)
  {
    return begin_ + coords_to_index(p);
  }

  template <typename V, unsigned N>
  const V*
  imageNd<V, N>::address_of(const vint<N>& p) const
  {
    return begin_ + coords_to_index(p);
  }

  template <typename V, unsigned N>
  imageNd<V, N> clone(imageNd<V, N>& img)
  {
    imageNd<V, N> n(img.domain(), img.border());
    copy(img, n);
    return n;
  }

};

#endif
