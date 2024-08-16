#pragma once
#include <tuple>

namespace dp {
    namespace genetic {
        /**
         * @brief Create a "problem description" that can be passed to the solver. This function is
         * a convenience function that bundles the arguments together but also fills in gaps with missing arguments.
         *
         * @tparam Args
         * @param args
         * @return std::tuple<Args...>
         */
        template <typename... Args>
        auto make_problem_description(Args... args) -> std::tuple<Args...> {
            // TODO
            return std::tie(args...);
        }
    }  // namespace genetic

}  // namespace dp
