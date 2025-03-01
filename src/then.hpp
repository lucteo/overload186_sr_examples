#include <beman/execution/execution.hpp>

namespace ex = beman::execution;

namespace detail {

template <ex::receiver Receiver, typename Fun>
struct then_receiver {
  Fun f_;
  Receiver receiver_;

  // This is a receiver type.
  using receiver_concept = ex::receiver_t;

  // Called when the previous sender completes with a value.
  void set_value(int value) noexcept {
    try {
      ex::set_value(std::move(receiver_), f_(value));
    } catch (...) {
      ex::set_error(std::move(receiver_), std::current_exception());
    }
  }
  void set_error(std::exception_ptr e) noexcept { ex::set_error(std::move(receiver_), e); }
  void set_stopped() noexcept { ex::set_stopped(std::move(receiver_)); }
};

} // namespace detail

template <ex::sender Previous, std::invocable<int> Fun>
struct then_sender {
  // The data of the sender.
  Previous previous_;
  Fun f_;

  // This is a sender type.
  using sender_concept = ex::sender_t;

  // This sender always complete with an `int` value.
  using completion_signatures = ex::completion_signatures< //
      ex::set_value_t(int),                                //
      ex::set_error_t(std::exception_ptr),                 //
      ex::set_stopped_t()>;

  // No environment to provide.
  ex::empty_env get_env() const noexcept { return {}; }

  // Connect to the given receiver, and produce an operation state.
  template <ex::receiver Receiver>
  auto connect(Receiver receiver) noexcept {
    return ex::connect(previous_, detail::then_receiver{f_, receiver});
  }
};

template <ex::sender Previous, std::invocable<int> Fun>
then_sender<Previous, Fun> then(Previous prev, Fun f) {
  return {prev, f};
}
