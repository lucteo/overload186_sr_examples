#include <beman/execution/execution.hpp>

namespace ex = beman::execution;

namespace detail {
template <ex::receiver Receiver>
struct just_int_op {
  int value_to_send_;
  Receiver receiver_;

  // This is an operation-state type.
  using operation_state_concept = ex::operation_state_t;

  // The actual work of the operation state.
  void start() noexcept {
    // No actual work, just send the value to the receiver.
    ex::set_value(std::move(receiver_), value_to_send_);
  }
};
} // namespace detail

struct just_int_sender {
  // The data of the sender.
  int value_to_send_;

  // This is a sender type.
  using sender_concept = ex::sender_t;

  // This sender always complete with an `int` value.
  using completion_signatures = ex::completion_signatures<ex::set_value_t(int)>;

  // No environment to provide.
  ex::empty_env get_env() const noexcept { return {}; }

  // Connect to the given receiver, and produce an operation state.
  template <ex::receiver Receiver>
  auto connect(Receiver receiver) noexcept {
    return detail::just_int_op{value_to_send_, receiver};
  }
};

auto just_int(int x) { return just_int_sender{x}; }
