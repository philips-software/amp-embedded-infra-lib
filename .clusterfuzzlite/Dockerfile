FROM gcr.io/oss-fuzz-base/base-builder@sha256:aba4629f06c8d16b3b3e9e525f4bcc3b9e9b852c5529f2b459ee75294575082d

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
