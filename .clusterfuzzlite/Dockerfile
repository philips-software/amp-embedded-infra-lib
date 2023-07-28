FROM gcr.io/oss-fuzz-base/base-builder@sha256:5f9a6522e8b8055389b6b3a9ef85e557c847a6d3ab3c05b3f31757dff461edb8

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
