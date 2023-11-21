#!/bin/bash -eu

cmake --preset Fuzzing
cmake --build --preset Fuzzing

cp build/Fuzzing/infra/syntax/fuzz/infra.syntax_json_fuzzer $OUT/infra-syntax_json_fuzzer
