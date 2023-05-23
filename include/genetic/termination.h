#pragma once

namespace dp::genetic {

    struct generations_termination_criteria {
        explicit generations_termination_criteria(std::size_t max_generations = 1000)
            : count_(max_generations) {}
        template <typename T>
        bool operator()(T, double) {
            count_--;
            return count_ == 0;
        }

      private:
        std::size_t count_;
    };

    struct fitness_termination_criteria {
        explicit fitness_termination_criteria(double target_fitness) : fitness_(target_fitness) {}
        template <typename T>
        bool operator()(T, double fitness) {
            return fitness >= fitness_;
        }

      private:
        double fitness_{};
    };
}  // namespace dp::genetic
