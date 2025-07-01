FROM gcr.io/oss-fuzz-base/base-builder@sha256:75fb7d16e4bd7840d26bb4ebb0bcbafa3aa55219989fe7e1d9c530adaa8af598

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
