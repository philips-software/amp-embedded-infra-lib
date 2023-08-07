FROM gcr.io/oss-fuzz-base/base-builder@sha256:894e222384e92429a8f5335ed6ac7ef0fb82d5820c3a4c674b41bccf72f5f7bc

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
