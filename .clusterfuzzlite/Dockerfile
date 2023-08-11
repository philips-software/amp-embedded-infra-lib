FROM gcr.io/oss-fuzz-base/base-builder@sha256:8b614f0e88502b5d0259f008e07726b256c34297eda320f1897d70d45a0ea43d

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
