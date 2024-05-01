#pragma once

#include <algorithm>
#include <iterator>
#include <numeric>
#include <random>
#include <ranges>
#include <utility>
#include <functional>

#include "genetic/op/selection/rank_selection.h"
#include "genetic/op/selection/roulette_selection.h"

namespace dp::genetic {

    template <std::ranges::range Population, typename SelectionOperator, typename FitnessOperator,
              typename T = std::ranges::range_value_t<Population>,
              typename FitnessResult = std::invoke_result_t<FitnessOperator, T>>
        requires concepts::fitness_operator<FitnessOperator, T> &&
                 concepts::selection_operator<SelectionOperator, T, Population, FitnessOperator>

    constexpr inline std::pair<T, T> select_parents(SelectionOperator&& selection_op,
                                                    Population&& population,
                                                    FitnessOperator&& fitness_op) {
        return std::invoke(std::forward<SelectionOperator>(selection_op),
                           std::forward<Population>(population),
                           std::forward<FitnessOperator>(fitness_op));
    }

}  // namespace dp::genetic
