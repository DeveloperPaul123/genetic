<h1 align=center>
genetic
</h1>

[![say thanks](https://img.shields.io/badge/Say%20Thanks-👍-1EAEDB.svg)](https://github.com/DeveloperPaul123/genetic/stargazers)
[![Discord](https://img.shields.io/discord/652515194572111872)](https://discord.gg/CX2ybByRnt)

A flexible and performant implementation of the genetic algorithm in C++20/23.

## Features

* Built entirely with C++20/23
* Supply your own functions for:
    * Selection
    * Crossover
    * Mutation
    * Fitness

## Integration

`dp::genetic` is a header only library. All the files needed are in `include/genetic`. 

### CMake

`genetic` defines two CMake targets:

* `Genetic::Genetic`
* `dp::genetic`

You can then use `find_package()`:

```cmake
find_package(dp::genetic REQUIRED)
```

Alternatively, you can use something like [CPM](https://github.com/TheLartians/CPM) which is based on CMake's `Fetch_Content` module.

```cmake
CPMAddPackage(
    NAME genetic
    GITHUB_REPOSITORY DeveloperPaul123/genetic
    GIT_TAG 0.1.0 # change this to latest commit or release tag
)
```

## Usage

Simple example:

🚧

You can see other examples in the `/examples` folder.

## Benchmarks 

🚧

## Building

This project has been built with:

* Visual Studio 2022
* Clang `10.+` (via WSL on Windows)
* GCC `11.+` (vis WSL on Windows)
* CMake `3.21+`

To build, run:

```bash
cmake -S . -B build
cmake --build build
```

### Build Options

| Option | Description | Default |
|:-------|:------------|:--------:|
| `GENETIC_BUILD_TESTS` | Turn on to build unit tests. Required for formatting build targets. | ON |
| `GENETIC_BUILD_EXAMPLES` | Turn on to build examples | ON |

### Run clang-format

Use the following commands from the project's root directory to check and fix C++ and CMake source style.
This requires _clang-format_, _cmake-format_ and _pyyaml_ to be installed on the current system. To use this feature you must turn on `TP_BUILD_TESTS`.

```bash
# view changes
cmake --build build/test --target format

# apply changes
cmake --build build/test --target fix-format
```

See [Format.cmake](https://github.com/TheLartians/Format.cmake) for details.

### Build the documentation

The documentation is automatically built and [published](https://developerpaul123.github.io/genetic) whenever a [GitHub Release](https://help.github.com/en/github/administering-a-repository/managing-releases-in-a-repository) is created.
To manually build documentation, call the following command.

```bash
cmake -S documentation -B build/doc
cmake --build build/doc --target GenerateDocs
# view the docs
open build/doc/doxygen/html/index.html
```

To build the documentation locally, you will need Doxygen and Graphviz on your system.

## Contributing

Contributions are very welcome. Please see [contribution guidelines for more info](CONTRIBUTING.md).

## License

The project is licensed under the MIT license. See [LICENSE](LICENSE) for more details.

## Author

| [<img src="https://avatars0.githubusercontent.com/u/6591180?s=460&v=4" width="100"><br><sub>@DeveloperPaul123</sub>](https://github.com/DeveloperPaul123) |
|:----:|
