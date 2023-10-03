#include <doctest/doctest.h>
#include <genetic/details/concepts.h>
#include <genetic/termination.h>

TEST_CASE("Generations termination") {
    std::string chromosome{};
    auto termination = dp::genetic::generations_termination(1234);
    std::uint64_t count{1};
    while (!dp::genetic::should_terminate(termination, chromosome, 0.0)) {
        ++count;
    }

    CHECK_EQ(count, termination.max_generations);

    auto fitness_term = dp::genetic::fitness_termination{100.};
    CHECK(dp::genetic::should_terminate(fitness_term, chromosome, 110.0));
    CHECK_FALSE(dp::genetic::should_terminate(fitness_term, chromosome, 99.99));
}
