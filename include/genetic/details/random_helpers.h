#pragma once
#include <concepts>
#include <random>
#include <ranges>

namespace dp::genetic {
    namespace details {
        template <class T,
                  std::size_t NumberOfSeeds = T::state_size * sizeof(typename T::result_type)>
        T initialize_random_engine() {
            std::random_device source;
            auto random_data =
                std::views::iota(std::size_t(), (NumberOfSeeds - 1) / sizeof(source()) + 1) |
                std::views::transform([&](auto) { return source(); });
            std::seed_seq seeds(std::begin(random_data), std::end(random_data));
            return T(seeds);
        }

        template <std::uniform_random_bit_generator T>
        T initialize_random_engine(const std::seed_seq &seed_seq) {
            return T(seed_seq);
        }
    }  // namespace details

    // TODO: Add a way to inject a random device and/or how to initialize it
    struct uniform_integral_generator {
        template <std::integral T, std::uniform_random_bit_generator RandomDevice = std::mt19937>
        auto operator()(const T &lower_bound, const T &upper_bound) {
            // generate random crossover points
            thread_local auto device = details::initialize_random_engine<RandomDevice>();
            std::uniform_int_distribution<T> dist(lower_bound, upper_bound);
            return dist(device);
        }
    };

    // TODO: Add a way to inject a random device and/or how to initialize it
    struct uniform_floating_point_generator {
        template <std::floating_point T,
                  std::uniform_random_bit_generator RandomDevice = std::mt19937>
        auto operator()(const T &lower_bound, const T &upper_bound) {
            thread_local auto device = details::initialize_random_engine<RandomDevice>();
            std::uniform_real_distribution<T> dist(lower_bound, upper_bound);
            return dist(device);
        }
    };
}  // namespace dp::genetic
