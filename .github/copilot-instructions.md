# Copilot Instructions for amp-embedded-infra-lib

This repository is a set of C++ libraries and headers that provide heap-less, STL-like, infrastructure for embedded software development.

## Build System Conventions

**CMake Presets**: Use CMake presets extensively. Key presets:
- `host` - Host tooling/tests (use for development)
- `coverage` - Test coverage analysis

**EmIL Patterns**: All CMake uses EmIL conventions:
```cmake
emil_build_for(target PREREQUISITE_BOOL EMIL_BUILD_TESTS)
emil_generate_artifacts(TARGET target HEX MAP)
emil_exclude_from_clang_format(directory)
```

**Target Conditionals**: Build logic heavily uses target conditionals:
```cmake
if ("${TARGET_MCU}" STREQUAL stm32wb55)
if (TARGET_MCU_VENDOR STREQUAL ti)
```

## Development Workflow

**Container Development**: Strongly recommended to use devcontainer - contains all cross-compilation toolchains and tools. Commands assume container environment.

**Code Style and Formatting**: MUST follow the `./documents/modules/ROOT/pages/CodingStandard.adoc`. Use `clang-format` for formatting; configured via `.clang-format`.

**Build Commands**:
```bash
# List available presets
cmake --list-presets

# Configure and build for target
cmake --preset=<configure_preset>
cmake --build --preset=<build_preset>

# Run all tests (host preset only)
ctest --preset host

# Run a single test by name
ctest --preset host -R <test_name>
```

**Pull Requests and Commits**:
- Follow Conventional Commit style from `.github/CONTRIBUTING.md` when proposing pull request titles and when creating commit messages.

**VS Code Integration**:
- CMake extension manages presets via status bar
- TestMate C++ for micro tests
- Gcov Viewer for coverage analysis

## Service Architecture

**Echo/Sesame**: RPC mechanisms from EmIL. Echo for TCP/IP and/or Echo on top of Sesame for serial communication. Refer to `./documents/modules/ROOT/pages/Sesame.adoc` for details on Sesame and `./documents/modules/ROOT/pages/Echo.adoc` for details on Echo.

**EventDispatcher**: Centralized event handling system. Always read `./documents/modules/ROOT/pages/EventDispatcher.adoc` if usage of EventDispatcher is involved.

**ClaimableResource**: Resource management pattern for shared software resources. Refer to `./documents/modules/ROOT/pages/ClaimableResource.adoc` for details.

## Testing Strategy

**Micro Tests**: Unit tests alongside source code in `test/` subdirectories. Use `emil_add_test()`, and guard the executable with `emil_build_for(<target> BOOL EMIL_BUILD_TESTS)`. For more information on micro tests, see `./.github/instructions/microtest.instructions.md`.

**Test Doubles**: Mock implementations in `test_doubles/` directories.

## File Naming Conventions

- Source files: `CamelCase.cpp/.hpp`
- CMake targets: `package.component` (e.g., `infra.timer`, `services.tracer`)
- Executables: Often match directory structure (`examples.serial_net`)

## Common Issues

- Cross-compilation toolchain paths are container-specific
- EmIL build flags (`EMIL_*`) control feature inclusion

## General Coding Rules

- Avoid adding code comments unless they are necessary to clarify non-obvious intent.
- Prefer `infra` containers (e.g., `infra::Bounded*`, `infra::Intrusive*`) over standard library containers, except in host applications and tests.
- When declaring a class with base classes, place the colon and inheritance list on a separate line with proper indentation.
- Constructors with one parameter must be an explicit constructor.
- No exceptions shall be used in the codebase, except in Host applications and tests.
- Avoid protected members in classes except test classes; prefer private members with public/protected accessors if needed.
- Use SFINAE to restrict template parameters if the template is only valid for specific types.
- Use `assert` for non-critical preconditions and `really_assert` for critical preconditions that must never be violated.
- Use `LOG_AND_ABORT(...)` when the code reached an unexpected location/state.
