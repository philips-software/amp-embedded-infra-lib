FROM gcr.io/oss-fuzz-base/base-builder@sha256:75c10493854955a8c086426e36299581d4aadd81b869965f2e2cc748247d70dd

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
