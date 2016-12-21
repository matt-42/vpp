#pragma once

#include <iod/options.hh>
#include <iod/utils.hh>
#include <vpp/algorithms/symbols.hh>
#include <vpp/algorithms/descriptor_matcher/bruteforce_matcher.hh>
//#include <vpp/algorithms/descriptor_matcher/index1d.hh>
#include <vpp/algorithms/descriptor_matcher/distances.hh>

namespace vpp
{

  using s::_bruteforce;
  using s::_match;
  using s::_distance;
  using s::_size1;
  using s::_size2;

  template <typename... OPTS>
  void descriptor_matcher(OPTS&&... opts)
  {
    auto options = iod::options(opts...);
    typedef decltype(options) O;
    static_assert(iod::has_symbol<O, s::_distance_t>::value, "descriptor_matcher: _distance options missing.");
    static_assert(iod::has_symbol<O, s::_match_t>::value, "descriptor_matcher: _match callback is missing.");

    constexpr bool is_bruteforce = iod::has_symbol<O, s::_bruteforce_t>::value;
    static_assert(is_bruteforce, "descritor_matcher: Only the bruteforce matcher is available.");

    // Bruteforce.
    iod::static_if<is_bruteforce>
      ([&] (auto o) { return bruteforce_matcher(opts...); },
       [] (auto) {}, options);

  }

  
}
