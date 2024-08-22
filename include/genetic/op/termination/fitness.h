#pragma once

namespace dp::genetic {
    namespace details {
        // todo:: add customization for comparison method
        struct fitness_termination_criteria_op {
            double target_fitness{};
            template <typename T>
            bool operator()(T, double fitness) {
                return fitness >= target_fitness;
            }
        };
    }  // namespace details

    /**
     * @brief Fitness termination criteria for case where fitness is greater than or equal to target
     * fitness.
     */
    using fitness_termination = details::fitness_termination_criteria_op;

}  // namespace dp::genetic
