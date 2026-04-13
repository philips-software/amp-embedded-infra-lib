FROM gcr.io/oss-fuzz-base/base-builder@sha256:cb8b7ff401eeb38a378428e6d9ec5a7dc18441ba999e1db21f1641d4debf8f3b

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
