#include <genetic/details/concepts.h>

#include <string>
#include <vector>

using namespace dp::genetic::type_traits;

/// @brief type traits concepts tests
/// @{

// value type
static_assert(has_value_type<std::vector<int>>);
static_assert(has_value_type<const std::vector<int> &>);

// size_type
static_assert(has_size_type<std::vector<int>>);
static_assert(has_size_type<std::string>);

// size
static_assert(has_size<std::vector<int>>);
static_assert(has_size<std::string>);

// has push back
static_assert(has_push_back<std::vector<int>>);
static_assert(has_push_back<std::string>);

// has reserve
static_assert(has_reserve<std::vector<int>>);
static_assert(has_reserve<std::string>);

// has empty
static_assert(has_empty<std::vector<int>>);
static_assert(has_empty<std::string>);
static_assert(!has_empty<int>);

// number concept
static_assert(number<int>);
static_assert(number<std::uint16_t>);
static_assert(number<std::uint32_t>);
static_assert(number<unsigned int>);
static_assert(number<float>);
static_assert(number<long double>);
static_assert(number<long double>);

// addable
static_assert(addable<int>);
static_assert(addable<double>);
static_assert(addable<float>);
static_assert(addable<unsigned int>);
static_assert(addable<std::string>);
static_assert(!addable<std::vector<int>>);

struct sample {
    std::int32_t first{};
    std::int64_t second{};
};

sample operator+(const sample &first, const sample &second) {
    return {.first = first.first + second.first, .second = first.second + second.second};
}

static_assert(addable<sample>);

/// @}