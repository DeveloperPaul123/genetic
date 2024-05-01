#pragma once

#include <functional>
#include <ranges>

#include "crossover.h"
#include "details/concepts.h"
#include "fitness.h"
#include "mutation.h"
#include "selection.h"
#include "termination.h"

namespace dp::genetic {
    template <typename ChromosomeType, typename PopulationType = std::vector<ChromosomeType>>
        requires dp::genetic::concepts::population<PopulationType, ChromosomeType>
    class params {
      public:
        /// @brief Type definitions
        /// @{
        using value_type = ChromosomeType;
        using population_type = PopulationType;
        using mutation_operator_type = std::function<ChromosomeType(ChromosomeType)>;
        using crossover_operator_type =
            std::function<ChromosomeType(ChromosomeType, ChromosomeType)>;
        using fitness_evaluation_type = std::function<double(ChromosomeType)>;
        using termination_evaluation_type = std::function<bool(ChromosomeType, double)>;
        using selection_operator_type = std::function<std::pair<ChromosomeType, ChromosomeType>(
            PopulationType, fitness_evaluation_type)>;
        /// @}

        template <class FitnessOperator = details::accumulation_fitness_op,
                  class MutationOperator = no_op_mutator,
                  class CrossoverOperator = random_crossover,
                  class TerminationOperator = generations_termination,
                  class SelectionOperator = roulette_selection>
            requires concepts::mutation_operator<MutationOperator, ChromosomeType> &&
                         concepts::fitness_operator<FitnessOperator, ChromosomeType> &&
                         concepts::crossover_operator<CrossoverOperator, ChromosomeType> &&
                         concepts::termination_operator<
                             TerminationOperator, ChromosomeType,
                             std::invoke_result_t<FitnessOperator, ChromosomeType>> &&
                         concepts::selection_operator<SelectionOperator, ChromosomeType,
                                                      PopulationType, FitnessOperator>
        explicit params(FitnessOperator&& fitness = FitnessOperator{},
                        MutationOperator&& mutator = MutationOperator{},
                        TerminationOperator&& terminator = TerminationOperator{},
                        CrossoverOperator&& crosser = CrossoverOperator{},
                        SelectionOperator selection_operator = SelectionOperator{})
            : mutator_(std::forward<MutationOperator>(mutator)),
              crossover_(std::forward<CrossoverOperator>(crosser)),
              fitness_(std::forward<FitnessOperator>(fitness)),
              termination_(std::forward<TerminationOperator>(terminator)),
              selection_(std::forward<SelectionOperator>(selection_operator)) {}

        [[nodiscard]] auto&& fitness_operator() const { return fitness_; }
        [[nodiscard]] auto&& mutation_operator() const { return mutator_; }
        [[nodiscard]] auto&& crossover_operator() const { return crossover_; }
        [[nodiscard]] auto&& termination_operator() const { return termination_; }
        [[nodiscard]] auto&& selection_operator() const { return selection_; }
        /// @brief builder class that helps with parameter construction
        class builder {
          public:
            builder() = default;

            builder& with_fitness_operator(
                dp::genetic::concepts::fitness_operator<ChromosomeType> auto&& op) {
                data_.fitness_ = op;
                return *this;
            }

            builder& with_mutation_operator(
                dp::genetic::concepts::mutation_operator<ChromosomeType> auto&& op) {
                data_.mutator_ = op;
                return *this;
            }

            template <typename Fn, dp::genetic::type_traits::number Numeric = double>
                requires dp::genetic::concepts::termination_operator<Fn, ChromosomeType, Numeric>
            builder& with_termination_operator(Fn&& op) {
                data_.termination_ = std::forward<Fn>(op);
                return *this;
            }

            builder& with_crossover_operator(
                dp::genetic::concepts::crossover_operator<ChromosomeType> auto&& op) {
                data_.crossover_ = std::forward<decltype(op)>(op);
                return *this;
            }

            // TODO enable CTAD here
            template <typename UnaryOp>
            builder& with_selection_operator(dp::genetic::concepts::selection_operator<
                                             ChromosomeType, PopulationType, UnaryOp> auto&& op) {
                data_.selection_ = std::forward<decltype(op)>(op);
                return *this;
            }

            [[nodiscard]] auto build() const { return data_; }

          private:
            params data_{};
        };

      private:
        friend class builder;

        mutation_operator_type mutator_;
        crossover_operator_type crossover_;
        fitness_evaluation_type fitness_;
        termination_evaluation_type termination_;
        selection_operator_type selection_;
    };
}  // namespace dp::genetic
