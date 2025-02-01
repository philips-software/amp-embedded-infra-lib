FROM gcr.io/oss-fuzz-base/base-builder@sha256:1314b399d2db5f4274bea94e340325aae7ba9b1e8b30fbb77dabcf3cd785af28

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
