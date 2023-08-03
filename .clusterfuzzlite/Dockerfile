FROM gcr.io/oss-fuzz-base/base-builder@sha256:0ee35bd4b1b317246150a15ac899898a0c668f772d7aae7023d6cc33daf9024d

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
