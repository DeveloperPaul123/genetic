#pragma once

#include <concepts>
#include <cstdint>
#include <ranges>

#include "genetic/details/concepts.h"

namespace dp::genetic {

    template <
        std::ranges::sized_range Range,
        dp::genetic::concepts::value_generator<std::ranges::range_value_t<Range>> ValueGenerator,
        dp::genetic::concepts::index_generator IndexGenerator =
            dp::genetic::uniform_integral_generator>
    struct value_replacement {
        explicit value_replacement(const ValueGenerator& generator,
                                   std::uint_least64_t num_replacements = 1)
            : generator_(generator), number_of_replacements_(num_replacements) {}

        template <typename T>
        constexpr T operator()(const T& t) {
            static IndexGenerator index_generator{};
            auto return_value = t;
            for (std::uint_least64_t _ :
                 std::views::iota(static_cast<std::uint_least64_t>(0), number_of_replacements_)) {
                const auto output_index =
                    index_generator(static_cast<std::size_t>(0),
                                    static_cast<std::size_t>(std::ranges::size(t) - 1));

                auto location = std::ranges::begin(return_value) + output_index;
                auto value = std::invoke(generator_);
                std::swap(*location, value);
            }
            return return_value;
        }

      private:
        ValueGenerator generator_;
        std::uint_least64_t number_of_replacements_;
    };

}  // namespace dp::genetic