#pragma once

#include <algorithm>
#include <random>
#include <ranges>

#include "genetic/details/concepts.h"
#include "genetic/details/crossover_helpers.h"
#include "genetic/details/random_helpers.h"
#include "genetic/op/crossover/random_crossover.h"

namespace dp::genetic {
    namespace details {
        struct make_children_fn {
            template <std::ranges::range T, concepts::crossover_operator<T> CrossoverOp,
                      typename SimpleType = std::remove_cvref_t<T>,
                      typename IndexProvider = genetic::uniform_integral_generator>
                requires(std::is_default_constructible_v<SimpleType> ||
                         std::is_trivially_default_constructible_v<SimpleType>)
            constexpr auto operator()(CrossoverOp &&crossover_op, const T &first, const T &second) {
                return std::invoke(std::forward<CrossoverOp>(crossover_op), first, second);
            }
        };

    }  // namespace details

    inline auto make_children = details::make_children_fn{};

}  // namespace dp::genetic
