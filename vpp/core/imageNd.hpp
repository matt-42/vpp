#ifndef VPP_IMAGENd_HPP__
# define VPP_IMAGENd_HPP__

# include <vpp/core/imageNd.hh>
# include <vpp/core/boxNd.hh>
# include <vpp/core/copy.hh>

namespace vpp
{

  template <typename V, unsigned N>
  imageNd<V, N>::imageNd()
  {
  }

  template <typename V, unsigned N>
  imageNd<V, N>::imageNd(const std::initializer_list<int>& dims, vpp::border b)
  {
    allocate(dims, b);
  }

  template <typename V, unsigned N>
  imageNd<V, N>::imageNd(const std::vector<int>& dims, vpp::border b)
  {
    allocate(dims, b);
  }


  template <typename V, unsigned N>
  imageNd<V, N>::imageNd(int ncols, vpp::border b)
    : imageNd({ncols}, b)
  {
    static_assert(N == 1, "ImageNd constructor: bad dimension.");
  }

  template <typename V, unsigned N>
  imageNd<V, N>::imageNd(int nrows, int ncols, vpp::border b)
    : imageNd({nrows, ncols}, b)
  {
    static_assert(N == 2, "ImageNd constructor: bad dimension.");
  }

  template <typename V, unsigned N>
  imageNd<V, N>::imageNd(int nslices, int nrows, int ncols, vpp::border b)
    : imageNd({nslices, nrows, ncols}, b)
  {
    static_assert(N == 3, "ImageNd constructor: bad dimension.");
  }

  template <typename V, unsigned N>
  imageNd<V, N>::imageNd(std::vector<int> dims, vpp::border b, V* data, int pitch, bool own_data)
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

    ptr_->border_ = b.size();

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
  imageNd<V, N>::imageNd(const boxNd<N>& domain, vpp::border b)
  {
    std::vector<int> dims(N);

    for (int i = 0; i < int(N); i++)
      dims[i] = domain.size(i);

    allocate(dims, b);
  }

  template <typename V, unsigned N>
  imageNd<V, N>::~imageNd()
  {
    // The shared_ptr this_->ptr_ will destroy the data if needed.
  }

  template <typename V, unsigned N>
  void imageNd<V, N>::allocate(const std::vector<int>& dims, vpp::border b)
  {
    typedef vint16 align_type;
    const int align_size = sizeof(align_type);

    ptr_ = std::make_shared<imageNd_data<V, N>>();
    auto& d = *ptr_;
    d.own_data_ = true;
    d.border_ = b.size();

    int border_size = d.border_ * sizeof(V);
    int border_padding = 0;
    if (border_size % align_size)
    {
      border_padding = align_size - (border_size % align_size);
      border_size += border_padding;
    }

    d.pitch_ = dims[N - 1] * sizeof(V) + border_size * 2;
    if (d.pitch_ % 16) d.pitch_ += 16 - (d.pitch_ % 16);

    int size = 1;
    for (int i = 0; i < N - 1; i++)
      size *= (dims[i] + b.size() * 2);
    size *= d.pitch_;
    d.data_ = (V*)(new align_type[1 + (size) / align_size]);

    if (long(d.data_) % align_size)
    {
      d.data_ = (V*)(((char*)d.data_) + align_size - long(d.data_) % align_size);
    }

    d.data_end_ = d.data_ + size / sizeof(V);
    assert(!(long(d.data_) % align_size));
    d.domain_.p1() = vint<N>::Zero();
    for (unsigned i = 0; i < N; i++)
      d.domain_.p2()[i] = dims[i] - 1;

    // Set begin_, the address of the first pixel.
    vint<N> p = vint<N>::Ones() * d.border_;
    d.begin_ = (V*)((char*)d.data_ + border_padding + coords_to_offset(p));

  }

  // template <typename V, unsigned N>
  // int imageNd<V, N>::coords_to_index(const vint<N>& p) const
  // {
  //   int idx = p[N-1];
  //   int ds = 1;
  //   for (int i = N - 2; i >= 0; i--)
  //   {
  //     ds *= ptr_->domain_.size(i + 1);
  //     idx += ds * p[i];
  //   }
  //   return idx;
  // }

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
    assert(domain_with_border().has(p));
    V* addr = (V*)((char*)ptr_->begin_ + coords_to_offset(p));
    assert(addr < ptr_->data_end_);
    return *addr;
  }

  template <typename V, unsigned N>
  const V&
  imageNd<V, N>::operator()(const vint<N>& p) const
  {
    assert(domain_with_border().has(p));
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
  I clone_with_border(I img, vpp::border b)
  {
    I n(img.domain(), b);
    copy(img, n);
    return n;
  }

};

#endif
