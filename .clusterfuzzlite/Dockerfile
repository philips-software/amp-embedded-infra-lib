FROM gcr.io/oss-fuzz-base/base-builder@sha256:43cd6e556243a3a7ff5df49635cecd290905e1027d2e91b9a28b3b4c0e8e2431

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
