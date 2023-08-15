FROM gcr.io/oss-fuzz-base/base-builder@sha256:48c2c669fbccbbec47625288d7028b9880164e463e088c6ee63c33a56c9d4c8a

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
