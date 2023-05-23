#include <doctest/doctest.h>
#include <genetic/params.h>

#include <numeric>
#include <string>
#include <vector>

TEST_CASE("Testing constructing genetic params") {
    dp::genetic::params<std::vector<double>> pars(
        [](std::vector<double> val) -> double {
            return std::accumulate(val.begin(), val.end(), 0.0);
        },
        [](const std::vector<double>&) -> std::vector<double> { return {}; });
}

TEST_CASE("Create params with builder") {
    const auto genetic_params = dp::genetic::params<std::string>::builder()
                                    .with_fitness_operator([](const std::string&) { return 0.0; })
                                    .build();

    CHECK(genetic_params.fitness_operator()("") == 0.0);
}
