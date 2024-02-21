FROM gcr.io/oss-fuzz-base/base-builder@sha256:5e4a33eb0d8264da878ce067e18e6b052848519a00ef0f3ef2a2c02bc73b4bfc

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
