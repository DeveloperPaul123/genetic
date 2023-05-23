#pragma once

#include <thread_pool/thread_pool.h>

#include <algorithm>
#include <concepts>
#include <functional>
#include <iterator>
#include <numeric>
#include <ranges>
#include <vector>

#include "genetic/details/concepts.h"
#include "genetic/params.h"
#include "genetic/selection.h"

namespace dp {

    namespace genetic {
        /// @brief Settings type for probabilities
        struct algorithm_settings {
            double elitism = 0.0;
            double mutation_rate = 0.5;
            double crossover_rate = 0.2;
        };
    }  // namespace genetic

    template <typename ChromosomeType, typename PopulationType = std::vector<ChromosomeType>>
        requires std::default_initializable<ChromosomeType> &&
                 dp::genetic::concepts::population<PopulationType, ChromosomeType>
    class genetic_algorithm {
      public:
        using value_type = ChromosomeType;
        using list_type = std::vector<value_type>;

        /// @brief Result type that holds the best result
        struct results {
            value_type best;
            double fitness{};
        };

        struct iteration_statistics {
            results current_best;
            std::size_t current_generation_count{};
            std::size_t population_size{};
        };

        genetic_algorithm(
            genetic::algorithm_settings settings,
            dp::genetic::params<ChromosomeType, PopulationType> params,
            const std::size_t& number_of_threads = std::thread::hardware_concurrency())
            : params_(std::move(params)), settings_(settings),
              worker_pool_(static_cast<unsigned>(number_of_threads)) {}

        template <typename SelectorType = dp::genetic::roulette_selection,
                  typename IterationCallback = std::function<void(const iteration_statistics&)>>
            requires std::invocable<IterationCallback, const iteration_statistics&> &&
                     dp::genetic::concepts::selection_operator<
                         SelectorType, ChromosomeType, PopulationType,
                         typename dp::genetic::params<ChromosomeType,
                                                      PopulationType>::fitness_evaluation_type>
        [[nodiscard]] results solve(
            const PopulationType& initial_population,
            const IterationCallback& callback = [](const iteration_statistics&) {}) {
            population current_population;
            using params = dp::genetic::params<ChromosomeType, PopulationType>;

            auto termination_ = params_.termination_operator();
            auto crossover_ = params_.crossover_operator();
            auto mutator_ = params_.mutation_operator();
            auto fitness_ = params_.fitness_operator();

            // initialize our population
            std::ranges::transform(initial_population, std::back_inserter(current_population),
                                   [&](ChromosomeType value) {
                                       return chromosome_metadata{value, fitness_(value)};
                                   });
            // sort by fitness
            std::ranges::sort(current_population, fitness_sort_op{});

            auto best_element = *std::ranges::max_element(
                current_population, [](const std::pair<ChromosomeType, double>& first,
                                       const std::pair<ChromosomeType, double>& second) {
                    return std::get<double>(first) < std::get<double>(second);
                });

            iteration_statistics stats{};
            stats.current_best.best = std::get<ChromosomeType>(best_element);

            while (!termination_(std::get<ChromosomeType>(best_element),
                                 std::get<double>(best_element))) {
                auto number_elitism = static_cast<std::size_t>(
                    std::round(static_cast<double>(current_population.size()) * settings_.elitism));
                // perform elitism selection if it is enabled
                if (number_elitism == 0 && settings_.elitism > 0.0) number_elitism = 2;
                // generate elite population
                auto elite_population = elitism(current_population, number_elitism);

                // cross over
                auto crossover_number = static_cast<std::size_t>(std::round(
                    static_cast<double>(current_population.size()) * settings_.crossover_rate));
                if (crossover_number <= 1) crossover_number = 4;

                // create a new "generation", we will also insert the elite population into this
                // one.
                std::vector<std::future<std::pair<chromosome_metadata, chromosome_metadata>>>
                    future_results{};
                future_results.reserve(crossover_number);

                for (std::size_t i = 0; i < crossover_number; i++) {
                    std::future<std::pair<chromosome_metadata, chromosome_metadata>> future =
                        worker_pool_.enqueue(
                            [](typename params::crossover_operator_type crossover,
                               typename params::fitness_evaluation_type fitness,
                               typename params::mutation_operator_type mutator,
                               const population& pop)
                                -> std::pair<chromosome_metadata, chromosome_metadata> {
                                static thread_local SelectorType selector{};

                                // randomly select 2 parents
                                auto chromosomes_only_view = pop | std::views::elements<0>;
                                const auto& [parent1, parent2] =
                                    selector(chromosomes_only_view, fitness);

                                // generate two children from each parent sets
                                auto child1 = crossover(parent1, parent2);
                                auto child2 = crossover(parent2, parent1);

                                child1 = mutator(child1);
                                child2 = mutator(child2);

                                return std::make_pair(std::make_pair(child1, fitness(child1)),
                                                      std::make_pair(child2, fitness(child2)));
                            },
                            crossover_, fitness_, mutator_, current_population);
                    future_results.emplace_back(std::move(future));
                }

                population crossover_population;
                crossover_population.reserve(crossover_number * 2 + elite_population.size());

                for (auto& result : future_results) {
                    auto [child1, child2] = result.get();
                    crossover_population.push_back(child1);
                    crossover_population.push_back(child2);
                }

                // add elite population
                crossover_population.insert(crossover_population.end(), elite_population.begin(),
                                            elite_population.end());

                // sort crossover population by fitness (lowest first)
                std::ranges::sort(crossover_population, fitness_sort_op{});

                // reset the current population
                current_population.clear();
                // assign/move the new generation
                current_population = std::move(crossover_population);

                // update the best element
                best_element = *std::ranges::max_element(current_population);

                // send callback stats for each generation
                stats.current_best.best = std::get<ChromosomeType>(best_element);
                stats.current_best.fitness = std::get<double>(best_element);
                stats.population_size = current_population.size();
                ++stats.current_generation_count;
                callback(std::add_const_t<iteration_statistics>(stats));
            }

            return {std::get<ChromosomeType>(best_element), std::get<double>(best_element)};
        }

      private:
        dp::genetic::params<ChromosomeType, PopulationType> params_;

        using chromosome_metadata = std::pair<ChromosomeType, double>;
        using population = std::vector<chromosome_metadata>;

        template <typename Comparator = std::less<>>
        struct fitness_sort_op {
            template <typename T>
            bool operator()(const T& first, const T& second) {
                Comparator cmp;
                return cmp(std::get<double>(first), std::get<double>(second));
            }
        };

        dp::genetic::algorithm_settings settings_{};
        dp::thread_pool<> worker_pool_{};

        static auto elitism(population& current_population, std::size_t number_elitism) {
            // perform elitism selection

            // sort so that largest fitness item is at front
            std::ranges::partial_sort(current_population,
                                      current_population.begin() +
                                          std::min(current_population.size(), number_elitism + 1),
                                      fitness_sort_op<std::greater<>>{});

            // select the first n in the current population
            return std::ranges::views::take(current_population, number_elitism);
        }
    };
}  // namespace dp
