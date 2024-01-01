FROM gcr.io/oss-fuzz-base/base-builder@sha256:ce7fc094ee67dd50cf7173127b0ddf1e97b4d89c34bac34e2c74f942a331b2a1

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
