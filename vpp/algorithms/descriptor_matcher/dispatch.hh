#pragma once

//#include <iod/options.hh>
#include <iod/utils.hh>
#include <vpp/algorithms/symbols.hh>
#include <vpp/algorithms/descriptor_matcher/bruteforce_matcher.hh>
//#include <vpp/algorithms/descriptor_matcher/index1d.hh>
#include <vpp/algorithms/descriptor_matcher/distances.hh>

namespace vpp
{

  using s::_bruteforce;
  using s::_local_search;
  using s::_index1d;
  using s::_flann;
  using s::_match;
  using s::_distance;
  using s::_size1;
  using s::_size2;

  template <typename... OPTS>
  void descriptor_matcher(OPTS&&... opts)
  {
    //auto options = iod::options(opts...);
    auto options = iod::D(opts...);
    typedef decltype(options) O;
    static_assert(iod::has_symbol<O, s::_distance_t>::value, "descriptor_matcher: _distance options missing.");
    static_assert(iod::has_symbol<O, s::_match_t>::value, "descriptor_matcher: _match callback is missing.");

    constexpr bool is_bruteforce = iod::has_symbol<O, s::_bruteforce_t>::value;
    constexpr bool is_index1d = iod::has_symbol<O, s::_index1d_t>::value;
    constexpr bool is_local_search = iod::has_symbol<O, s::_local_search_t>::value;
    constexpr bool is_flann = iod::has_symbol<O, s::_flann_t>::value;
    static_assert(is_bruteforce || is_index1d, "descritor_matcher: Missing matching strategy. Use _bruteforce or _index1d.");

    // Index strategy selection.
    auto index = iod::static_if<is_bruteforce>
      ([&] (auto o) { return bruteforce_matcher_caller; }, // Bruteforce
       [&] (auto) {

         // Index1d.
         return iod::static_if<is_index1d>
         ([&] (auto o) { return index1d(o.index1d); }, // Index1d
          [] (auto) {

            // Flann
            // return iod::static_if<is_flann>
            // ([&] (auto o) { return flann_matcher(opts...); }, // Flann
            //  [] (auto) {}, options);
            
          }, options);
         
       }, options);

    // Search.
    auto F = iod::static_if<is_local_search>
      ([&] (auto o) { return local_search(index, o.local_search); },
       [] (auto o) { return global_search(index); },
       options);

  }

  
}

  
