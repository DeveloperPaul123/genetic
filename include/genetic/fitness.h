#pragma once
#include <numeric>
#include <ranges>

namespace dp::genetic {
    struct accumulation_fitness {
        template <std::ranges::range T, typename U = double>
        U operator()(T&& value) {
            return std::accumulate(std::ranges::begin(value), std::ranges::end(value), U{});
        }
    };

    template <std::ranges::range T>
    struct element_wise_comparison {
        explicit element_wise_comparison(T solution) : solution_(std::move(solution)) {}
        template <typename U = double>
        U operator()(const T& value) {
            U score{};
            const auto sol_length = std::ranges::distance(solution_);
            const auto val_length = std::ranges::distance(value);
            std::ranges::range_difference_t<T> index;
            for (index = 0; index < std::min(sol_length, val_length); ++index) {
                auto solution_val = solution_.at(index);
                auto val = value.at(index);
                if (val == solution_val) score += 10;
            }

            // subtract for difference in length
            if (sol_length != val_length) {
                score -= std::abs(sol_length - val_length);
            }

            return score;
        }

      private:
        T solution_;
    };
}  // namespace dp::genetic