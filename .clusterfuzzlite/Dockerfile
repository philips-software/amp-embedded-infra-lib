FROM gcr.io/oss-fuzz-base/base-builder@sha256:5db974d9037e7d65fe3e03109f8346b152084d3618f6b03793e6f3ae84ac75d2

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
