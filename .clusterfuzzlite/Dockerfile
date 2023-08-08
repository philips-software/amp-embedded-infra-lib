FROM gcr.io/oss-fuzz-base/base-builder@sha256:bf37c217a2c62b5a8c6713bdda54e8c497d3728a784f04b7063a90c0f8d04479

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
