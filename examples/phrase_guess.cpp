#include <genetic/crossover.h>
#include <genetic/fitness.h>
#include <genetic/genetic.h>
#include <genetic/selection.h>

#include <iostream>
#include <string>

struct random_word_generator {
    [[nodiscard]] std::string operator()(std::string_view char_set, std::size_t max_length) const {
        static thread_local auto generator = dp::genetic::uniform_integral_generator{};
        const auto& out_length = generator(static_cast<std::size_t>(1), max_length);
        std::string out(out_length, '\0');
        std::generate_n(out.begin(), out_length,
                        [&]() { return char_set[generator(std::size_t{0}, char_set.size() - 1)]; });
        return out;
    }
};

[[nodiscard]] bool is_space(char q) noexcept {
    static constexpr auto ws = {' ', '\t', '\n', '\v', '\r', '\f'};
    return std::ranges::any_of(ws, [q](auto p) { return p == q; });
};

const std::string alphabet = R"(abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!,. ,')";

int main(int argc, char** argv) {
    std::string solution;
    auto prompt = std::format("Acceptable characters: {}\nEnter a phrase: ", alphabet);
    std::cout << prompt;
    std::getline(std::cin, solution);

    constexpr auto is_space = [](const auto& c) { return std::isspace(c); };
    // filter and trim input to only include characters in the alphabet that we accept
    solution = solution | std::ranges::views::filter([](const auto& c) {
                   return std::ranges::find(alphabet, c) != std::end(alphabet);
               }) |
               std::ranges::views::drop_while(isspace) | std::views::reverse |
               std::ranges::views::drop_while(isspace) | std::views::reverse |
               std::ranges::to<std::string>();

    std::cout << std::format("Using filtered text: {}\n", solution);

    const std::string available_chars = alphabet;
    const auto max_word_length = solution.size() + solution.size() / 2;

    // generate random words as our initial population
    random_word_generator word_generator{};
    // generate initial population
    constexpr auto initial_pop_size = 1000;
    std::vector<std::string> initial_population(initial_pop_size);
    std::generate_n(initial_population.begin(), initial_pop_size,
                    [&] { return word_generator(available_chars, max_word_length); });

    // fitness evaluator
    dp::genetic::element_wise_comparison fitness_op(solution, 1.0);

    auto string_mutator = dp::genetic::composite_mutator{
        [&](const std::string& input) {
            if (input.empty()) {
                return word_generator(available_chars, max_word_length);
            }
            return input;
        },
        dp::genetic::value_replacement_mutator<std::string>{available_chars}};

    // termination criteria
    auto termination = dp::genetic::fitness_termination(fitness_op(solution));

    static_assert(
        dp::genetic::concepts::selection_operator<dp::genetic::rank_selection, std::string,
                                                  std::vector<std::string>, decltype(fitness_op)>);
    // algorithm settings
    dp::genetic::algorithm_settings settings{0.3, 0.6, 0.3};
    dp::genetic::params<std::string> params =
        dp::genetic::params<std::string>::builder()
            .with_mutation_operator(string_mutator)
            .with_crossover_operator(dp::genetic::default_crossover{})
            .with_fitness_operator(fitness_op)
            .with_termination_operator(termination)
            .build();

    auto start = std::chrono::steady_clock::now();
    auto [best, fitness] =
        dp::genetic::solve(initial_population, settings, params, [](const auto& stats) {
            std::cout << "best: " << stats.current_best.best
                      << " fitness: " << stats.current_best.fitness << "\n";
        });
    auto stop = std::chrono::steady_clock::now();
    auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    std::cout << "Total time (ms): " << std::to_string(time_ms) << "\n";

    return 0;
}
