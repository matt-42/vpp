#ifndef VPP_TUPLE_UTILS_HH_
# define VPP_TUPLE_UTILS_HH_

#include <utility>
#include <cstddef>
#include <tuple>
#include <functional>

namespace vpp
{
  namespace internals
  {

    template<std::size_t... I, class... Args, class F>
    inline void apply_args_impl(std::tuple<Args...>& t, std::integer_sequence<std::size_t, I...>, F&& f)
    {
      f(std::forward<Args>(std::get<I>(t))...);
    }
      
    template<class... Args, class F>
    inline void apply_args(std::tuple<Args...>& t, F&& f)
    {
      apply_args_impl(t, std::make_integer_sequence<std::size_t, sizeof...(Args)>{}, f);
    }
    
    
    template<std::size_t... I, class... Args, class F>
    inline void apply_args_star_impl(std::tuple<Args...>& t, std::integer_sequence<std::size_t, I...>, F&& f)
    {
      f(*std::get<I>(t)...);
    }
      
    template<class... Args, class F>
    inline void apply_args_star(std::tuple<Args...>& t, F&& f)
    {
      apply_args_star_impl(t, std::make_integer_sequence<std::size_t, sizeof...(Args)>{}, f);
    }

    template<std::size_t... I, class... Args, class F, class G>
    inline void apply_args_transform_impl(std::tuple<Args...>& t, std::integer_sequence<std::size_t, I...>,
                                          F f, G g)
    {
      f(g(std::get<I>(t))...);
    }
      
    template<class... Args, class F, class G>
    inline void apply_args_transform(std::tuple<Args...>& t, F f, G g)
    {
      apply_args_transform_impl(t, std::make_integer_sequence<std::size_t, sizeof...(Args)>{}, f, g);
    }
    

    template<typename F, size_t... I, typename... T>
    inline F tuple_map(std::tuple<T...>& t, F f, std::index_sequence<I...>)
    {
      return (void)std::initializer_list<int>{((void)f(std::get<I>(t)), 0)...}, f;
    }
    
    template<typename F, typename... T>
    inline void tuple_map(std::tuple<T...>& t, F f)
    {
      tuple_map(t, f, std::index_sequence_for<T...>{});
    }

    template<typename F, size_t... I, typename T>
    inline decltype(auto) tuple_transform(T&& t, F f, std::index_sequence<I...>)
    {
      return std::make_tuple(f(std::get<I>(std::forward<T>(t)))...);
    }

    template<typename F, typename T>
    inline decltype(auto) tuple_transform(T&& t, F f)
    {
      return tuple_transform(std::forward<T>(t), f,
                             std::make_index_sequence<std::tuple_size<std::decay_t<T>>{}>{});
    }

  }
}

#endif
