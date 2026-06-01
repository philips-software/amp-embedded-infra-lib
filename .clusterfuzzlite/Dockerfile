FROM gcr.io/oss-fuzz-base/base-builder@sha256:34ac75bca6276404ff4f1805b1a99750e48b7f69809d205b340d9c5c4a9d284e

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
