#pragma once
#include <cstdint>
namespace dp::genetic {
    namespace details {
        /**
         * @brief Checks if fitness changes by a significant amount within a given number of
         * generations.
         * @details Checks if previous fitness and the new fitness differ by at least some
         * threshold. If they don't, a generations count is incremented. When the generation count
         * reaches the max, the functor returns true to terminate.
         */
        struct fitness_hysteresis_op {
            double fitness_variation_threshold{1.0};
            std::uint_least64_t max_generations_between_changes{1000};
            explicit fitness_hysteresis_op(double fitness_threshold,
                                           std::uint_least64_t max_generations_between)
                : fitness_variation_threshold(fitness_threshold),
                  max_generations_between_changes(max_generations_between) {}
            template <typename T>
            [[nodiscard]] constexpr bool operator()(T, double fitness) {
                if (std::abs(previous_fitness_ - fitness) > fitness_variation_threshold) {
                    // significant change in fitness
                    previous_fitness_ = fitness;
                    // reset our count
                    count_ = 0;
                } else {
                    // fitness did not change significantly
                    count_++;
                }

                if (count_ >= max_generations_between_changes) return true;

                return false;
            }

          private:
            double previous_fitness_{};
            std::uint_least64_t count_{};
        };
    }  // namespace details

    using fitness_hysteresis = details::fitness_hysteresis_op;
}  // namespace dp::genetic
