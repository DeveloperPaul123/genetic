#pragma once

#include <functional>

#include "genetic/details/concepts.h"
#include "genetic/op/termination/fitness.h"
#include "genetic/op/termination/fitness_hysteresis.h"
#include "genetic/op/termination/generations.h"

namespace dp::genetic {
    /**
     * @brief Determines if the genetic algorithm should terminate.
     * @details This function is a helper function that will call the termination operator
     * with the given fitness value and the termination object.
     * @tparam T The type of the termination operator.
     * @tparam TerminationOp The type of the termination object.
     * @param termination_op The termination operator.
     * @param t The termination object.
     * @param fitness The fitness value.
     * @return True if the algorithm should terminate, false otherwise.
     */
    template <typename T, typename TerminationOp>
        requires dp::genetic::concepts::termination_operator<TerminationOp, T, double>
    constexpr bool should_terminate(TerminationOp &&termination_op, T &&t, double fitness) {
        return std::invoke(std::forward<TerminationOp>(termination_op), t, fitness);
    }
}  // namespace dp::genetic
