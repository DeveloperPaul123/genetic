#pragma once

#include <ranges>
#include <type_traits>

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
}  // namespace dp::genetic
