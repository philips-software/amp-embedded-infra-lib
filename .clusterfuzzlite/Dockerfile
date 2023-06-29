FROM gcr.io/oss-fuzz-base/base-builder@sha256:71094600297841a34a97ae4acbab5a654601ff4e3be7c324642e7f1cbd5d62a9

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
