FROM gcr.io/oss-fuzz-base/base-builder@sha256:ce182660ed7680cbf6d8bd32e562d2924dee63f4fba92be3e4d0aa3fac8f44a7

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
