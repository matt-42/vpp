#pragma once

#include <vpp/algorithms/symbols.hh>

namespace vpp
{
  using namespace s;

  // Match two set of descritors.

  
  // Options:

  // For all matchers:
  //
  //   _distance = function(i, j, best_distance)
  //     compute the distance between the ith and jth descritors.
  //   _match = function(i, j, distance)
  //     called if there a match between the ith and jth descritor.
  //
  // For the bruteforce matcher only:
  //
  //   _query_size: Size of the first set
  //   _train_size: Size of the second set
  //
  // For the _flann and _index1d indexes:
  //
  //   _query = descriptors1, // or a lambda [] (int i) { return kps[i].descriptor; }
  //   _train = descriptors2,
  //
  // Methods selection:
  //
  //   _bruteforce
  //   _index1d(_approximation = 2) TODO
  //   _flann(_trees = 4, _nchecks = 50) TODO

  // Spatial indexes:
  //   _grid_index(_query_positions = positions1,
  //               _train_positions = positions2,
  //               _search_radius = 100)
  //

  template <typename... OPTS>
  void descriptor_matcher(OPTS&&... opts);

  template <typename... OPTS>
  void bruteforce_matcher(int query_size, int train_size,
			  OPTS... opts);

  template <typename... OPTS>
  void local_index1d_sad_descriptor_matcher(OPTS... opts);

  // Todo
  // template <typename D, typename E, typename... OPTS>
  // void flann_descriptor_matcher(int query_size,
  // 				int train_size,
  // 				OPTS... opts);

}

//#include "descriptor_matcher/dispatch.hh"
#include "descriptor_matcher/bruteforce_matcher.hh"
#include "descriptor_matcher/distances.hh"
#include "descriptor_matcher/local_index1d_sad_descriptor_matcher.hh"
