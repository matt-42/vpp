
int main()
{

  std::vector<vfloat16> descriptors, descriptors2;
  std::vector<vint> positions;

  std::vector<> matches;
  
  // Classical neareast neighbor search.
  match_keypoints(

    _distance = [] (int q, int t) { return (descriptors2[q] - descriptors[t]).norm(); },
    _match = [] (int q, int t, float dist) { matches.push_back({q, t, dist}); },
    
    // Only for bruteforce
    _size1 = descriptors1.size()
    _size2 = descriptors2.size()

    // For all methods except bruteforce
    _query = descriptors2, // or a lambda [] (int i) { return kps[i].descriptor; }
    _train = descriptors,
   

    _bruteforce
    _flann(_trees = 4, _nchecks = 10),
    _index1d(_approximation = 1),

    _local_search(_query_positions = positions2,
                  _train_positions = positions1,
                  _search_radius = 100)
    _
    ); 


  // Neareast neighbor search in a local neighborhood.
  local_match_keypoints_nearest(
    // Same than classical neighbor search plus:

    _query_positions = positions2,
    _train_positions = positions1,

    // Search radius.
    _search_radius = 12,
    );

}
