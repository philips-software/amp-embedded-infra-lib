FROM gcr.io/oss-fuzz-base/base-builder@sha256:eb77f5af37831bde58f992e15783ff23f04ee81af67962b7e0aaa7e1e26a7662

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
