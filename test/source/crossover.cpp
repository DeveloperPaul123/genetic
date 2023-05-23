#include <doctest/doctest.h>
#include <genetic/crossover.h>

#include <array>
#include <iostream>
#include <string>
#include <vector>

using namespace dp::genetic;

TEST_CASE("Test generic cross technique") {
    constexpr auto first_point = 2;
    constexpr auto second_point = 4;

    std::string first(4, 'a');
    std::string second(6, 'b');

    std::string child1;
    std::string child2;
    details::cross(first, second, first_point, second_point, std::back_inserter(child1));
    details::cross(second, first, second_point, first_point, std::back_inserter(child2));

    CHECK(child1 == "aabb");
    CHECK(child2 == "bbbbaa");

    const auto& c1_size =
        details::calculate_crossover_output_size(first, second, first_point, second_point);
    const auto& c2_size =
        details::calculate_crossover_output_size(second, first, second_point, first_point);
    CHECK(child1.size() == c1_size);
    CHECK(child2.size() == c2_size);

    std::array a_first{1, 1, 1, 1};
    std::array a_second{2, 2, 2, 2, 2, 2};
    static_assert(std::is_same_v<decltype(a_first)::value_type, decltype(a_second)::value_type>);

    std::vector<int> v_c1, v_c2;
    details::cross(a_first, a_second, first_point, second_point, std::back_inserter(v_c1));
    details::cross(a_second, a_first, second_point, first_point, std::back_inserter(v_c2));

    // do a cross over but pre-compute size and pre-allocate
    std::vector<int> v_c1_pre_alloc(
        details::calculate_crossover_output_size(a_first, a_second, first_point, second_point));
    std::vector<int> v_c2_pre_alloc(
        details::calculate_crossover_output_size(a_second, a_first, second_point, first_point));
    details::cross(a_first, a_second, first_point, second_point, std::begin(v_c1_pre_alloc));
    details::cross(a_second, a_first, second_point, first_point, std::begin(v_c2_pre_alloc));
}

TEST_CASE("Test default crossover operator") {
    default_crossover crossover{};

    using namespace std::string_literals;

    const auto& p1 = "aabb"s;
    const auto& p2 = "bbaa"s;
    const std::string& child1 = crossover(p1, p2);
    const std::string& child2 = crossover(p2, p1);

    std::cout << child1 << '\n' << child2 << '\n' << std::endl;
}

// cross over concept checks
static_assert(dp::genetic::concepts::crossover_operator<default_crossover, std::string>);
static_assert(dp::genetic::concepts::crossover_operator<default_crossover, int>);
static_assert(dp::genetic::concepts::crossover_operator<default_crossover, std::array<int, 4>>);
static_assert(dp::genetic::concepts::crossover_operator<default_crossover, std::vector<int>>);
static_assert(
    dp::genetic::concepts::crossover_operator<default_crossover, std::vector<std::string>>);
