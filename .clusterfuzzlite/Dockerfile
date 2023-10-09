FROM gcr.io/oss-fuzz-base/base-builder@sha256:6c61e467be42d10170ab0c4715d25805829e7c97aa60ce7b408687a02cd40c2d

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
