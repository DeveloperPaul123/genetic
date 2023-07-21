#include <genetic/details/concepts.h>
#include <genetic/fitness.h>

#include <vector>

static_assert(
    dp::genetic::concepts::fitness_operator<dp::genetic::accumulation_fitness, std::string>);
static_assert(dp::genetic::concepts::fitness_operator<
              dp::genetic::element_wise_comparison<std::string>, std::string>);
static_assert(dp::genetic::concepts::fitness_operator<
              dp::genetic::element_wise_comparison<std::vector<int>>, std::vector<int>>);
