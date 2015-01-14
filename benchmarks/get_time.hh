
inline double get_time_in_seconds()
{
  timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return double(ts.tv_sec) + double(ts.tv_nsec) / 1000000000.;
}

template <typename F>
double time(F f)
{
  double t = get_time_in_seconds();

  f();
  
  return get_time_in_seconds() - t;
}
