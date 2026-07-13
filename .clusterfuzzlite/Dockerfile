FROM gcr.io/oss-fuzz-base/base-builder@sha256:89605ec1d4a3ad29b145472cc3b61000a4d182022aac96e3ad113a8e6bf98853

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
