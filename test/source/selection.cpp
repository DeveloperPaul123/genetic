#include <doctest/doctest.h>
#include <genetic/selection.h>

#include <iostream>
#include <ranges>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

// Helper function for testing selectors
template <typename Selector, typename T, std::ranges::range PopulationType>
std::unordered_map<T, int> test_selection(Selector& selector, const T& test_value,
                                          PopulationType initial_population,
                                          const unsigned& selection_count = 1000) {
    auto fitness_op = [test_value](const std::string& value) -> double {
        double score = 0.0;
        score -=
            std::abs(static_cast<double>(test_value.size()) - static_cast<double>(value.size())) *
            10.0;
        for (unsigned i = 0; i < test_value.size(); i++) {
            score += test_value[i] == value[i];
        }
        return score;
    };

    std::unordered_map<T, int> selection_histogram;

    // do 1000 selections and build up a selection histogram
    for (unsigned i = 0; i < selection_count; i++) {
        const auto& [parent1, parent2] = selector(initial_population, fitness_op);
        if (!selection_histogram.contains(parent1)) {
            selection_histogram[parent1] = 0;
        } else {
            ++selection_histogram[parent1];
        }

        if (!selection_histogram.contains(parent2)) {
            selection_histogram[parent2] = 0;
        } else {
            ++selection_histogram[parent2];
        }
    }

    return selection_histogram;
}

TEST_CASE("Roulette selection") {
    const std::string test_value = "test";

    // create a sample population where 1 member has a much larger fitness compared to the others.
    const std::vector<std::string> initial_population{"tesa", "aaaa", "bbbb", "aaa", "bbb"};

    dp::genetic::roulette_selection selection{};
    const auto selection_histogram = test_selection(selection, test_value, initial_population);

    for (auto& [string, count] : selection_histogram) {
        std::cout << string << " : " << std::to_string(count) << "\n";
    }

    const auto [string_value, count] = *std::ranges::max_element(
        selection_histogram, [](auto first, auto second) { return first.second < second.second; });
    CHECK(string_value == "tesa");
}

TEST_CASE("Roulette selection with ranges::view") {
    const std::string test_value = "test";
    using data = std::pair<std::string, int>;
    // create a sample population where 1 member has a much larger fitness compared to the others.
    const std::vector<data> initial_population{data{"tesa", 0}, data{"aaaa", 1}, data{"bbbb", 2},
                                               data{"aaa", 3}, data{"bbb", 4}};

    dp::genetic::roulette_selection selection{};
    const auto selection_histogram =
        test_selection(selection, test_value, initial_population | std::views::elements<0>);

    for (auto& [string, count] : selection_histogram) {
        std::cout << string << " : " << std::to_string(count) << "\n";
    }

    const auto [string_value, count] = *std::ranges::max_element(
        selection_histogram, [](auto first, auto second) { return first.second < second.second; });
    CHECK(string_value == "tesa");
}

TEST_CASE("Rank selection") {
    dp::genetic::rank_selection selector{};
    const std::string test_value = "test";

    // create a sample population where 1 member has a much larger fitness compared to the others.
    const std::vector<std::string> initial_population{"tesa", "aaaa", "bbbb", "aaa", "bbb"};

    const auto selection_histogram = test_selection(selector, test_value, initial_population);

    for (auto& [string, count] : selection_histogram) {
        std::cout << string << " : " << std::to_string(count) << "\n";
    }

    constexpr auto comp_op = [](auto first, auto second) { return first.second < second.second; };
    const auto [string_value, count] = *std::ranges::max_element(selection_histogram, comp_op);
    const auto [min_value, min_count] = *std::ranges::min_element(selection_histogram, comp_op);

    CHECK(string_value == "tesa");
    CHECK(min_value == "bbb");
}

TEST_CASE("Rank selection with ranges::view") {
    dp::genetic::rank_selection selector{};
    const std::string test_value = "test";

    using data = std::pair<std::string, int>;

    // create a sample population where 1 member has a much larger fitness compared to the others.
    std::vector initial_population{data{"tesa", 0}, data{"aaaa", 1}, data{"bbbb", 2},
                                   data{"aaa", 3}, data{"bbb", 4}};

    const auto selection_histogram =
        test_selection(selector, test_value, initial_population | std::views::elements<0>);
    for (auto& [string, count] : selection_histogram) {
        std::cout << string << " : " << std::to_string(count) << "\n";
    }

    constexpr auto comp_op = [](auto first, auto second) { return first.second < second.second; };
    const auto [string_value, count] = *std::ranges::max_element(selection_histogram, comp_op);
    const auto [min_value, min_count] = *std::ranges::min_element(selection_histogram, comp_op);

    CHECK(string_value == "tesa");
    CHECK(min_value == "bbb");
}
