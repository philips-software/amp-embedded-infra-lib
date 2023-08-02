FROM gcr.io/oss-fuzz-base/base-builder@sha256:36068cdd7166c14943036d321790a99d516c20e117ae9250ca715b169dc2d41b

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
