FROM gcr.io/oss-fuzz-base/base-builder@sha256:094001a315e67e5f96c26d7beabc4eeee791a639dade216c49e1dd55e3ac3d50

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
