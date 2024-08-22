#pragma once

#include <algorithm>
#include <random>

#include "details/random_helpers.h"
#include "genetic/details/concepts.h"
#include "genetic/op/mutation/composite_mutation.h"
#include "genetic/op/mutation/no_op.h"
#include "genetic/op/mutation/value_generator.h"
#include "genetic/op/mutation/value_insertion.h"
#include "genetic/op/mutation/value_mutation.h"
#include "genetic/op/mutation/value_replacement.h"

namespace dp::genetic {

    /**
     * @brief Mutate a value using the provided mutation operator.
     *
     * @tparam T The input value type.
     * @tparam Mutator The mutation operator.
     * @param mutator The mutation operator, takes in a value and returns a mutated value.
     * @param input_value The input value to mutate.
     * @return The mutated value.
     */
    template <typename T, dp::genetic::concepts::mutation_operator<T> Mutator>
    [[nodiscard]] constexpr T mutate(Mutator&& mutator, const T& input_value) {
        return std::invoke(std::forward<Mutator>(mutator), input_value);
    }
}  // namespace dp::genetic
