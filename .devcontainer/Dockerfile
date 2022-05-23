FROM gcc:8

RUN apt-get update && apt-get install -y --no-install-recommends \
        ccache=3.6-1 \
        ninja-build=1.8.2-1 \
        python3 \
        python3-pip \
        xsltproc=1.1.32-2.2~deb10u1 \
    && rm -rf /var/lib/apt/lists/* \
    && python3 -m pip --no-cache-dir install cmake==3.22.2
