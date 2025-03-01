#include <beman/execution/execution.hpp>

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>

#include "dummy_thread_scheduler.hpp"

namespace ex = beman::execution;

struct serializer_context {

  using continuation_t = std::function<void()>;

  void enqueue(continuation_t cont) {
    {
      std::lock_guard<std::mutex> lock{bottleneck_};
      if (busy_) {
        // If we are busy, we need to enqueue the continuation.
        to_run_.push_back(std::move(cont));
        return;
      }
      // We are free; mark ourselves as busy, and execute continuation inplace.
      busy_ = true;
    }
    cont();
  }

  void on_done() {
    continuation_t cont;
    {
      std::lock_guard<std::mutex> lock{bottleneck_};
      assert(busy_);
      if (to_run_.empty()) {
        // Nothing to run next, we are done.
        busy_ = false;
        return;
      }
      // We have more work to do; extract the first continuation.
      cont = std::move(to_run_.front());
      to_run_.erase(to_run_.begin());
    }
    if (cont) {
      cont();
    }
  }

private:
  bool busy_{false};
  std::vector<continuation_t> to_run_;
  std::mutex bottleneck_;
};

namespace detail {

template <ex::receiver Receiver, ex::sender Work>
struct on_serializer_receiver {
  serializer_context& context_;
  Work work_;
  Receiver receiver_;

  // This is a receiver type.
  using receiver_concept = ex::receiver_t;

  // Called when the previous sender completes with a value.
  void set_value() noexcept {
    context_.enqueue([this] {
      ex::sender auto work_and_done = work_ | ex::then([this] { context_.on_done(); });
      auto op = ex::connect(work_and_done, std::move(receiver_));
      ex::start(op);
    });
  }
  void set_error(std::exception_ptr e) noexcept { ex::set_error(std::move(receiver_), e); }
  void set_stopped() noexcept { ex::set_stopped(std::move(receiver_)); }
};

} // namespace detail

template <ex::sender Previous, ex::sender Work>
struct on_serializer_sender {
  // The data of the sender.
  Previous previous_;
  serializer_context& context_;
  Work work_;

  // This is a sender type.
  using sender_concept = ex::sender_t;

  // This sender always complete with an empty value.
  using completion_signatures = ex::completion_signatures< //
      ex::set_value_t(),                                   //
      ex::set_error_t(std::exception_ptr),                 //
      ex::set_stopped_t()>;

  // No environment to provide.
  ex::empty_env get_env() const noexcept { return {}; }

  // Connect to the given receiver, and produce an operation state.
  template <ex::receiver Receiver>
  auto connect(Receiver receiver) noexcept {
    return ex::connect(previous_, detail::on_serializer_receiver{context_, work_, receiver});
  }
};

template <ex::sender Previous, ex::sender Work>
on_serializer_sender<Previous, Work> on_serializer(Previous prev, serializer_context& ctx,
                                                   Work work) {
  return {prev, ctx, work};
}
