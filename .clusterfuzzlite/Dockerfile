FROM gcr.io/oss-fuzz-base/base-builder@sha256:7edcb5fff04473db9a4e48548a047c5695a94dd417d1925a712b0cccfc266a0a

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
