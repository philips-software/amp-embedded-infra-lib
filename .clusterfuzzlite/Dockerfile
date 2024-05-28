FROM gcr.io/oss-fuzz-base/base-builder@sha256:97bb083e4c83ed53b89e35a3e47359c453375bd4ef1be3f0394f9d7fd2bf2466

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
