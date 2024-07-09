FROM gcr.io/oss-fuzz-base/base-builder@sha256:d84b8dc0394b7c14247fb6d9f0bf437f428526363d2a20ee4414191492a84def

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
