FROM gcr.io/oss-fuzz-base/base-builder@sha256:4c4826f4815fd4d6b609aaf5ae5e5055830bf45b9c34297e75e9478da11a8f0e

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
