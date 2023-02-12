FROM gcr.io/oss-fuzz-base/base-builder@sha256:769b77a7684f8936457486ff330929e3b5d61826d12c1f1630470445abb0a256

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
