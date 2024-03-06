#pragma once
#include <ranges>

#include "genetic/details/concepts.h"
#include "genetic/op/selection/roulette_selection.h"

namespace dp::genetic {
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

            roulette_selection selection{};
            // use roulette selection for the rest
            return selection(reverse_view, rank_fitness_op);
        }
    };
}  // namespace dp::genetic