FROM gcr.io/oss-fuzz-base/base-builder@sha256:de7bd002f4a5545cd939c6f1b4c139676f9506f7724195b3c9b506f5924890ac

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
