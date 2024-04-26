FROM gcr.io/oss-fuzz-base/base-builder@sha256:1d64e1074bda7413c6a340284207c45fb8aa39653299b4b4e5e63c2131cac60a

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
