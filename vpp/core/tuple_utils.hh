#ifndef VPP_TUPLE_UTILS_HH_
# define VPP_TUPLE_UTILS_HH_

#include <cstddef>
#include <tuple>
#include <functional>

namespace vpp
{
  namespace internals
  {

    template<size_t argIndex, size_t argSize, class... Args, class...
             Unpacked, class F>
    inline typename std::enable_if<(argIndex == argSize),
                              void>::type apply_args_impl(std::tuple<Args...>& t, F f,
                                                          Unpacked&&... u)
    {
      f(u...); // f(std::forward<Unpacked>(u)...);
    }

    template<size_t argIndex, size_t argSize, class... Args, class...
             Unpacked, class F>
    inline typename std::enable_if<(argIndex < argSize),
      void>::type apply_args_impl(std::tuple<Args...>& t, F f,
                                  Unpacked&&... u)
    {
      apply_args_impl<argIndex + 1, argSize>(t, f, u...,
                                             std::get<argIndex>(t));
    }

    template<class... Args, class F>
    inline void apply_args(std::tuple<Args...>& t, F f)
    {
      apply_args_impl<0, sizeof...(Args)>(t, f);
    }


    template<size_t argIndex, size_t argSize, class... Args, class...
             Unpacked, class F>
    inline typename std::enable_if<(argIndex == argSize),
                                   void>::type apply_args_star_impl(std::tuple<Args...>& t, F f,
                                                          Unpacked&&... u)
    {
      f((*u)...);
    }

    template<size_t argIndex, size_t argSize, class... Args, class...
             Unpacked, class F>
    inline typename std::enable_if<(argIndex < argSize),
      void>::type apply_args_star_impl(std::tuple<Args...>& t, F f,
                                  Unpacked&&... u)
    {
      apply_args_star_impl<argIndex + 1, argSize>(t, f, u...,
                                             std::get<argIndex>(t));
    }

    template<class... Args, class F>
    inline void apply_args_star(std::tuple<Args...>& t, F f)
    {
      apply_args_star_impl<0, sizeof...(Args)>(t, f);
    }


    template<size_t argIndex, size_t argSize, class... Args, class...
             Unpacked, class F, class G>
    inline typename std::enable_if<(argIndex == argSize),
                                   void>::type apply_args_transform_impl(std::tuple<Args...>& t, F f, G g,
                                                          Unpacked&&... u)
    {
      f((g(u))...);
    }

    template<size_t argIndex, size_t argSize, class... Args, class...
             Unpacked, class F, class G>
    inline typename std::enable_if<(argIndex < argSize),
      void>::type apply_args_transform_impl(std::tuple<Args...>& t, F f, G g,
                                  Unpacked&&... u)
    {
      apply_args_transform_impl<argIndex + 1, argSize>(t, f, g, u...,
                                             std::get<argIndex>(t));
    }

    template<class... Args, class F, class G>
    inline void apply_args_transform(std::tuple<Args...>& t, F f, G g)
    {
      apply_args_transform_impl<0, sizeof...(Args)>(t, f, g);
    }

    template<unsigned N, unsigned SIZE, typename F, typename... T>
    inline typename std::enable_if<(N == SIZE), void>::type
    tuple_map_(std::tuple<T...>& t, F f)
    {
    }

    template<unsigned N, unsigned SIZE, typename F, typename... T>
    inline typename std::enable_if<(N < SIZE), void>::type
    tuple_map_(std::tuple<T...>& t, F f)
    {
      f(std::get<N>(t));
      tuple_map_<N+1, SIZE>(t, f);
    }

    template<typename F, typename... T>
    inline void tuple_map(std::tuple<T...>& t, F f)
    {
      tuple_map_<0, sizeof...(T)>(t, f);
    }



    template <typename F, typename... U>
    struct transform_tuple_elements
    {
      typedef std::tuple<std::result_of_t<F(std::add_rvalue_reference_t<U>)>...> type;
    };

    template <typename F, typename... U>
    using transform_tuple_elements_t = typename transform_tuple_elements<F, U...>::type;

    template<unsigned N, unsigned SIZE, typename R, typename F, typename... T, typename... U>
    inline
    typename std::enable_if<N == SIZE, R>::type
    tuple_map2_(std::tuple<T...>& t, F f, U&&... u) 
    {
      return std::make_tuple(f(u)...);
    }

    template<unsigned N, unsigned SIZE, typename R, typename F, typename... T, typename... U>
    inline
    typename std::enable_if<(N < SIZE), R>::type
    tuple_map2_(std::tuple<T...>& t, F f, U&&... u)
    {
      return tuple_map2_<N + 1, SIZE, R>(t, f, u..., std::get<N>(t));
    }

    template<typename F, typename... T>
    inline auto tuple_transform(std::tuple<T...>& t, F f) -> transform_tuple_elements_t<F, T...>
    {
      typedef transform_tuple_elements_t<F, T...> R;
      return tuple_map2_<0, sizeof...(T), R>(t, f);
    }

  }
}

#endif
