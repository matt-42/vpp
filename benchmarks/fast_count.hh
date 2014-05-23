

inline bool Find9Jeremy(uint code32)
{
  uint64_t code48 = code32;
  code48 |= code48 << 32;
  code48 &= code48 << 2;
  code48 &= code48 << 2;
  code48 &= code48 << 2;
  code48 &= code48 << 2;
  code48 &= code48 << 2;
  code48 &= code48 << 2;
  code48 &= code48 << 2;
  return code48 & (code48 << 2);
}


// "mov ebx, eax"
// "rol ebx, 2" : "=r" (x) )
// and eax, ebx
// mov ebx, eax
// rol ebx, 2
// and eax, ebx
// mov ebx, eax
// rol ebx, 2
// and eax, ebx
// mov ebx, eax
// rol ebx, 2
// and eax, ebx
// mov ebx, eax
// rol ebx, 2
// and eax, ebx
// mov ebx, eax
// rol ebx, 2
// and eax, ebx
// mov ebx, eax
// rol ebx, 2
// and eax, ebx

inline bool Find9Opt(uint code)
{
  const ulong Mask1 = 0xAAAAAAAAAAAAAAAA;
  const ulong Mask2 = 0x5555555555555555;
  int count = 0;
  ulong mask = code;
  mask += mask << 32;

  while ((mask & Mask1) != 0 || (mask & Mask2) != 0)
  {
    mask = (mask & (mask << 2));
    count++;
    if (count > 8)
      return true;
  }
  return false;
}

inline int fast_count_pixel2(unsigned int code)
{
  // unsigned int T1 = 0b00000000100000001000000010000000;
  // unsigned int T2 = 0b10000000000000001000000010000000;
  // unsigned int T3 = 0b10000000100000000000000010000000;
  // unsigned int T4 = 0b10000000100000001000000000000000;

  // unsigned int T5 = 0b00000000010000000100000001000000;
  // unsigned int T6 = 0b01000000000000000100000001000000;
  // unsigned int T7 = 0b01000000010000000000000001000000;
  // unsigned int T8 = 0b01000000010000000100000000000000;


  // if ((code & T1) != T1 &&
  //     (code & T2) != T2 &&
  //     (code & T3) != T3 &&
  //     (code & T4) != T4 &&

  //     (code & T5) != T5 &&
  //     (code & T6) != T6 &&
  //     (code & T7) != T7 &&
  //     (code & T8) != T8
  //     ) return 0;

  unsigned int it = code;
  char prev = it & 3;
  char max_n = 0;
  char n = prev != 0;
  it >>= 2;
  // std::cout << "v " <<  int(prev) << std::endl;
  for (int i = 1; i < 16; i++)
  {
    char v = it & 3;
    // std::cout << "v " <<  int(v) << std::endl;
    if (v && v == prev)
      n++;
    else
    {
      max_n = max_n > n ? max_n : n;
      n = 1;
    }
    prev = v;
    it >>= 2;
    //if (n + (16 - n)
    // std::cout << "    n " <<  int(n) << std::endl;
  }
  return (max_n > n ? max_n : n) >= 9;
}

inline int fast_count_pixel3(unsigned int code)
{
  unsigned int T1 = 0b10000000100000000000000000000000;
  unsigned int T2 = 0b00000000100000001000000000000000;
  unsigned int T3 = 0b00000000000000001000000010000000;
  unsigned int T4 = 0b10000000000000000000000010000000;

  unsigned int T5 = 0b01000000010000000000000000000000;
  unsigned int T6 = 0b00000000010000000100000000000000;
  unsigned int T7 = 0b00000000000000000100000001000000;
  unsigned int T8 = 0b01000000000000000000000001000000;

  if ((code & T1) != T1 &&
      (code & T2) != T2 &&
      (code & T3) != T3 &&
      (code & T4) != T4 &&

      (code & T5) != T5 &&
      (code & T6) != T6 &&
      (code & T7) != T7 &&
      (code & T8) != T8
      ) return 0;

  unsigned int mask1 = 0b10101010101010101000000000000000;
  unsigned int mask2 = 0b01010101010101010100000000000000;

  for (int i = 0; i < 16; i++)
  {
    if ((code & mask1) == mask1
        || (code & mask2) == mask2) return 1;
    code = (code >> 2) + ((code & 3) << 30);
  }
  return 0;
}

inline int fast_count_pixel(unsigned int code)
{
  int i = 0;
  int n = 0;
  int max_n = 0;
  int cur = 0;
  unsigned int it = code;
  int first_type = 0;
  int first_n = 0;
  for (i = 0; i < 16; i++)
  {
    int v = it & 3;
    if (!v)
    {
      max_n = std::max(n, max_n);
      n = 0;
    }
    else
    {
      if (v == cur) n++;
      else
      {
        if (!first_n)
        {
          first_n = n;
        }
        max_n = std::max(n, max_n);
        n = 1;
        cur = v;
      }
    }
    it >>= 2;
  }
  max_n = std::max(n, max_n);
  return max_n;
}
