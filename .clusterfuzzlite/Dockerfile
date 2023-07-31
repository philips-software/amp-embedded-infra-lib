FROM gcr.io/oss-fuzz-base/base-builder@sha256:7b2af946dbd6b17eb277f4c2bd2c8ea145ab569623bd1591c6931f7d9fcaeb9b

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
