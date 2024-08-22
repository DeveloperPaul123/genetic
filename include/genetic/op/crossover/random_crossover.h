#pragma once

#include <ranges>
#include <type_traits>

#include "genetic/details/crossover_helpers.h"

namespace dp::genetic {
    /**
     * @brief Randomly crosses over two parent ranges to produce a child range.
     * @details The pivot index (where the "splice" occurs) is randomly chosen using an
     * IndexProvider which defaults to a uniform integral generator. If the parents are empty,
     * the child will be default constructed.
     */
    struct random_crossover {
        template <std::ranges::range T, typename SimpleType = std::remove_cvref_t<T>,
                  typename IndexProvider = genetic::uniform_integral_generator>
            requires(std::is_default_constructible_v<SimpleType> ||
                     std::is_trivially_default_constructible_v<SimpleType>)
        auto operator()(T &&first, T &&second) {
            const auto &first_size = std::ranges::distance(first);
            const auto &second_size = std::ranges::distance(second);

            if (first_size == 0 || second_size == 0) {
                return SimpleType{};
            }

            static thread_local IndexProvider index_provider{};
            const auto &first_pivot =
                index_provider(static_cast<decltype(first_size)>(0), first_size);
            const auto &second_pivot =
                index_provider(static_cast<decltype(second_size)>(0), second_size);

            // construct our children
            SimpleType child{};

            if constexpr (dp::genetic::type_traits::has_push_back<T>) {
                // check for reserve() and empty() to try and save on allocations
                if constexpr (dp::genetic::type_traits::has_reserve<T>) {
                    // calculate the child size
                    auto child_size = details::calculate_crossover_output_size(
                        first, second, first_pivot, second_pivot);

                    // check for empty and if empty, reserve
                    if constexpr (dp::genetic::type_traits::has_empty<T>) {
                        if (child.empty()) child.reserve(child_size);

                    } else {
                        // blindly reserve to try and save on allocations
                        child.reserve(child_size);
                    }
                }
                // has push_back so we use a back inserter
                details::cross(first, second, first_pivot, second_pivot, std::back_inserter(child));
            } else {
                // otherwise, assume we can insert directly into the type
                details::cross(first, second, first_pivot, second_pivot, std::ranges::begin(child));
            }

            return std::move(child);
        }
    };
}  // namespace dp::genetic
