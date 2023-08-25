FROM gcr.io/oss-fuzz-base/base-builder@sha256:01ebb1d40fab67f0b4425b5e1beda6013443741e82cac815a9efb579829c85ef

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
