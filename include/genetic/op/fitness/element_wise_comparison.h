#pragma once
#include <concepts>
#include <ranges>

namespace dp::genetic {
    namespace details {
        template <std::ranges::range Range, typename ScoreType = double>
        struct element_wise_comparison_op {
            explicit element_wise_comparison_op(Range solution, ScoreType match_score)
                : solution_(std::move(solution)), match_score_(std::move(match_score)) {}
            constexpr ScoreType operator()(const Range& value) const {
                ScoreType score{};
                const auto sol_length = std::ranges::distance(solution_);
                const auto val_length = std::ranges::distance(value);
                std::ranges::range_difference_t<Range> index;
                for (index = 0; index < std::min(sol_length, val_length); ++index) {
                    auto solution_val = solution_.at(index);
                    auto val = value.at(index);
                    if (val == solution_val) score += match_score_;
                }

                // subtract for difference in length
                if (sol_length != val_length) {
                    score -= std::abs(sol_length - val_length);
                }

                return score;
            }

          private:
            Range solution_;
            ScoreType match_score_;
        };
    }  // namespace details

    template <std::ranges::range Range, typename ScoreType = double>
    using element_wise_comparison = details::element_wise_comparison_op<Range, ScoreType>;
    // template <std::ranges::range Range, typename Value = std::ranges::range_value_t<Range>>
    // inline constexpr auto element_wise_comparison = [](Range solution, Value match_score) {
    //     return details::element_wise_comparison_op{std::move(solution), std::move(match_score)};
    // };
}  // namespace dp::genetic
