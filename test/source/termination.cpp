#include <genetic/details/concepts.h>
#include <genetic/termination.h>

static_assert(dp::genetic::concepts::termination_operator<
              dp::genetic::generations_termination_criteria, std::string, double>);
static_assert(dp::genetic::concepts::termination_operator<dp::genetic::fitness_termination_criteria,
                                                          std::string, double>);
