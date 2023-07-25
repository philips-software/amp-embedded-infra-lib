FROM gcr.io/oss-fuzz-base/base-builder@sha256:0f19e868205a6e5b4cb4203fd9950c2f23522fdef5582dec954fd74b9d4c07ce

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
