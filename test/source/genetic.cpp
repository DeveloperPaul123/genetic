#include <doctest/doctest.h>
#include <genetic/crossover.h>
#include <genetic/details/concepts.h>
#include <genetic/genetic.h>
#include <genetic/mutation.h>
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

// type declaration for knapsack problem
// declared here to be used in ostream operator
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

    // available boxes for the knapsack : {value, weight}
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

    auto engine = dp::genetic::details::initialize_random_engine<std::mt19937>();

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
    std::vector<knapsack> initial_population{};
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

    // generate the initial population
    std::ranges::generate_n(std::back_inserter(initial_population), population_size,
                            knapsack_generator);

    // define the termination criteria
    auto termination = dp::genetic::fitness_termination(fitness(solution));

    static_assert(dp::genetic::concepts::termination_operator<dp::genetic::fitness_termination,
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

    auto start = std::chrono::steady_clock::now();
    auto [best, _] = dp::genetic::solve(initial_population, settings, params, [](auto& stats) {
        std::cout << "best: " << stats.current_best.best
                  << " fitness: " << stats.current_best.fitness
                  << " pop size: " << std::to_string(stats.current_generation_count) << "\n";
    });
    auto stop = std::chrono::steady_clock::now();
    auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    std::cout << "Total time (ms): " << std::to_string(time_ms) << "\n";
    // sort the best to match against the solution
    std::ranges::sort(best);
    CHECK(best == solution);
}

TEST_CASE("Beale function") {
    // define our data type as a 2D "vector"
    using data_t = std::array<double, 2>;
    const auto fitness = [](const data_t& value) -> double {
        const auto x = value[0];
        const auto y = value[1];
        // multiply by -1 to maximize the function
        const auto xy = x * y;
        const auto y_squared = std::pow(y, 2);
        const auto y_cubed = std::pow(y, 3);
        return -(std::pow(1.5 - x + xy, 2) + std::pow(2.25 - x + x * y_squared, 2) +
                 std::pow(2.625 - x + x * y_cubed, 2));
    };

    CHECK(fitness({3, 0.5}) == doctest::Approx(0.0).epsilon(0.01));

    dp::genetic::uniform_floating_point_generator generator{};
    auto generate_value = [&generator] { return generator(-4.5, 4.5); };

    std::vector<data_t> initial_population;

    // generate our initial population
    std::ranges::generate_n(std::back_inserter(initial_population), 10'000, [&generate_value]() {
        return std::array{generate_value(), generate_value()};
    });

    constexpr double increment = 0.00001;

    auto mutator = [increment](const data_t& value) -> data_t {
        thread_local dp::genetic::uniform_floating_point_generator generator{};
        const auto [x, y] = value;
        return std::array{std::clamp(x + generator(-increment, increment), -4.5, 4.5),
                          std::clamp(y + generator(-increment, increment), -4.5, 4.5)};
    };

    // if fitness doesn't change a significant amount in 30 generations, terminate
    auto termination = dp::genetic::fitness_hysteresis{1.e-8, 30};
    // auto termination = dp::genetic::generations_termination{50'000};
    const auto params = dp::genetic::params<data_t>::builder()
                            .with_fitness_operator(fitness)
                            .with_mutation_operator(mutator)
                            .with_crossover_operator(dp::genetic::default_crossover{})
                            .with_termination_operator(termination)
                            .build();

    const auto [best, _] = dp::genetic::solve(
        initial_population, dp::genetic::algorithm_settings{.elitism_rate = 0.25}, params,
        [](auto& stats) {
            std::cout << std::format("best: [{}, {}]", stats.current_best.best[0],
                                     stats.current_best.best[1])
                      << " fitness: " << stats.current_best.fitness
                      << " pop size: " << std::to_string(stats.current_generation_count) << "\n";
        });

    const auto [x, y] = best;
    CHECK(x == doctest::Approx(3.0).epsilon(0.001));
    CHECK(y == doctest::Approx(0.5).epsilon(0.001));
}
