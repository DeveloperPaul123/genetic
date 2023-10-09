#pragma once
#include <concepts>
#include <ranges>
#include <type_traits>
#include <utility>

namespace dp::genetic {

    namespace type_traits {
        template <typename T>
        using element_type_t =
            std::remove_reference_t<decltype(*std::ranges::begin(std::declval<T &>()))>;

        template <typename T>
        constexpr inline bool is_std_array = false;

        template <typename T, std::size_t N>
        constexpr inline bool is_std_array<std::array<T, N>> = true;

        template <typename T, typename SimpleType = std::remove_cvref_t<T>>
        concept has_value_type = requires(SimpleType) { typename SimpleType::value_type; };

        template <typename T, typename SimpleType = std::remove_cvref_t<T>>
        concept has_size_type = requires(SimpleType) { typename SimpleType::size_type; };

        template <typename T, typename SimpleType = std::remove_cv_t<T>,
                  typename SizeType = typename SimpleType::size_type>
        concept has_size = has_size_type<T> && requires(T &&t) {
            { t.size() } -> std::convertible_to<SizeType>;
        };

        template <typename T, typename SimpleType = std::remove_cvref_t<T>,
                  typename ValueType = typename SimpleType::value_type>
        concept has_push_back = has_value_type<SimpleType> &&
                                requires(SimpleType &&t, ValueType &&value) { t.push_back(value); };

        template <typename T, typename SimpleType = std::remove_cvref_t<T>,
                  typename SizeType = typename SimpleType::size_type>
        concept has_reserve = has_size_type<SimpleType> && std::integral<SizeType> &&
                              requires(SimpleType &&t, SizeType value) { t.reserve(value); };

        template <typename T, typename SimpleType = std::remove_cvref_t<T>>
        concept has_empty = requires(SimpleType &&t) {
            { t.empty() } -> std::convertible_to<bool>;
        };

        template <typename T>
        concept number = std::integral<T> || std::floating_point<T>;

        template <typename T, typename T2 = T>
        concept addable = requires(T first, T2 second) {
            { first + second } -> std::convertible_to<T>;
        };

        template <typename T, typename T2 = T>
        concept subtractable = requires(T first, T2 second) {
            { first + second } -> std::convertible_to<T>;
        };
    }  // namespace type_traits

    namespace concepts {
        /// @brief Custom concepts needed for genetic algorithm class
        /// @{
        template <class Fn, class T>
        concept mutation_operator =
            std::invocable<Fn, T> && std::is_same_v<std::invoke_result_t<Fn, T>, T>;

        template <class Fn, class T>
        concept fitness_operator = std::invocable<Fn, T> && requires(Fn fn) {
            { fn(std::declval<T>()) } -> type_traits::number;
        };

        template <class Fn, class T, class SimpleType = std::remove_cvref_t<T>,
                  class Result = std::invoke_result_t<Fn, T, T>>
        concept crossover_operator =
            std::invocable<Fn, T, T> && std::is_convertible_v<Result, SimpleType>;

        template <class Fn, class T, class Numeric,
                  class Result = std::invoke_result_t<Fn, T, Numeric>>
        concept termination_operator =
            std::invocable<Fn, T, Numeric> && dp::genetic::type_traits::number<Numeric> &&
            std::convertible_to<Result, bool>;

        template <class Fn, class T, class Container, class UnaryOp>
        concept selection_operator =
            std::invocable<Fn, Container, UnaryOp> &&
            std::is_same_v<std::invoke_result_t<Fn, Container, UnaryOp>, std::pair<T, T>>;

        template <typename T, typename Index = std::size_t>
        concept index_generator =
            std::integral<std::remove_cvref_t<Index>> && requires(T &&t, Index &&l, Index &&u) {
                { t(l, u) } -> std::convertible_to<std::remove_cvref_t<Index>>;
            };

        template <typename Fn, typename ValueType>
        concept value_generator =
            std::invocable<Fn> && std::convertible_to<std::invoke_result_t<Fn>, ValueType>;

        template <typename Range, typename Chromosome,
                  typename T = type_traits::element_type_t<Range>>
        concept population = std::ranges::range<Range> && std::is_same_v<Chromosome, T>;
        /// @}
    }  // namespace concepts

}  // namespace dp::genetic
