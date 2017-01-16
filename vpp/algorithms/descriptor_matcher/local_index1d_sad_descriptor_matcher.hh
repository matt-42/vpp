#pragma once

#include "grid_index.hh"
#include "index1d.hh"
#include <vpp/algorithms/symbols.hh>

namespace vpp
{

  template <typename... OPTS>
  void local_index1d_sad_descriptor_matcher(OPTS... opts)
  {

    auto options = iod::D(opts...);
    typedef decltype(options) O;

    grid_index<index1d> idx(options);
    
    static_assert(iod::has_symbol<O, s::_train_t>::value, "descriptor_matcher: _train options missing.");
    static_assert(iod::has_symbol<O, s::_query_t>::value, "descriptor_matcher: _query is missing.");

    static_assert(iod::has_symbol<O, s::_train_positions_t>::value, "descriptor_matcher: _train_positions options missing.");
    static_assert(iod::has_symbol<O, s::_query_positions_t>::value, "descriptor_matcher: _query_positions options missing.");

    static_assert(iod::has_symbol<O, s::_distance_t>::value, "descriptor_matcher: _distance options missing.");
    static_assert(iod::has_symbol<O, s::_match_t>::value, "descriptor_matcher: _match callback is missing.");

    assert(options.train.size() == options.train_positions.size());
    assert(options.query.size() == options.query_positions.size());

    for (int i = 0; i < options.train.size(); i++)
      idx.index(iod::D(_idx = i,
		       _position = options.train_positions[i],
		       _descriptor = options.train[i]));

    idx.finalize();
#pragma omp parallel for
    for (int i = 0; i < options.query.size(); i++)
      {
	auto m = idx.search(iod::D(_position = options.query_positions[i],
				   _descriptor = options.query[i],
				   _idx = i),
			    options.distance);

	if (m.idx >= 0)
	  {
	    options.match(i, m.idx, m.distance);
	  }
      }

  }
  
}
