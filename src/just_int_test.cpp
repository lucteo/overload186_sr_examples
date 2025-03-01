#include "just_int.hpp"

int main() {
  static_assert(ex::sender<just_int_sender>);

  ex::sender auto work = just_int(13) | ex::then([](int x) -> int { return x * 2; });
  auto [r] = ex::sync_wait(work).value();
  printf("%d\n", r);
  if (r != 26)
    std::terminate();
  return 0;
}