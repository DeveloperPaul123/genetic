#pragma once

#include <cstdint>

namespace dp::genetic {
    namespace details {
        struct generations_termination_op {
            std::uint_least64_t max_generations{1000};
            explicit generations_termination_op(std::uint_least64_t max_gens = 1000)
                : max_generations(max_gens), count_(max_gens) {}
            template <typename T>
            [[nodiscard]] constexpr bool operator()(T, double) {
                count_--;
                return count_ == 0;
            }

          private:
            std::uint_least64_t count_{max_generations};
        };
    }  // namespace details

    /**
     * @brief Termination criteria for a fixed number of generations.
     */
    using generations_termination = details::generations_termination_op;
}  // namespace dp::genetic
