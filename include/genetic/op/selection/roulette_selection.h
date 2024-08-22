#pragma once
#include <algorithm>
#include <ranges>

#include "genetic/details/concepts.h"
#include "genetic/details/random_helpers.h"

namespace dp::genetic {
    /**
     * @brief Perform roulette selection on a population.
     */
    struct roulette_selection {
        template <std::ranges::range Range, typename UnaryOperator,
                  typename T = std::ranges::range_value_t<Range>,
                  typename FitnessResult = std::invoke_result_t<UnaryOperator, T>>
        std::pair<T, T> operator()(Range population, UnaryOperator fitness_op) {
            // convert our data to work with std::accumulate
            auto data = population | std::views::common;
            // generate sum
            FitnessResult sum = std::accumulate(data.begin(), data.end(), FitnessResult{},
                                                [&](FitnessResult current_sum, const T& value) {
                                                    return current_sum + fitness_op(value);
                                                });

            thread_local auto generator = uniform_floating_point_generator{};
            auto first_value = generator(0.0, 1.0);
            auto second_value = generator(0.0, 1.0);

            auto threshold1 = first_value * sum;
            auto threshold2 = second_value * sum;

            std::pair<T, T> return_pair{};

            auto first_found = false;
            auto second_found = false;

            FitnessResult accumulator{};
            for (const auto& value : population) {
                accumulator += fitness_op(value);
                if (accumulator >= threshold1 && !first_found) {
                    return_pair.first = value;
                    first_found = true;
                }
                if (accumulator >= threshold2 && !second_found) {
                    return_pair.second = value;
                    second_found = true;
                }
                if (first_found && second_found) break;
            }

            // pick 2 parents and return them
            return return_pair;
        }
    };
}  // namespace dp::genetic
