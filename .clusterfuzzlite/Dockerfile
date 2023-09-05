FROM gcr.io/oss-fuzz-base/base-builder@sha256:5e1404dc68c77c2a8fdefd8c50f21d069aee193fe3648a9e4fe1ef4d45e3f732

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
