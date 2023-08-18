FROM gcr.io/oss-fuzz-base/base-builder@sha256:21122bee580072f9a0513c3d5ad6cffb5b4a9b43edceee4428ee91efee4379d5

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
