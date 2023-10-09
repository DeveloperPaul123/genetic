#pragma once

namespace dp::genetic {
    /**
     * @brief No-op mutator, returns the value unchanged.
     */
    struct no_op_mutator {
        template <typename T>
        constexpr T operator()(const T& t) const {
            return t;
        }
    };
}  // namespace dp::genetic
