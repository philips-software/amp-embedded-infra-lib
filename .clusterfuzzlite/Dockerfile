FROM gcr.io/oss-fuzz-base/base-builder@sha256:56c035f4f18cdf5cc87a70e25b5ab19cfa1775bb4028dfea8e4df0030dc0ea76

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
