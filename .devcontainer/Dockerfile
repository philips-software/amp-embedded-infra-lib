FROM gcc:8

RUN apt-get update && apt-get install -y --no-install-recommends \
        ccache=3.6-1 \
        ninja-build=1.8.2-1 \
        python3=3.7.3-1 \
        python3-pip=18.1-5 \
        xsltproc=1.1.32-2.2~deb10u1 \
    && rm -rf /var/lib/apt/lists/* \
    && python3 -m pip --no-cache-dir install cmake==3.22.2
