FROM gcr.io/oss-fuzz-base/base-builder@sha256:0b185ee6320e1668595f25991cca2bc58e0c78f0658eee252ec280ccb056ba8d

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
