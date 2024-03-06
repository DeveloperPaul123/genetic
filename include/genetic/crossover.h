#pragma once

#include <algorithm>
#include <random>
#include <ranges>

#include "details/concepts.h"
#include "details/random_helpers.h"

namespace dp::genetic {

    namespace details {
        /**
         * @brief Calculates the output size of the child given the parents and the pivot point.
         * Pairs with the default_crossover operator and the cross function to allow for child
         * pre-allocation to avoid using back_inserter.
         * @tparam FirstParent Type for the first parent
         * @tparam SecondParent Type for the second parent
         * @param first The first parent for the child
         * @param second The second parent for the child
         * @param first_pivot The first pivot point, or where the first parent will be "split" to
         * create the child.
         * @param second_pivot The second pivot point, or where the second parent will be "split"
         * @return The output size of the child.
         */
        template <std::ranges::input_range FirstParent, std::ranges::input_range SecondParent>
        auto calculate_crossover_output_size(FirstParent &&first, SecondParent &&second,
                                             const std::size_t &first_pivot,
                                             const std::size_t &second_pivot) -> std::size_t {
            auto first_pivot_point = std::ranges::begin(first) + first_pivot;
            auto second_pivot_point = std::ranges::begin(second) + second_pivot;
            auto child_size = std::ranges::distance(std::ranges::begin(first), first_pivot_point) +
                              std::ranges::distance(second_pivot_point, std::ranges::end(second));

            return child_size;
        }

        template <typename FirstParent, typename SecondParent,
                  typename T = dp::genetic::type_traits::element_type_t<FirstParent>>
            requires std::ranges::input_range<FirstParent> &&
                     std::ranges::input_range<SecondParent> &&
                     std::is_same_v<dp::genetic::type_traits::element_type_t<FirstParent>,
                                    dp::genetic::type_traits::element_type_t<SecondParent>>
        void cross(FirstParent &&first, SecondParent &&second, const std::size_t &first_pivot,
                   const std::size_t &second_pivot, std::output_iterator<T> auto child_output) {
            const auto &parent1_first_half_size = std::ranges::distance(
                std::ranges::begin(first), std::ranges::begin(first) + first_pivot);

            const auto &parent2_first_half_size = std::ranges::distance(
                std::ranges::begin(second), std::ranges::begin(second) + second_pivot);

            auto parent1_first_part = first | std::ranges::views::take(parent1_first_half_size);

            auto parent2_second_part = second | std::ranges::views::drop(parent2_first_half_size);

            // child is the combination of the first half of parent 1 + second half of parent 2
            std::ranges::copy(parent1_first_part, child_output);
            std::ranges::copy(parent2_second_part, child_output);
        }

        template <typename FirstParent, typename SecondParent,
                  typename T = dp::genetic::type_traits::element_type_t<FirstParent>>
            requires std::ranges::input_range<FirstParent> &&
                     std::ranges::input_range<SecondParent> &&
                     std::is_same_v<dp::genetic::type_traits::element_type_t<FirstParent>,
                                    dp::genetic::type_traits::element_type_t<SecondParent>>
        void cross(FirstParent &&first, SecondParent &&second, const std::size_t &pivot,
                   std::output_iterator<T> auto child_output) {
            details::cross(first, second, pivot, pivot, child_output);
        }
    }  // namespace details

    /**
     * @brief A default crossover implementation that works for a variety of types and satisfies the
     * dp::details::crossover_operator concept.
     */
    struct default_crossover {
        template <std::ranges::range T, typename SimpleType = std::remove_cvref_t<T>,
                  typename IndexProvider = genetic::uniform_integral_generator>
            requires(std::is_default_constructible_v<SimpleType> ||
                     std::is_trivially_default_constructible_v<SimpleType>)
        auto operator()(T &&first, T &&second) {
            const auto &first_size = std::ranges::distance(first);
            const auto &second_size = std::ranges::distance(second);

            // todo handle empty parents
            if (first_size == 0 || second_size == 0) {
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

        template <dp::genetic::type_traits::addable T, typename SimpleType = std::remove_cvref_t<T>>
            requires(!std::is_convertible_v<T, std::string>)
        auto operator()(T &&first, T &&second) {
            return first + second;
        }
    };
}  // namespace dp::genetic
