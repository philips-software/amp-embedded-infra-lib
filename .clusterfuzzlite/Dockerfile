FROM gcr.io/oss-fuzz-base/base-builder@sha256:982962af87ec1abf8c12a9d210ab24118a8cbc88e90c44900c11b276eed5c54b

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
