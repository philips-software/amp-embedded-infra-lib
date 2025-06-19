FROM gcr.io/oss-fuzz-base/base-builder@sha256:499d4d7e362d19a8ed9b8ca3a8ae1beef8413f22c89f8c3522d54aa17118b738

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
