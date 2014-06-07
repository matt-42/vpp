#include <iostream>
#include <vpp/core/tuple_utils.hh>

int sum(int a, int b)
{
  std::cout << a + b << std::endl;
}

int main()
{
  using namespace vpp;

  auto t1 = std::make_tuple(1, 2);

  internals::apply_args(t1, sum);
  internals::apply_args_transform(t1, sum, [] (int x) { return x + 1; } );

}
