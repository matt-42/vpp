namespace vpp
{

  inline int lbp_hamming_distance(unsigned char a, unsigned b)
  {
    unsigned char val = a ^ b;
    int dist = 0;
    while(val)
    {
      ++dist; 
      val &= val - 1; // why?
    }
    return dist;
  }

}
