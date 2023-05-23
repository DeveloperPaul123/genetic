#pragma once

#include <algorithm>
#include <iterator>
#include <numeric>
#include <random>
#include <ranges>
#include <utility>

#include "genetic/details/concepts.h"
#include "genetic/details/random_helpers.h"

namespace dp::genetic {
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

            static thread_local auto generator = dp::genetic::uniform_floating_point_generator{};
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

    struct rank_selection {
        template <std::ranges::range Range, typename UnaryOperator,
                  typename T = std::ranges::range_value_t<Range>,
                  typename FitnessResult = std::invoke_result_t<UnaryOperator, T>>
        std::pair<T, T> operator()(Range population, UnaryOperator fitness_op) {
            // assume population is sorted already
            auto reverse_view = population | std::views::reverse;

            // fitness evaluator/operator
            auto rank_fitness_op = [&](const T& value) -> FitnessResult {
                auto location = std::find(reverse_view.begin(), reverse_view.end(), value);
                return static_cast<FitnessResult>(
                    static_cast<double>(std::distance(reverse_view.begin(), location)) + 1.0);
            };

            // use roulette selection
            return selection_(reverse_view, rank_fitness_op);
        }

      private:
        roulette_selection selection_{};
    };
}  // namespace dp::genetic
