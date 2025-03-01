#include "serializer.hpp"

int main() {
  using just_t = decltype(ex::just());
  static_assert(ex::sender<on_serializer_sender<just_t, just_t>>);

  dummy_thread_context threads;
  ex::scheduler auto sched = threads.get_scheduler();

  std::atomic<int> counter{0};

  serializer_context ctx;
  ex::sender auto work = //
      ex::just()         //
      | ex::then([&] {
          auto c0 = ++counter;
          printf("work started\n");
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
          auto c1 = counter.load();
          if (c0 != c1)
            std::terminate();
          printf("work done\n");
        });
  ex::sender auto branch1 = on_serializer(ex::schedule(sched), ctx, work);
  ex::sender auto branch2 = on_serializer(ex::schedule(sched), ctx, work);
  ex::sender auto branch3 = on_serializer(ex::schedule(sched), ctx, work);
  ex::sync_wait(ex::when_all(branch1, branch2, branch3));

  printf("All ok.\n");
  return 0;
}