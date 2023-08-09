FROM gcr.io/oss-fuzz-base/base-builder@sha256:92a7fc6080f7d7ba51e1631ac52bf7172e303951bd799db415adb9769fc916b0

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
