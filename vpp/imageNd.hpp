#ifndef VPP_IMAGENd_HPP__
# define VPP_IMAGENd_HPP__

# include <vpp/imageNd.hh>
# include <vpp/copy.hh>

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
    ptr_ = std::shared_ptr<imageNd_data<V, N>>(new imageNd_data<V, N>(),
                               delete_imageNd_data<imageNd_data<V, N>>);
    ptr_->data_ = data;
    ptr_->begin_ = ptr_->data_;
    ptr_->pitch_ = pitch;
    ptr_->own_data_ = own_data;
    ptr_->domain_.p1() = vint<N>::Zero();
    for (unsigned i = 0; i < N; i++)
      ptr_->domain_.p2()[i] = dims[i] - 1;

    ptr_->border_ = border;

    int size = ptr_->pitch_;
    for (int n = 0; n < N - 1; n++)
      size *= ptr_->domain_.size(n);

    ptr_->data_end_ = (V*)((char*) ptr_->data_ + size);
  }

  template <typename V, unsigned N>
  imageNd<V, N>::imageNd(const imageNd<V, N>&& other)
  {
    *this = other;
  }


  template <typename V, unsigned N>
  imageNd<V, N>& imageNd<V, N>::operator=(const imageNd<V, N>& other)
  {
    ptr_ = other.ptr_;
    return *this;
  }

  template <typename V, unsigned N>
  imageNd<V, N>::imageNd(const imageNd<V, N>& other)
  {
    *this = other;
  }


  template <typename V, unsigned N>
  imageNd<V, N>& imageNd<V, N>::operator=(const imageNd<V, N>&& other)
  {
    ptr_ = other.ptr_;
    return *this;
  }

  template <typename V, unsigned N>
  imageNd<V, N>::imageNd(const boxNd<N>& domain, int border)
  {
    int dims[N];

    for (int i = 0; i < int(N); i++)
      dims[i] = domain.size(i);

    allocate(dims, border);
  }

  template <typename V, unsigned N>
  imageNd<V, N>::~imageNd()
  {
    // The shared_ptr this_->ptr_ will destroy the data if needed.
  }

  template <typename V, unsigned N>
  void imageNd<V, N>::allocate(int* dims, int border)
  {
    ptr_ = std::make_shared<imageNd_data<V, N>>();
    auto& d = *ptr_;
    d.own_data_ = true;
    d.border_ = border;
    d.pitch_ = (dims[N - 1] + border * 2) * sizeof(V);
    int size = 1;
    for (int i = 0; i < N; i++)
      size *= (dims[i] + border * 2);
    d.data_ = new V[size];
    d.data_end_ = d.data_ + size;

    d.domain_.p1() = vint<N>::Zero();
    for (unsigned i = 0; i < N; i++)
      d.domain_.p2()[i] = dims[i] - 1;

    vint<N> b = vint<N>::Ones() * border;
    d.begin_ = (V*)((char*)d.data_ + coords_to_offset(b));
  }

  template <typename V, unsigned N>
  int imageNd<V, N>::coords_to_index(const vint<N>& p) const
  {
    int idx = p[N-1];
    int ds = 1;
    for (int i = N - 2; i >= 0; i--)
    {
      ds *= ptr_->domain_.size(i + 1);
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
      ds *= ptr_->domain_.size(i + 1);
      row_idx += ds * p[i];
    }
    return row_idx * ptr_->pitch_ + p[N - 1] * sizeof(V);
  }

  template <typename V, unsigned N>
  V&
  imageNd<V, N>::operator()(const vint<N>& p)
  {
    assert(ptr_->domain_.has(p));
    V* addr = (V*)((char*)ptr_->begin_ + coords_to_offset(p));
    assert(addr < ptr_->data_end_);
    return *addr;
  }

  template <typename V, unsigned N>
  const V&
  imageNd<V, N>::operator()(const vint<N>& p) const
  {
    assert(ptr_->domain_.has(p));
    const V* addr = (V*)((char*)ptr_->begin_ + coords_to_offset(p));
    assert(addr < ptr_->data_end_);
    return *addr;
  }


  template <typename V, unsigned N>
  V*
  imageNd<V, N>::address_of(const vint<N>& p)
  {
    return (V*)((char*)(ptr_->begin_) + coords_to_offset(p));
  }

  template <typename V, unsigned N>
  const V*
  imageNd<V, N>::address_of(const vint<N>& p) const
  {
    return (V*)((char*)(ptr_->begin_) + coords_to_offset(p));
  }

  template <typename V, unsigned N>
  int
  imageNd<V, N>::offset_of(const vint<N>& p) const
  {
    return coords_to_offset(p);
  }

  template <typename I>
  I clone(I img)
  {
    I n(img.domain(), img.border());
    copy(img, n);
    return n;
  }


  template <typename I>
  I clone_with_border(I img, int border)
  {
    I n(img.domain(), border);
    copy(img, n);
    return n;
  }

};

#endif
