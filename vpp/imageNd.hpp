#ifndef VPP_IMAGENd_HPP__
# define VPP_IMAGENd_HPP__

# include <vpp/imageNd.hh>

namespace vpp
{

  template <typename V, unsigned N>
  imageNd<V, N>::imageNd()
  {
  }

  template <typename V, unsigned N>
  imageNd<V, N>::imageNd(int* dims, int border)
  {
    allocate(dims, border);
  }

  template <typename V, unsigned N>
  imageNd<V, N>::imageNd(int* dims, int border, V* data, int pitch, bool own_data)
  {
    data_ = data;
    begin_ = data_;
    pitch_ = pitch;
    own_data_ = own_data;
    domain_.p1() = vint<N>::Zero();
    for (unsigned i = 0; i < N; i++)
      domain_.p2()[i] = dims[i] - 1;

    border_ = border;

    int size = pitch_;
    for (int n = 0; n < N - 1; n++)
      size *= domain_.size(n);

    data_end_ = (V*)((char*) data_ + size);

  }

  template <typename V, unsigned N>
  imageNd<V, N>::imageNd(imageNd<V, N>&& other)
  {
    *this = other;
  }


  template <typename V, unsigned N>
  imageNd<V, N>& imageNd<V, N>::operator=(imageNd<V, N>& other)
  {
    data_ = other.data_;
    data_end_ = other.data_end_;
    begin_ = other.begin_;
    domain_ = other.domain_;
    border_ = other.border_;
    pitch_ = other.pitch_;
    own_data_ = false;
    return *this;
  }

  template <typename V, unsigned N>
  imageNd<V, N>::imageNd(imageNd<V, N>& other)
  {
    *this = other;
    own_data_ = false;
  }


  template <typename V, unsigned N>
  imageNd<V, N>& imageNd<V, N>::operator=(imageNd<V, N>&& other)
  {
    data_ = other.data_;
    data_end_ = other.data_end_;
    begin_ = other.begin_;
    domain_ = other.domain_;
    border_ = other.border_;
    pitch_ = other.pitch_;
    own_data_ = other.own_data_;

    other.data_ = 0;

    return *this;
  }

  template <typename V, unsigned N>
  imageNd<V, N>::imageNd(const boxNd<N>& domain, int border)
  {
    int dims[N];

    for (int i = 0; i < N; i++)
      dims[i] = domain.size(i);

    allocate(dims, border);
  }

  template <typename V, unsigned N>
  imageNd<V, N>::~imageNd()
  {
    if (data_ and own_data_)
    {
      std::cout << "!!!!!!!!!!!!!!!!!!! !!!!!!!!!!!!!!!! Free " << data_ << std::endl;
      delete[] data_;
    }
    data_ = 0;
    begin_ = 0;
  }

  template <typename V, unsigned N>
  void imageNd<V, N>::allocate(int* dims, int border)
  {
    own_data_ = true;
    border_ = border;
    pitch_ = dims[N - 1] * sizeof(V);
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
  int imageNd<V, N>::coords_to_offset(const vint<N>& p) const
  {
    int row_idx = p[N-2];
    int ds = 1;
    for (int i = N - 3; i >= 0; i--)
    {
      ds *= domain_.size(i + 1);
      row_idx += ds * p[i];
    }
    return row_idx * pitch_ + p[N - 1] * sizeof(V);
  }

  template <typename V, unsigned N>
  V&
  imageNd<V, N>::operator()(const vint<N>& p)
  {
    assert(domain_.has(p));
    V* addr = (V*)((char*)begin_ + coords_to_offset(p));
    assert(addr < data_end_);
    return *addr;
  }

  template <typename V, unsigned N>
  const V&
  imageNd<V, N>::operator()(const vint<N>& p) const
  {
    assert(domain_.has(p));
    const V* addr = (V*)((char*)begin_ + coords_to_offset(p));
    assert(addr < data_end_);
    return *addr;
  }


  template <typename V, unsigned N>
  V*
  imageNd<V, N>::address_of(const vint<N>& p)
  {
    return (V*)((char*)(begin_) + coords_to_offset(p));
  }

  template <typename V, unsigned N>
  const V*
  imageNd<V, N>::address_of(const vint<N>& p) const
  {
    return (V*)((char*)(begin_) + coords_to_offset(p));
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
