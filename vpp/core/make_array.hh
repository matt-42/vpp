#pragma once

#include <tuple>
#include <array>

namespace vpp
{
  template <typename T1, typename... T>
  decltype(auto) make_array(T1&& t1, T&&... t)
  {
    return std::array<T1, 1 + sizeof...(t)>
      {{std::forward<T1>(t1),
	    std::forward<T>(t)...}};
  }

}
