FROM gcr.io/oss-fuzz-base/base-builder@sha256:5be6a533efa0121f7a5382fe9caccb9a2fcda06a0b8a91312e598966712417bf

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
