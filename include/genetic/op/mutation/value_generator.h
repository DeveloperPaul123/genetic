#pragma once

#include <concepts>
#include <ranges>

#include "genetic/details/concepts.h"
#include "genetic/details/random_helpers.h"

namespace dp::genetic {
    /**
     * @brief Generates a value from a pool of possible values. Value is selected randomly
     */
    template <std::ranges::sized_range T,
              typename ValueType = std::remove_cvref_t<std::ranges::range_value_t<T>>,
              dp::genetic::concepts::index_generator IndexGenerator =
                  dp::genetic::uniform_integral_generator>
    struct pooled_value_generator {
        pooled_value_generator(const T possible_values) : values_(std::move(possible_values)) {}
        [[nodiscard]] constexpr ValueType operator()() {
            static IndexGenerator index_generator{};
            const auto output_index =
                index_generator(static_cast<std::size_t>(0),
                                static_cast<std::size_t>(std::ranges::size(values_) - 1));

            auto location = std::ranges::begin(values_) + output_index;
            return *location;
        }

      private:
        T values_;
    };
}  // namespace dp::genetic
