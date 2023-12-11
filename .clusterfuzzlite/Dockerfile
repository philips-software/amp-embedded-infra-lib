FROM gcr.io/oss-fuzz-base/base-builder@sha256:7c4fa0fe88e271549c978c6b2b3260588c73f8d63cc3b014c1300aa8187697df

RUN apt-get update && apt-get upgrade -y --no-install-recommends \
        ninja-build \
 && rm -rf /var/lib/apt/lists/*

COPY . $SRC/amp-embedded-infra-lib
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/amp-embedded-infra-lib
