#pragma once

#include <tuple>
#include <array>

namespace vpp
{
  template <typename... T>
  decltype(auto) make_array(T&&... t)
  {
    return std::array<std::tuple_element_t<0, std::tuple<T...> >, sizeof...(t)>
      {std::forward<T>(t)...};
  }

}
