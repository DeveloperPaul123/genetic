#pragma once
#include <concepts>
#include <numeric>
#include <ranges>

#include "genetic/details/concepts.h"
#include "genetic/op/fitness/accumulation.h"
#include "genetic/op/fitness/composite_fitness.h"
#include "genetic/op/fitness/element_wise_comparison.h"

namespace dp::genetic {
    /**
     * @brief Evaluates fitness of a range using the provided fitness operator.
     *
     * @tparam Range The input type.
     * @tparam FitnessOp The fitness operator type.
     * @tparam T The return type of the fitness operator.
     */
    template <std::ranges::range Range, typename FitnessOp,
              typename T = std::invoke_result_t<std::decay_t<FitnessOp>, Range>>
        requires concepts::fitness_operator<std::decay_t<FitnessOp>, Range>
    constexpr T evaluate_fitness(FitnessOp&& fitness_op, const Range& range) {
        return std::invoke(std::forward<FitnessOp>(fitness_op), range);
    }
}  // namespace dp::genetic
