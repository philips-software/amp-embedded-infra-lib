FROM gcr.io/oss-fuzz-base/base-builder@sha256:6bc204c52f3c00f691125cadc3f13cce24bd663fb0fed41fec4119444f8bfe5b

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
