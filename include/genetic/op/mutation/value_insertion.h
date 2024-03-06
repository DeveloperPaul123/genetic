#pragma once

#include <functional>
#include <ranges>

#include "genetic/details/concepts.h"
#include "genetic/op/mutation/value_generator.h"

namespace dp::genetic {
    /**
     * @brief Inserts n values at random positions by randomly selecting values from the input
     * range.
     * @tparam T Population type
     * @tparam IndexGenerator Random number generator that generates indices
     */
    template <std::ranges::range Range, typename ValueType = std::ranges::range_value_t<Range>,
              dp::genetic::concepts::index_generator IndexGenerator =
                  dp::genetic::uniform_integral_generator>
    struct value_insertion_mutator {
        template <typename ValueGenerator>
            requires std::invocable<ValueGenerator> &&
                         std::convertible_to<std::invoke_result_t<ValueGenerator>, ValueType>
        explicit value_insertion_mutator(ValueGenerator&& generator,
                                         std::uint_least64_t number_of_insertions = 1)
            : generator_(std::forward<ValueGenerator>(generator)),
              number_of_insertions_(number_of_insertions) {}

        template <typename T>
        [[nodiscard]] T operator()(const T& value) {
            static IndexGenerator index_generator{};
            T return_value = value;

            for (std::size_t i = 0; i < number_of_insertions_; ++i) {
                const auto output_index =
                    index_generator(static_cast<std::size_t>(0),
                                    static_cast<std::size_t>(std::ranges::size(value)) - 1);
                auto location = std::ranges::begin(return_value) + output_index;
                // todo: abstract the insertion logic
                return_value.insert(location, std::invoke(generator_));
            }

            return return_value;
        }

      private:
        std::function<ValueType()> generator_;
        std::uint_least64_t number_of_insertions_;
    };
}  // namespace dp::genetic
