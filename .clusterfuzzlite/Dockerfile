FROM gcr.io/oss-fuzz-base/base-builder@sha256:81c0c136cda1ba3f85fd41f65851f7feb942a5fc0aeca4c4f02fb53fa5e835d8

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
