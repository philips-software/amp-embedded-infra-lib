FROM gcr.io/oss-fuzz-base/base-builder@sha256:379a89ac38a5257979215115502a289c77eba83e0277db4b00cc224ffe5a0be6

HEALTHCHECK NONE

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
