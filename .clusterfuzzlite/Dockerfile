FROM gcr.io/oss-fuzz-base/base-builder@sha256:9111ea87e56c6e9209136ca5cfc17b6ae9b67ab87a0330066c8000a78b0944d1

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
