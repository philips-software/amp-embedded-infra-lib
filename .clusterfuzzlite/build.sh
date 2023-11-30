#!/bin/bash -eu

cmake --preset fuzzing
cmake --build --preset fuzzing

cp build/fuzzing/infra/syntax/fuzz/infra.syntax_json_fuzzer $OUT/infra-syntax_json_fuzzer
