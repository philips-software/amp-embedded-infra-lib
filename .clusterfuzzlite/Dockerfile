FROM gcr.io/oss-fuzz-base/base-builder@sha256:cb2666967e34f29d509cc40d6c131f7fc1725739d146d4ae5d09610830c604ae

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
