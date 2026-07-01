FROM gcr.io/oss-fuzz-base/base-builder@sha256:33f90392168663c1b55ca3daf82089a619db54c1b9a43121198e50271c4ed266

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
