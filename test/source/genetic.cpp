#include <doctest/doctest.h>
#include <genetic/crossover.h>
#include <genetic/details/concepts.h>
#include <genetic/genetic.h>
#include <genetic/selection.h>
#include <genetic/termination.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>

struct random_word_generator {
    [[nodiscard]] std::string operator()(std::string_view char_set, std::size_t max_length) const {
        static thread_local auto generator = dp::genetic::uniform_integral_generator{};
        const auto& out_length = generator(static_cast<std::size_t>(1), max_length);
        std::string out(out_length, '\0');
        std::generate_n(out.begin(), out_length,
                        [&]() { return char_set[generator(std::size_t{0}, char_set.size() - 1)]; });
        return out;
    }
};

TEST_CASE("Search for string") {
    // solution
    const std::string solution = "Hello, world!";

    // define available characters
    const std::string alphabet = R"(abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!,.)";
    const std::string available_chars = alphabet + " ,'";

    const auto word_length = solution.size() + solution.size() / 2;

    // generate random words as our initial population
    random_word_generator word_generator{};
    // generate initial population
    constexpr auto initial_pop_size = 1000;
    std::vector<std::string> initial_population(initial_pop_size);
    std::generate_n(initial_population.begin(), initial_pop_size,
                    [&]() { return word_generator(available_chars, word_length); });

    // fitness evaluator
    dp::genetic::element_wise_comparison fitness_op(solution);

    auto string_mutator = dp::genetic::composite_mutator{
        [&](const std::string& input) {
            if (input.empty()) {
                return word_generator(available_chars, word_length);
            }
            return input;
        },
        dp::genetic::value_replacement_mutator<std::string>{available_chars}};

    // termination criteria
    auto termination = dp::genetic::fitness_termination_criteria(fitness_op(solution));

    static_assert(
        dp::genetic::concepts::termination_operator<dp::genetic::fitness_termination_criteria,
                                                    std::string, double>);

    static_assert(
        dp::genetic::concepts::selection_operator<dp::genetic::rank_selection, std::string,
                                                  std::vector<std::string>, decltype(fitness_op)>);
    // algorithm settings
    dp::genetic::algorithm_settings settings{0.1, 0.5, 0.25};
    dp::genetic::params<std::string> params =
        dp::genetic::params<std::string>::builder()
            .with_mutation_operator(string_mutator)
            .with_crossover_operator(dp::genetic::default_crossover{})
            .with_fitness_operator(fitness_op)
            .with_termination_operator(termination)
            .build();
    // set up algorithm
    dp::genetic_algorithm genetics(settings, params);

    auto start = std::chrono::steady_clock::now();
    auto [best, fitness] = genetics.solve(initial_population, [](auto& stats) {
        std::cout << "best: " << stats.current_best.best
                  << " fitness: " << stats.current_best.fitness << "\n";
    });
    auto stop = std::chrono::steady_clock::now();
    auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    std::cout << "Total time (ms): " << std::to_string(time_ms) << "\n";
    CHECK(best == solution);
}

// type declaration for knapsack problem
using knapsack = std::array<int, 5>;

// print helper for knapsack
inline std::ostream& operator<<(std::ostream& out, const knapsack& ks) {
    out << "[ ";
    std::ranges::copy_n(ks.begin(), static_cast<long long>(ks.size()) - 1,
                        std::ostream_iterator<int>(out, ", "));
    // print last element
    out << ks[ks.size() - 1] << " ]";
    return out;
}

TEST_CASE("Knapsack problem") {
    // knapsack problem as described here: https://en.wikipedia.org/wiki/Knapsack_problem

    struct knapsack_box {
        int value;
        int weight;
        auto operator<=>(const knapsack_box&) const = default;
    };

    // weight capacity of our knapsack
    constexpr auto max_weight = 15;

    // available boxes for the knapsack
    std::vector<knapsack_box> available_items = {{4, 12}, {2, 1}, {10, 4}, {1, 1}, {2, 2}};

    // fitness evaluator
    auto fitness = [&available_items](const knapsack ks) -> int {
        auto value_sum = 0;
        auto weight_sum = 0;
        for (const auto& index : ks) {
            if (index >= 0 && index < static_cast<int>(available_items.size())) {
                value_sum += available_items[index].value;
                weight_sum += available_items[index].weight;
            }
        }
        // if the weight is less than the max, it adds to the value
        if (weight_sum > max_weight) value_sum -= 25 * std::abs(weight_sum - max_weight);
        return value_sum;
    };

    std::random_device device;
    std::mt19937 engine(device());

    // random mutation operator
    auto mutator = [&engine, &available_items](const knapsack& ks) {
        knapsack output = ks;

        std::uniform_int_distribution<std::size_t> distribution(0, ks.size() - 1);
        const auto index = distribution(engine);

        if (std::ranges::count(output, -1) > 0) {
            // the output has some empty spaces, so there is the potential for us to add a new
            // number to the output
            assert(!available_items.empty());
            std::uniform_int_distribution<std::size_t> item_dist(0, available_items.size() - 1);
            auto new_value = item_dist(engine);
            // only allow unique values
            while (std::ranges::find(output, new_value) != std::end(output)) {
                new_value = item_dist(engine);
            }

            output[index] = static_cast<int>(new_value);
        } else {
            // the output already has unique numbers in it, so we'll just shuffle it
            std::ranges::shuffle(output, engine);
        }

        return output;
    };

    // crossover operator (i.e. child generator)
    auto crossover = [](const knapsack& first, const knapsack& second) {
        knapsack child{};
        std::ranges::fill(child, -1);

        auto first_copy_end = first.begin() + 3;
        const auto first_negative = std::ranges::find(first, -1);
        if (first_negative != first.end() && first_negative < first_copy_end) {
            first_copy_end = first_negative;
        }

        // copy first elements over.
        std::ranges::copy(first.begin(), first_copy_end, child.begin());

        // find the first negative value in the child knapsack (i.e. it's empty)
        auto child_first_negative = std::ranges::find(child, -1);

        // we need to copy from child_first_negative the first "negative_number_count" numbers that
        // are not already in the child knapsack
        for (const auto& value : second) {
            if (child_first_negative == child.end()) break;
            if (std::ranges::find(child, value) == child.end()) {
                *child_first_negative = value;
                child_first_negative += 1;
            }
        }
        return child;
    };

    // check that the crossover function works correctly
    const knapsack p1 = {1, -1, -1, -1, -1};
    const knapsack p2 = {0, 2, 3, -1, -1};
    const knapsack p3 = {0, 1, -1, -1, -1};
    const knapsack p4 = {0, 1, 2, 3, -1};
    auto c1 = crossover(p1, p2);
    auto c2 = crossover(p2, p1);
    auto c3 = crossover(p2, p4);
    auto c4 = crossover(p3, p4);

    CHECK(c1 == knapsack({1, 0, 2, 3, -1}));
    CHECK(c2 == knapsack({0, 2, 3, 1, -1}));
    CHECK(c3 == knapsack({0, 2, 3, 1, -1}));
    CHECK(c4 == knapsack({0, 1, 2, 3, -1}));

    // the solution is all the boxes except for the heaviest one.
    const knapsack solution = {-1, 1, 2, 3, 4};
    const knapsack all_items = {0, 1, 2, 3, 4};

    // assert that the actual solution has the highest fitness value.
    // this needs to be true for the algorithm to converge correctly.
    CHECK(fitness(solution) > fitness(all_items));

    // genetic algorithm settings.
    constexpr dp::genetic::algorithm_settings settings{0.1, 0.5, 0.25};

    // generate an initial random population
    constexpr auto population_size = 2;
    std::vector<knapsack> initial_population{};  // TODO: Generate initial population
    initial_population.reserve(population_size);

    // random length uniform distribution
    std::uniform_int_distribution length_dist(1, 4);

    // random value uniform distribution
    std::uniform_int_distribution<std::size_t> values_dist(0, available_items.size() - 1);

    // generator lambda
    auto knapsack_generator = [&engine, &length_dist, &values_dist]() {
        knapsack basic;
        std::ranges::fill(basic, -1);

        const auto random_length = length_dist(engine);
        for (auto i = 0; i < random_length; ++i) {
            auto value = values_dist(engine);
            // only allow unique values
            while (std::ranges::find(basic, value) != std::end(basic)) {
                value = values_dist(engine);
            }
            basic[i] = static_cast<int>(value);
        }
        return basic;
    };

    // generate the population
    std::ranges::generate_n(std::back_inserter(initial_population), population_size,
                            knapsack_generator);

    // define the termination criteria
    auto termination = dp::genetic::fitness_termination_criteria(fitness(solution));

    static_assert(
        dp::genetic::concepts::termination_operator<dp::genetic::fitness_termination_criteria,
                                                    knapsack, int>);

    static_assert(
        dp::genetic::concepts::selection_operator<dp::genetic::rank_selection, knapsack,
                                                  std::vector<knapsack>, decltype(fitness)>);

    auto params = dp::genetic::params<knapsack>::builder()
                      .with_mutation_operator(mutator)
                      .with_crossover_operator(crossover)
                      .with_fitness_operator(fitness)
                      .with_termination_operator(termination)
                      .build();

    dp::genetic_algorithm genetics(settings, params);

    auto start = std::chrono::steady_clock::now();
    auto [best, _] = genetics.solve(initial_population, [](auto& stats) {
        std::cout << "best: " << stats.current_best.best
                  << " fitness: " << stats.current_best.fitness << "\n";
    });
    auto stop = std::chrono::steady_clock::now();
    auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    std::cout << "Total time (ms): " << std::to_string(time_ms) << "\n";
    // sort the best to match against the solution
    std::ranges::sort(best);
    CHECK(best == solution);
}
