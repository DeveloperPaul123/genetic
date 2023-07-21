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
        namespace details {
            /**
             * @brief Fitness comparator for the population
             * @tparam Comparator compare operator (std::less, std::greater, etc)
             */
            template <typename Comparator = std::less<>>
            struct fitness_sort_op {
                template <typename T>
                bool operator()(const T& first, const T& second) {
                    Comparator cmp;
                    return cmp(std::get<double>(first), std::get<double>(second));
                }
            };
        }  // namespace details

        /// @brief Settings type for probabilities
        struct algorithm_settings {
            double elitism_rate = 0.0;
            double mutation_rate = 0.5;
            double crossover_rate = 0.2;
        };

        template <typename ChromosomeType>
        struct results {
            ChromosomeType best;
            double fitness{};
        };

        template <typename ChromosomeType>
        struct iteration_statistics {
            results<ChromosomeType> current_best;
            std::size_t current_generation_count{};
            std::size_t population_size{};
        };

        template <typename PopulationType,
                  typename ChromosomeType = std::ranges::range_value_t<PopulationType>,
                  typename IterationCallback =
                      std::function<void(const iteration_statistics<ChromosomeType>&)>>
            requires std::default_initializable<ChromosomeType> &&
                     concepts::population<PopulationType, ChromosomeType>
        results<ChromosomeType> solve(
            const PopulationType& initial_population, const algorithm_settings& settings,
            dp::genetic::params<ChromosomeType, PopulationType> parameters = {},
            const IterationCallback& callback = [](const iteration_statistics<ChromosomeType>&) {
            }) {
            using chromosome_metadata = std::pair<ChromosomeType, double>;
            using population = std::vector<chromosome_metadata>;
            using iteration_stats = iteration_statistics<ChromosomeType>;

            static dp::thread_pool worker_pool{};

            // Performs elitism selection on the current population
            auto elitism = [](population& current_population, std::size_t number_elitism) {
                // no elitism, so return empty population
                if (number_elitism == 0) return std::ranges::views::take(current_population, 0);

                // sort so that largest fitness item is at front
                std::ranges::partial_sort(
                    current_population,
                    current_population.begin() +
                        std::min(current_population.size(), number_elitism + 1),
                    details::fitness_sort_op<std::greater<>>{});

                // select the first n in the current population
                return std::ranges::views::take(current_population, number_elitism);
            };

            population current_population;
            // initialize our population
            std::ranges::transform(
                initial_population, std::back_inserter(current_population),
                [&](ChromosomeType value) {
                    return chromosome_metadata{value, parameters.fitness_operator()(value)};
                });
            // sort by fitness
            std::ranges::sort(current_population, details::fitness_sort_op{});

            auto best_element = *std::ranges::max_element(
                current_population, [](const std::pair<ChromosomeType, double>& first,
                                       const std::pair<ChromosomeType, double>& second) {
                    return std::get<double>(first) < std::get<double>(second);
                });

            iteration_stats stats{};
            stats.current_best.best = std::get<ChromosomeType>(best_element);

            while (!parameters.termination_operator()(std::get<ChromosomeType>(best_element),
                                                      std::get<double>(best_element))) {
                auto number_elitism = static_cast<std::size_t>(std::round(
                    static_cast<double>(current_population.size()) * settings.elitism_rate));
                // perform elitism selection if it is enabled
                if (number_elitism == 0 && settings.elitism_rate > 0.0) number_elitism = 2;
                // generate elite population
                auto elite_population = elitism(current_population, number_elitism);

                // cross over
                auto crossover_number = static_cast<std::size_t>(std::round(
                    static_cast<double>(current_population.size()) * settings.crossover_rate));
                if (crossover_number <= 1) crossover_number = 4;

                // create a new "generation", we will also insert the elite population into this one
                std::vector<std::future<std::pair<chromosome_metadata, chromosome_metadata>>>
                    future_results{};
                future_results.reserve(crossover_number);

                for (std::size_t i = 0; i < crossover_number; i++) {
                    std::future<std::pair<chromosome_metadata, chromosome_metadata>> future =
                        worker_pool.enqueue(
                            [](params<ChromosomeType, PopulationType> prms, const population& pop)
                                -> std::pair<chromosome_metadata, chromosome_metadata> {
                                // randomly select 2 parents
                                auto chromosomes_only_view = pop | std::views::elements<0> |
                                                             std::ranges::to<PopulationType>();
                                auto [parent1, parent2] = prms.selection_operator()(
                                    chromosomes_only_view, prms.fitness_operator());

                                // generate two children from each parent sets
                                auto child1 = prms.crossover_operator()(parent1, parent2);
                                auto child2 = prms.crossover_operator()(parent2, parent1);

                                // mutate the children
                                child1 = prms.mutation_operator()(child1);
                                child2 = prms.mutation_operator()(child2);

                                // return the result + their fitness
                                return std::make_pair(
                                    std::make_pair(child1, prms.fitness_operator()(child1)),
                                    std::make_pair(child2, prms.fitness_operator()(child2)));
                            },
                            parameters, current_population);
                    future_results.emplace_back(std::move(future));
                }

                population crossover_population;
                crossover_population.reserve(crossover_number * 2 + elite_population.size());

                for (auto& result : future_results) {
                    auto [child1, child2] = result.get();
                    crossover_population.push_back(child1);
                    crossover_population.push_back(child2);
                }

                if (!std::ranges::empty(elite_population)) {
                    // add elite population
                    crossover_population.insert(crossover_population.end(),
                                                elite_population.begin(), elite_population.end());
                }

                // sort crossover population by fitness (lowest first)
                std::ranges::sort(crossover_population, details::fitness_sort_op{});

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
                callback(std::add_const_t<iteration_stats>(stats));
            }

            return {std::get<ChromosomeType>(best_element), std::get<double>(best_element)};
        }
    }  // namespace genetic

}  // namespace dp
