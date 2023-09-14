#pragma once
#include <functional>

namespace dp::genetic {

    namespace details {

        template <typename TupleT, typename Fn, std::size_t... Is>
        void for_each_tuple_impl(TupleT&& tp, Fn&& fn, std::index_sequence<Is...>) {
            (fn(std::get<Is>(std::forward<TupleT>(tp))), ...);
        }

        template <typename TupleT, typename Fn,
                  std::size_t TupSize = std::tuple_size_v<std::remove_cvref_t<TupleT>>>
        void for_each_tuple(TupleT&& tp, Fn&& fn) {
            for_each_tuple_impl(std::forward<TupleT>(tp), std::forward<Fn>(fn),
                                std::make_index_sequence<TupSize>{});
        }
        /**
         * @brief A composite fitness function that allows you to chain together multiple fitness
         * functions with a custom binary operator to combine the results. By default, it uses
         * std::plus<>.
         */
        template <typename... Args>
        struct composite_fitness {
          private:
            // list of fitness evaluators
            std::tuple<Args...> fitness_ops_;

          public:
            explicit composite_fitness(Args... args)
                requires(sizeof...(Args) > 0)  // must have at least 1 operator
                : fitness_ops_(std::forward<Args>(args)...) {}
            template <typename BinaryOperator, std::ranges::range Range,
                      typename ScoreType = std::invoke_result_t<
                          std::decay_t<std::tuple_element_t<0, std::tuple<Args...>>>, Range>>
            ScoreType evaluate(const Range& data) {
                // initialize the result to the first fitness function
                ScoreType result = std::invoke(std::get<0>(fitness_ops_), data);

                // loop over the rest of them if there are more
                if constexpr (sizeof...(Args) > 1) {
                    auto rest_of_tuples = std::apply(
                        [&](auto&& head, auto&&... tail) {
                            // ignore the head and return tail
                            return std::make_tuple(std::forward<decltype(tail)>(tail)...);
                        },
                        fitness_ops_);

                    details::for_each_tuple(rest_of_tuples, [&](auto&& op) {
                        // update result based on the previous result and the new score
                        result = std::invoke(BinaryOperator{}, result, std::invoke(op, data));
                    });
                }

                return result;
            }
        };

        template <typename... Args>
        struct composite_base {
          private:
            using type = composite_fitness<Args...>;
            static constexpr type make_argument_tuple(Args... args) {
                return type(std::forward<Args>(args)...);
            }

          protected:
            type fitness_;

          public:
            explicit composite_base(Args... args) : fitness_{make_argument_tuple(args...)} {}
        };

    }  // namespace details

    template <typename... Args>
    struct composite_sum_fitness : details::composite_base<Args...> {
        explicit composite_sum_fitness(Args... args) : details::composite_base<Args...>(args...) {}
        template <std::ranges::range Range,
                  typename ScoreType = std::invoke_result_t<
                      std::decay_t<std::tuple_element_t<0, std::tuple<Args...>>>, Range>>
        ScoreType operator()(const Range& data) {
            return this->fitness_.template evaluate<std::plus<ScoreType>>(data);
        }
    };

    template <typename... Args>
    struct composite_difference_fitness : details::composite_base<Args...> {
        explicit composite_difference_fitness(Args... args)
            : details::composite_base<Args...>(args...) {}
        template <std::ranges::range Range,
                  typename ScoreType = std::invoke_result_t<
                      std::decay_t<std::tuple_element_t<0, std::tuple<Args...>>>, Range>>
        ScoreType operator()(const Range& data) {
            return this->fitness_.template evaluate<std::minus<ScoreType>>(data);
        }
    };

    template <typename... Args>
    struct composite_product_fitness : details::composite_base<Args...> {
        explicit composite_product_fitness(Args... args)
            : details::composite_base<Args...>(args...) {}
        template <std::ranges::range Range,
                  typename ScoreType = std::invoke_result_t<
                      std::decay_t<std::tuple_element_t<0, std::tuple<Args...>>>, Range>>
        ScoreType operator()(const Range& data) {
            return this->fitness_.template evaluate<std::multiplies<ScoreType>>(data);
        }
    };

}  // namespace dp::genetic
