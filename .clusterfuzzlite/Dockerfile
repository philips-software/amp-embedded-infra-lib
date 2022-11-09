FROM gcr.io/oss-fuzz-base/base-builder:v1
RUN apt-get update && apt-get upgrade -y --no-install-recommends ninja-build
COPY . $SRC/amp-embedded-infra-lib
WORKDIR amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/
