#include "then.hpp"

int main() {
  using just_t = decltype(ex::just(13));
  static_assert(ex::sender<then_sender<just_t, std::function<int(int)>>>);

  ex::sender auto work = then(ex::just(17), [](int x) -> int { return x * 2; });
  auto [r] = ex::sync_wait(work).value();
  if (r != 34)
    std::terminate();

  ex::sender auto work2 = then(ex::just_error(std::exception_ptr{}),
                               [](int x) -> int {
                                 std::terminate();
                                 return 0;
                               }) |
                          ex::upon_error([](std::exception_ptr e) -> int { return 11; });
  auto [r2] = ex::sync_wait(work2).value();
  if (r2 != 11)
    std::terminate();

  ex::sender auto work3 = then(ex::just_stopped(),
                               [](int x) -> int {
                                 std::terminate();
                                 return 0;
                               }) |
                          ex::upon_stopped([]() -> int { return 7; });
  auto [r3] = ex::sync_wait(work3).value();
  if (r3 != 7)
    std::terminate();

  printf("All ok.\n");
  return 0;
}