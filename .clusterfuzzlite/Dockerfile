FROM gcr.io/oss-fuzz-base/base-builder@sha256:5ab991457e1a8165e7e6250a0dcef0705009d3028fac6cb4cf6bccafd8edcd48

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
