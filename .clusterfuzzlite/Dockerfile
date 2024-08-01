FROM gcr.io/oss-fuzz-base/base-builder@sha256:2e0c7f2a1d2ebbd73c3c84a804ef8919c2c036f7f0b562221b6be4d670a4236c

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
