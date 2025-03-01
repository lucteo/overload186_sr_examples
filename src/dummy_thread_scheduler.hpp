#include <beman/execution/execution.hpp>

#include <thread>
#include <vector>

namespace ex = beman::execution;

struct dummy_thread_scheduler;

struct dummy_thread_context {
  std::vector<std::thread> threads_;

  ~dummy_thread_context() {
    for (auto& t : threads_) {
      t.join();
    }
  }

  dummy_thread_scheduler get_scheduler() noexcept;
};

namespace detail {
struct dummy_thread_sender;
}

struct dummy_thread_scheduler {
  dummy_thread_context* ctx_;

  using scheduler_concept = ex::scheduler_t;

  bool operator==(const dummy_thread_scheduler&) const = default;

  detail::dummy_thread_sender schedule() const noexcept;
};

namespace detail {
template <ex::receiver Receiver>
struct dummy_thread_op {
  dummy_thread_context& ctx_;
  Receiver receiver_;

  using operation_state_concept = ex::operation_state_t;

  void start() noexcept {
    ctx_.threads_.emplace_back([this] { ex::set_value(std::move(receiver_)); });
  }
};

struct dummy_thread_sender {
  dummy_thread_context& ctx_;

  using sender_concept = ex::sender_t;

  using completion_signatures = ex::completion_signatures<ex::set_value_t()>;
  // template <typename Env>
  // my_completion_sig get_completion_signatures(Env) const {
  //   return {};
  // }

  struct env {
    dummy_thread_context* ctx_;

    auto query(ex::get_completion_scheduler_t<ex::set_value_t>) const noexcept {
      return dummy_thread_scheduler{ctx_};
    }
  };

  env get_env() const noexcept { return {&ctx_}; }

  template <ex::receiver Receiver>
  auto connect(Receiver&& receiver) noexcept {
    return dummy_thread_op{ctx_, std::forward<Receiver>(receiver)};
  }
};
} // namespace detail

inline dummy_thread_scheduler dummy_thread_context::get_scheduler() noexcept { return {this}; }
inline detail::dummy_thread_sender dummy_thread_scheduler::schedule() const noexcept {
  return {*ctx_};
}
