#ifndef VPP_VECTOR_HH__
# define VPP_VECTOR_HH__

# include <Eigen/Core>

namespace vpp
{

  template <typename T, unsigned N>
  using vector = Eigen::Matrix<T, N, 1>;

#define VPP_ALIAS_DECL(T1, T2)                  \
  template <unsigned N>                         \
  using v##T2 = vector<T1, N>;               \
  typedef v##T2<1> v##T2##1;                    \
  typedef v##T2<2> v##T2##2;                    \
  typedef v##T2<3> v##T2##3;                    \
  typedef v##T2<4> v##T2##4; \
  typedef v##T2<8> v##T2##8; \
  typedef v##T2<16> v##T2##16;

  VPP_ALIAS_DECL(char, char);
  VPP_ALIAS_DECL(short, short);
  VPP_ALIAS_DECL(int, int);
  VPP_ALIAS_DECL(float, float);
  VPP_ALIAS_DECL(double, double);

  VPP_ALIAS_DECL(unsigned char, uchar);
  VPP_ALIAS_DECL(unsigned short, ushort);
  VPP_ALIAS_DECL(unsigned int, uint);

#undef VPP_ALIAS_DECL

  template <typename T>
  struct plus_promotion_
  {
    typedef decltype(T() + T()) type;
  };

  template <typename X, int N>
  struct plus_promotion_<Eigen::Matrix<X, N, 1>>
  {
    typedef Eigen::Matrix<decltype(X() + X()), N, 1> type;
  };

  template <typename T>
  using plus_promotion = typename plus_promotion_<T>::type;

  template <typename U, typename T>
  U cast(const T& t) { return t; }

  template <typename U, typename V, unsigned N>
  U cast(const vector<V, N>& t) { return t.cast<decltype(U()[0])>(); }

};

#endif
