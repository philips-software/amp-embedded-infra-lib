FROM gcr.io/oss-fuzz-base/base-builder@sha256:0d3e5b7e259deb6c4a64f3b850733394b1552bdd1940376702b4c48de53d8a14

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
