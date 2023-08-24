FROM gcr.io/oss-fuzz-base/base-builder@sha256:98bb98ceb733d1d50a392466c6eed2127c94871ce908f43b9d68fb57d22798d6

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
