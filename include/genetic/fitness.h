#pragma once
#include <concepts>
#include <numeric>
#include <ranges>

#include "genetic/details/concepts.h"
#include "genetic/op/fitness/accumulation.h"
#include "genetic/op/fitness/composite_fitness.h"
#include "genetic/op/fitness/element_wise_comparison.h"

namespace dp::genetic {
    template <std::ranges::range Range, typename FitnessOp,
              typename T = std::invoke_result_t<std::decay_t<FitnessOp>, Range>>
        requires concepts::fitness_operator<std::decay_t<FitnessOp>, Range>
    constexpr T evaluate_fitness(const Range& range, FitnessOp&& fitness_op) {
        return std::invoke(std::forward<FitnessOp>(fitness_op), range);
    }
}  // namespace dp::genetic