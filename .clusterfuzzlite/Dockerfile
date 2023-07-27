FROM gcr.io/oss-fuzz-base/base-builder@sha256:e808bf977a4c4bfab17c3c41c6c7e8d131a488c5e844296bd515ae5f90930131

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
