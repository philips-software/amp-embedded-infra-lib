FROM gcr.io/oss-fuzz-base/base-builder@sha256:27e8f4f46adcf23c78a3122e016cdbf4a821bffb28a2cd83a4f2fc7c8b4e1849

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
