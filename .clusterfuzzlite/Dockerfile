FROM gcr.io/oss-fuzz-base/base-builder@sha256:3227f5b85d99dfe4e21e1716c5074b2ab54d728e5feb7f10d6457e5c7e3fd553

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
