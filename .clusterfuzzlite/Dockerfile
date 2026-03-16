FROM gcr.io/oss-fuzz-base/base-builder@sha256:4c20153f428f2ddf623a725251c4b85f71eb19cf3718a33fa41f299a2c3a5226

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
