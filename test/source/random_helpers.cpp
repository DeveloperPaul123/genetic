#include <genetic/details/concepts.h>
#include <genetic/details/random_helpers.h>

// index generator concept
static_assert(dp::genetic::concepts::index_generator<dp::genetic::uniform_integral_generator>);
