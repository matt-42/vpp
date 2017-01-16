#pragma once

namespace vpp
{

  template <typename I>
  void global_search(I& index, int query_size, int train_size,
		     D distance)
  {
    for (int i = 0; i < query_size; i++)
      index.search(
  }
}
