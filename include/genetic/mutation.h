#pragma once

#include <random>
#include <algorithm>

#include "details/random_helpers.h"
#include "genetic/details/concepts.h"

namespace dp::genetic {
    /**
     * @brief No-op mutator, returns the value unchanged.
     */
    struct noop_mutator {
        template <typename T>
        constexpr T operator()(const T& value) const {
            return value;
        }
    };

    /**
     * @brief Replaces n values at random locations in the from a range of possible values.
     * @tparam T Population type
     * @tparam IndexGenerator Random number generator that generates indices
     */
    template <std::ranges::range T, dp::genetic::concepts::index_generator IndexGenerator =
                                        dp::genetic::uniform_integral_generator>
    struct value_replacement_mutator {
        template <std::ranges::range ValueRange>
            requires(
                std::same_as<std::ranges::range_value_t<T>, std::ranges::range_value_t<ValueRange>>)
        explicit value_replacement_mutator(ValueRange values,
                                           std::size_t number_of_replacements = 1)
            : replacement_count_(number_of_replacements) {
            // todo: what if value range is empty?
            values_.reserve(std::ranges::distance(values));
            std::ranges::copy(values, std::back_inserter(values_));
        }

        [[nodiscard]] T operator()(const T& value) {
            static thread_local IndexGenerator index_generator{};
            const auto value_list_size = std::ranges::distance(values_);
            const auto value_size = std::ranges::distance(value);
            T return_value = value;

            for (std::size_t i = 0; i < replacement_count_; ++i) {
                const auto value_index =
                    index_generator(static_cast<decltype(value_list_size)>(0), value_list_size - 1);
                const auto output_index =
                    index_generator(static_cast<decltype(value_size)>(0), value_size - 1);
                auto location = std::ranges::begin(return_value) + output_index;
                std::swap(*location, values_[value_index]);
            }

            return return_value;
        }

      private:
        std::vector<std::ranges::range_value_t<T>> values_;
        std::size_t replacement_count_;
    };

    /**
     * @brief Inserts n values at random positions by randomly selecting values from the input
     * range.
     * @tparam T Population type
     * @tparam IndexGenerator Random number generator that generates indices
     */
    template <std::ranges::range T, dp::genetic::concepts::index_generator IndexGenerator =
                                        dp::genetic::uniform_integral_generator>
    struct value_insertion_mutator {
        template <std::ranges::range ValueRange>
            requires(
                std::same_as<std::ranges::range_value_t<T>, std::ranges::range_value_t<ValueRange>>)
        explicit value_insertion_mutator(ValueRange values, std::size_t number_of_insertions = 1)
            : insertion_count_(number_of_insertions) {
            // todo: what if value range is empty?
            values_.reserve(std::ranges::distance(values));
            std::ranges::copy(values, std::back_inserter(values_));
        }

        [[nodiscard]] T operator()(const T& value) {
            static thread_local IndexGenerator index_generator{};
            const auto value_list_size = std::ranges::distance(values_);
            const auto value_size = std::ranges::distance(value);
            T return_value = value;

            for (std::size_t i = 0; i < insertion_count_; ++i) {
                const auto value_index =
                    index_generator(static_cast<decltype(value_list_size)>(0), value_list_size - 1);
                const auto output_index =
                    index_generator(static_cast<decltype(value_size)>(0), value_size - 1);
                auto location = std::ranges::begin(return_value) + output_index;
                // todo: abstract the insertion logic
                return_value.insert(location, values_[value_index]);
            }

            return return_value;
        }

      private:
        std::vector<std::ranges::range_value_t<T>> values_;
        std::size_t insertion_count_;
    };

    template <typename... Args>
    struct composite_mutator {
      private:
        // list of mutators
        std::tuple<Args...> mutators_;
        // https://stackoverflow.com/questions/75039429/chaining-callables-in-c
        /// @brief Helper to help us chain mutator calls together
        template <size_t index, class Arg>
        decltype(auto) call_helper(Arg&& arg) {
            if constexpr (index == 0) {
                return std::forward<Arg>(arg);
            } else {
                return std::get<index - 1>(mutators_)(
                    call_helper<index - 1>(std::forward<Arg>(arg)));
            }
        }

      public:
        explicit composite_mutator(Args&&... args) : mutators_(std::forward<Args>(args)...) {}
        template <typename T>
        T operator()(T t) {
            return call_helper<sizeof...(Args)>(t);
        }
    };
}  // namespace dp::genetic

static_assert(std::uniform_random_bit_generator<std::mt19937_64>);