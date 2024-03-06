#include <doctest/doctest.h>
#include <genetic/mutation.h>

#include <iostream>

static_assert(dp::genetic::concepts::mutation_operator<dp::genetic::no_op_mutator, std::string>);
static_assert(dp::genetic::concepts::mutation_operator<dp::genetic::no_op_mutator, int>);
static_assert(dp::genetic::concepts::mutation_operator<dp::genetic::no_op_mutator, double>);

static_assert(dp::genetic::concepts::mutation_operator<
              dp::genetic::composite_mutator<dp::genetic::value_insertion_mutator<
                  std::vector<std::string>,
                  dp::genetic::pooled_value_generator<std::vector<std::string>>>>,
              std::string>);
static_assert(dp::genetic::concepts::mutation_operator<
              dp::genetic::composite_mutator<dp::genetic::value_insertion_mutator<
                  std::vector<int>, dp::genetic::pooled_value_generator<std::vector<int>>>>,
              std::vector<int>>);

TEST_CASE("Value replacement mutator") {
    const std::string alphabet = R"(abcdefghijklmnopqrstuvwxyz)";
    // TODO: Better CTAD
    dp::genetic::pooled_value_generator<std::string> value_generator(alphabet);
    dp::genetic::value_replacement<std::string, dp::genetic::pooled_value_generator<std::string>>
        mutator(value_generator);
    const std::string value = "demo";
    const auto new_value = dp::genetic::mutate(mutator, value);

    // same length, but different values
    CHECK_NE(value, new_value);
    CHECK(new_value.length() == value.length());

    std::cout << "Value replacement mutator: " << new_value << "\n";
}

TEST_CASE("Value insertion mutator") {
    const std::string alphabet = R"(abcdefghijklmnopqrstuvwxyz)";
    // TODO: better CTAD
    dp::genetic::pooled_value_generator<std::string> value_generator(alphabet);
    dp::genetic::value_insertion_mutator<std::string> mutator(value_generator);
    const std::string value = "demo";
    const auto new_value = dp::genetic::mutate(mutator, value);
    CHECK_NE(value, new_value);
    CHECK(new_value.length() == value.length() + 1);

    std::cout << "Value insertion mutator: " << new_value << "\n";

    constexpr std::uint64_t insertions = 10;

    std::vector<int> value_range{};
    std::ranges::generate_n(std::back_inserter(value_range), insertions,
                            [i = 0]() mutable { return i++; });
    dp::genetic::pooled_value_generator<std::vector<int>> int_value_gen{value_range};
    dp::genetic::value_insertion_mutator<std::vector<int>> vector_mutator(int_value_gen,
                                                                          insertions);

    auto input = std::vector{1, 2, 3, 4};
    auto vector_result = dp::genetic::mutate(vector_mutator, input);
    CHECK(vector_result.size() == input.size() + insertions);
}

TEST_CASE("Composite mutator") {
    dp::genetic::composite_mutator mutator{
        [](const std::string& string) { return string + "part1"; },
        [](const std::string& string) { return string + "part2"; }};
    using namespace std::string_literals;
    const auto result = dp::genetic::mutate(mutator, "test"s);

    CHECK_EQ(result, "testpart1part2");
}

TEST_CASE("Value mutator") {
    const auto double_value_mutator = dp::genetic::double_value_mutator(-0.1, 0.1);
    const std::vector xyz{1.0, 2.0, 3.0};
    auto new_value = dp::genetic::mutate(double_value_mutator, xyz);
    for (auto i = 0; i < xyz.size(); ++i) {
        CHECK(std::abs(new_value[i] - xyz[i]) <= 0.1);
    }

    const auto float_value_mutator = dp::genetic::float_value_mutator(-1.f, 1.f);
    const std::vector xyz_f{1.f, 2.f, 3.f};
    auto new_value_f = dp::genetic::mutate(float_value_mutator, xyz_f);
    for (auto i = 0; i < xyz_f.size(); ++i) {
        CHECK(std::abs(new_value_f[i] - xyz_f[i]) <= 1.f);
    }

    const auto int_value_mutator = dp::genetic::integral_value_mutator(-5, 5);
    const std::vector xyz_int{1u, 2u, 3u};
    auto new_value_int = dp::genetic::mutate(int_value_mutator, xyz_int);
    for (auto i = 0; i < xyz_int.size(); ++i) {
        int diff = new_value_int[i] - xyz_int[i];
        CHECK(std::abs(diff) <= 5);
    }

    const std::array<double, 2> xy_array{1.0, 2.0};
    static_assert(dp::genetic::type_traits::is_std_array<std::array<double, 2>>);
    auto new_value_array = dp::genetic::mutate(double_value_mutator, xy_array);
    for (auto i = 0; i < xy_array.size(); ++i) {
        CHECK(std::abs(new_value_array[i] - xy_array[i]) <= 0.1);
    }
}
