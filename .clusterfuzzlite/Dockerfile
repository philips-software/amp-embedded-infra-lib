FROM gcr.io/oss-fuzz-base/base-builder@sha256:4a79fda7f254fb9610443f0a4ccb31b89934f3efd7f1783ed339f8c6ad1250da

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
