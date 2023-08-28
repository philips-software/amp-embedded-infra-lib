FROM gcr.io/oss-fuzz-base/base-builder@sha256:5c64b4f481f91c5f99c02a3d5866528344fbec869080b27695c471b46ad11ec7

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
