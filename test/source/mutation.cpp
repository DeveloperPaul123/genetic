#include <doctest/doctest.h>
#include <genetic/mutation.h>

#include <iostream>

static_assert(dp::genetic::concepts::mutation_operator<dp::genetic::noop_mutator, std::string>);
static_assert(dp::genetic::concepts::mutation_operator<dp::genetic::noop_mutator, int>);
static_assert(dp::genetic::concepts::mutation_operator<dp::genetic::noop_mutator, double>);

static_assert(dp::genetic::concepts::mutation_operator<
              dp::genetic::composite_mutator<
                  dp::genetic::value_insertion_mutator<std::vector<std::string>>>,
              std::string>);
static_assert(
    dp::genetic::concepts::mutation_operator<
        dp::genetic::composite_mutator<dp::genetic::value_insertion_mutator<std::vector<int>>>,
        std::vector<int>>);

TEST_CASE("Value replacement mutator") {
    const std::string alphabet = R"(abcdefghijklmnopqrstuvwxyz)";
    dp::genetic::value_replacement_mutator<std::string> mutator{alphabet};
    const std::string value = "demo";
    const auto new_value = mutator(value);

    // same length, but different values
    CHECK_NE(value, new_value);
    CHECK(new_value.length() == value.length());

    std::cout << "Value replacement mutator: " << new_value << "\n";
}

TEST_CASE("Value insertion mutator") {
    const std::string alphabet = R"(abcdefghijklmnopqrstuvwxyz)";
    dp::genetic::value_insertion_mutator<std::string> mutator{alphabet};
    const std::string value = "demo";
    const auto new_value = mutator(value);
    CHECK_NE(value, new_value);
    CHECK(new_value.length() == value.length() + 1);

    std::cout << "Value insertion mutator: " << new_value << "\n";

    constexpr std::size_t insertions = 10;

    std::vector<int> value_range{};
    std::ranges::generate_n(std::back_inserter(value_range), insertions,
                            [i = 0]() mutable { return i++; });
    dp::genetic::value_insertion_mutator<std::vector<int>> vector_mutator{value_range, insertions};

    auto input = std::vector{1, 2, 3, 4};
    auto vector_result = vector_mutator(input);
    CHECK(vector_result.size() == input.size() + insertions);
}

TEST_CASE("Composite mutator") {
    dp::genetic::composite_mutator mutator{
        [](const std::string& string) { return string + "part1"; },
        [](const std::string& string) { return string + "part2"; }};
    using namespace std::string_literals;
    const auto result = mutator("test"s);

    CHECK_EQ(result, "testpart1part2");
}
