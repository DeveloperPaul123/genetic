#pragma once

namespace dp::genetic {
    // TODO: constraint args to be mutation operators?
    template <typename... Args>
    struct composite_mutator {
      private:
        // list of mutators
        std::tuple<Args...> mutators_;
        // https://stackoverflow.com/questions/75039429/chaining-callables-in-c
        /// @brief Helper to help us chain mutator calls together
        template <size_t index, class Arg>
        decltype(auto) call_helper(Arg&& arg) {
            if constexpr (index == 0) {
                return std::forward<Arg>(arg);
            } else {
                return std::get<index - 1>(mutators_)(
                    call_helper<index - 1>(std::forward<Arg>(arg)));
            }
        }

      public:
        explicit composite_mutator(Args&&... args) : mutators_(std::forward<Args>(args)...) {}
        template <typename T>
        T operator()(T t) {
            return call_helper<sizeof...(Args)>(t);
        }
    };
}  // namespace dp::genetic