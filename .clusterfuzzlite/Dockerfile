FROM gcr.io/oss-fuzz-base/base-builder@sha256:c9a5e81eefd4182e4039f4181766ceee45748ec771961888b8993e9673c29f51

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
