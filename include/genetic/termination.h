#pragma once

#include "genetic/details/concepts.h"
#include "genetic/op/termination/fitness.h"
#include "genetic/op/termination/fitness_hysteresis.h"
#include "genetic/op/termination/generations.h"

#include <functional>

namespace dp::genetic {

    // TODO: Do we have to pass the chromosome type and the fitness value type to the termination?
    // TODO: Can we provide better overload options?
    template <typename T, typename TerminationOp>
        requires dp::genetic::concepts::termination_operator<TerminationOp, T, double>
    constexpr bool should_terminate(TerminationOp &&termination_op, T &&t, double fitness) {
        return std::invoke(std::forward<TerminationOp>(termination_op), t, fitness);
    }
}  // namespace dp::genetic
