FROM gcr.io/oss-fuzz-base/base-builder@sha256:9d3d9054487e2d641e910184b6fde9d1559e675ee622997ad77f7fa04c8a203f

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
