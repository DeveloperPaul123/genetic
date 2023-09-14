#pragma once
#include <numeric>
#include <ranges>

namespace dp::genetic {
    namespace details {
        struct accumulation_fitness_op {
            template <std::ranges::range Range, typename ScoreType = double>
            constexpr ScoreType operator()(Range&& value) const {
                return std::accumulate(std::ranges::begin(value), std::ranges::end(value), ScoreType{});
            }
        };
    }  // namespace details

    inline constexpr auto accumulation_fitness = details::accumulation_fitness_op{};
}  // namespace dp::genetic
