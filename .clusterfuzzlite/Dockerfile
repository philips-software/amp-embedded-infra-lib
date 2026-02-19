FROM gcr.io/oss-fuzz-base/base-builder@sha256:15357c44a5a12e9af78204ff4c14a998ee10ad3145ed8b97baf80c0ef25fa17d

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
