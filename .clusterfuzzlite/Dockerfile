FROM gcr.io/oss-fuzz-base/base-builder@sha256:5456a6265baa630d3d708a568e74eb766af5681b5845f61d237a95b20843bec1

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
