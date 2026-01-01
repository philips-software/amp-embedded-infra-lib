FROM gcr.io/oss-fuzz-base/base-builder@sha256:0c430433cb437e80f8091fc163aac707d35e987ce3b3dc3b64eaf4823a0e74b2

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
