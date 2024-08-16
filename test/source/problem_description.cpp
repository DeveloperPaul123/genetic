#include <doctest/doctest.h>
#include <genetic/crossover.h>
#include <genetic/details/concepts.h>
#include <genetic/fitness.h>
#include <genetic/mutation.h>
#include <genetic/problem_description.h>

TEST_CASE("Make problem description") {
    auto fitness = dp::genetic::accumulation_fitness;
    auto mutation = dp::genetic::no_op_mutator{};
    auto crossover = dp::genetic::random_crossover{};

    auto problem_description = dp::genetic::make_problem_description(fitness, mutation, crossover);

    static_assert(
        dp::genetic::concepts::fitness_operator<decltype(std::get<0>(problem_description)),
                                                std::vector<int>>);
                                                
}
