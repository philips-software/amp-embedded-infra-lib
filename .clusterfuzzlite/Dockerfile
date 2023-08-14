FROM gcr.io/oss-fuzz-base/base-builder@sha256:c482768df237b3f2ecfef75fbf097f7d70f24a7b9bfaf903fb6cc9b4e545fa0d

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
