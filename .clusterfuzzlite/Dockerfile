FROM gcr.io/oss-fuzz-base/base-builder@sha256:4dfa9f84d5c677988a612f76eff68340d4a7dd61d2067468e1db1df72a447e2e

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
