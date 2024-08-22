#include <doctest/doctest.h>
#include <genetic/details/concepts.h>
#include <genetic/fitness.h>

#include <unordered_map>
#include <vector>

static_assert(dp::genetic::concepts::fitness_operator<dp::genetic::details::accumulation_fitness_op,
                                                      std::string>);
static_assert(std::invocable<decltype(dp::genetic::accumulation_fitness), std::string>);
static_assert(dp::genetic::concepts::fitness_operator<
              dp::genetic::element_wise_comparison<std::string, double>, std::string>);
static_assert(dp::genetic::concepts::fitness_operator<
              dp::genetic::element_wise_comparison<std::vector<int>>, std::vector<int>>);

TEST_CASE("Accumulation fitness operator") {
    const std::vector values{1.0, 2.0, 3.0, 4.0};
    const auto fitness = dp::genetic::evaluate_fitness(dp::genetic::accumulation_fitness, values);

    CHECK(fitness == 10.0);

    constexpr std::array arr_values{1.f, 2.f, 3.f, 4.f, 5.f, 6.f};
    const auto arr_fitness =
        dp::genetic::evaluate_fitness(dp::genetic::accumulation_fitness, arr_values);
    CHECK(arr_fitness == 21.f);

    std::unordered_map<std::string, int> map_values{{"a", 1}, {"b", 2}, {"c", 3}};
    const auto map_value_fitness = dp::genetic::evaluate_fitness(
        dp::genetic::accumulation_fitness, map_values | std::ranges::views::values);

    CHECK(map_value_fitness == 6);
}

TEST_CASE("Element-wise fitness") {
    const std::vector data{1.0, 2.0, 3.0, 4.0};
    const std::vector solution{1.0, 2.0, 4.0};

    const auto fitness =
        dp::genetic::evaluate_fitness(dp::genetic::element_wise_comparison(solution, 1.0), data);
    CHECK(fitness == 1.0);
}

TEST_CASE("Composite fitness") {
    const std::vector data{1.0, 2.0, 3.0, 4.0};
    const std::vector solution{1.0, 2.0, 4.0};

    // clang-format off
    dp::genetic::composite_sum_fitness composite{
        dp::genetic::accumulation_fitness,
        dp::genetic::element_wise_comparison(solution, 1.0),
        [](const auto& rng) {
            return static_cast<double>(rng.size()) * 2.0;
        }
    };
    // clang-format on

    const auto fitness = dp::genetic::evaluate_fitness(composite, data);

    // accumulation fitness will give us 10.0
    // element-wise fitness will give us 1.0 (2 matches * 1.0 - 1 mismatch in size * 1.0)
    // total fitness will be 11.0 + data.size() * 2
    CHECK(fitness == 11.0 + data.size() * 2);

    // clang-format off
    dp::genetic::composite_difference_fitness composite_diff{
        dp::genetic::accumulation_fitness,
        [](const auto& rng) { return static_cast<double>(rng.size());},
        [](const auto&) {return 1.0;}
    };
    // clang-format on

    const auto diff_fitness = dp::genetic::evaluate_fitness(composite_diff, data);

    CHECK(diff_fitness == 10.0 - static_cast<double>(data.size()) - 1.0);

    // check product

    // clang-format off
    dp::genetic::composite_product_fitness composite_product{
        composite,
        composite_diff,
        [](const auto& rng) { return rng.size() * 2; }
    };
    // clang-format on

    const auto product_fitness = dp::genetic::evaluate_fitness(composite_product, data);
    CHECK(product_fitness == fitness * diff_fitness * data.size() * 2);
}
