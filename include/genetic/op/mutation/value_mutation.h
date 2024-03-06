#pragma once
#include <concepts>
#include <ranges>

#include "genetic/details/concepts.h"

namespace dp::genetic {
    namespace details {

        struct number_generator_helper_op {
            template <typename Number>
            [[nodiscard]] constexpr Number operator()(Number lower, Number upper) const {
                if constexpr (std::is_floating_point<Number>()) {
                    static dp::genetic::uniform_floating_point_generator float_gen{};
                    return float_gen(lower, upper);
                } else {
                    static dp::genetic::uniform_integral_generator int_gen{};
                    return int_gen(lower, upper);
                }
            }
        };

        template <typename OutputRange>
        struct range_converter_helper_op {
            template <typename InputRange>
                requires std::ranges::range<InputRange>
            OutputRange operator()(const InputRange& input) {
                // check for std::array and std::ranges::sized_range
                if constexpr (std::ranges::sized_range<InputRange> &&
                              dp::genetic::type_traits::is_std_array<OutputRange>) {
                    OutputRange output{};
                    for (auto i = 0; i < std::ranges::size(output); ++i) {
                        output[i] = input[i];
                    }
                    return output;

                } else {
                    // otherwise, just convert to the output range
                    // note we have to wrap this in an "else" otherwise it won't compile due to it
                    // being ill-formed for the "to" conversion
                    return input | std::ranges::to<std::remove_cvref_t<OutputRange>>();
                }
            }
        };

        constexpr inline auto number_generator = number_generator_helper_op{};
        template <typename Output>
        using range_converter_helper = range_converter_helper_op<Output>;

        template <dp::genetic::type_traits::number Number>
        struct value_mutation_op {
            Number lower_bound;
            Number upper_bound;

            template <std::ranges::range T, typename ValueType = typename std::remove_cvref_t<
                                                std::ranges::range_value_t<T>>>
                requires type_traits::addable<ValueType, Number>
            [[nodiscard]] T operator()(const T& t) const {
                namespace vw = std::ranges::views;
                auto result =
                    t | vw::transform([low = lower_bound, up = upper_bound](const auto& value) {
                        return static_cast<ValueType>(
                            std::plus()(value, number_generator(low, up)));
                    });

                auto converted_range = range_converter_helper<std::remove_cvref_t<T>>{}(result);
                return std::move(converted_range);
            }
        };
    }  // namespace details

    inline auto double_value_mutator(double lower_bound, double upper_bound) {
        return details::value_mutation_op<double>{lower_bound, upper_bound};
    }

    inline auto float_value_mutator(float lower_bound, float upper_bound) {
        return details::value_mutation_op<float>{lower_bound, upper_bound};
    }

    template <std::integral Number>
    constexpr inline auto integral_value_mutator(Number lower_bound, Number upper_bound) {
        return details::value_mutation_op<Number>{lower_bound, upper_bound};
    }
}  // namespace dp::genetic
