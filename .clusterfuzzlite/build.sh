#!/bin/bash -eu

cmake --preset Fuzzing
cmake --build --preset Fuzzing

cp Build/Fuzzing/infra/syntax/fuzz/infra.syntax_json_fuzzer $OUT/infra-syntax_json_fuzzer
