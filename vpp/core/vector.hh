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

  // Promote a type to avoid addition overflow. Scalar version.
  template <typename T>
  struct plus_promotion_
  {
    typedef decltype(T() + T()) type;
  };

  // Promote a type to avoid addition overflow. Vector version.
  template <typename X, int N>
  struct plus_promotion_<Eigen::Matrix<X, N, 1>>
  {
    typedef Eigen::Matrix<decltype(X() + X()), N, 1> type;
  };

  // plus_promotion_ helper.
  template <typename T>
  using plus_promotion = typename plus_promotion_<T>::type;


  // Cast a value from V to U. Scalar version.
  template <typename U, typename V, typename Enable = void>
  struct cast_
  {
    static U run(const V& v) { return v; }
  };

  // Cast an Eigen matrix to U.
  template <typename U, typename D>
  struct cast_<U, Eigen::MatrixBase<D>, typename std::enable_if<!std::is_scalar<U>::value>::type
               >
  {
    static U run(const Eigen::MatrixBase<D>& v) {
      return v.template cast<typename U::Scalar>();
    }
  };

  // Cast an Eigen matrix of size 1 to U. Handle convertion to scalar types likes float or int.
  template <typename U, typename D>
  struct cast_<U, Eigen::MatrixBase<D>, typename std::enable_if<Eigen::MatrixBase<D>::SizeAtCompileTime == 1 &&
                                                                std::is_scalar<U>::value>::type>
  {
    static U run(const Eigen::MatrixBase<D>& v) {
      return U(v[0]);
    }
  };

  // Cast a scalar to an Eigen matrix of size 1.
  template <typename US, typename V>
  struct cast_<vector<US, 1>, V,
               typename std::enable_if<std::is_scalar<V>::value>::type>
  {
    static vector<US, 1> run(const V& v) {
      vector<US, 1> res;
      res[0] = v;
      return res;
    }
  };

  // template <typename VS, typename US>
  // struct cast_<vector<US, 1>, vector<VS, 1>>
  // {
  //   static vector<US, 1> run(const vector<VS, 1>& v) {
  //     vector<US, 1> res;
  //     res[0] = v[0];
  //     return res;
  //   }
  // };

  // Cast helper.
  template <typename U, typename V>
  typename std::enable_if<!std::is_base_of<Eigen::MatrixBase<V>, V>::value, U>::type
  cast(const V& v) { return cast_<U, V>::run(v); }

  template <typename U, typename V>
  typename std::enable_if<std::is_base_of<Eigen::MatrixBase<V>, V>::value, U>::type
  cast(const V& v) { return cast_<U, Eigen::MatrixBase<V>>::run(v); }

  // template <typename U, typename V>
  // typename std::enable_if<std::is_base_of<Eigen::MatrixBase<U>, U>::value, U>::type
  // cast(const V& v) { return cast_<Eigen::MatrixBase<U>, V>::run(v); }

};

#endif
