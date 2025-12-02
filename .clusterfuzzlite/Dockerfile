FROM gcr.io/oss-fuzz-base/base-builder@sha256:ec285f652b71a7212c06852c565ea55360a7b8ebbd3825dafa0a0ca34e877f53

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
