FROM gcr.io/oss-fuzz-base/base-builder@sha256:33896919dc2768458b0ed83ab10f857d980191ddeecf1b6b82a086ea3e573be0

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
