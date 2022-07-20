FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive

SHELL ["/bin/bash", "-o", "pipefail", "-c"]

# Install the base system with all tool dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates=20211016~20.04.1 \
        ccache=3.7.7-1 \
        clang-12=1:12.0.0-3ubuntu1~20.04.5 \
        clang-format-12=1:12.0.0-3ubuntu1~20.04.5 \
        clang-tidy-12=1:12.0.0-3ubuntu1~20.04.5 \
        g++-8=8.4.0-3ubuntu2 \
        gcc-8=8.4.0-3ubuntu2 \
        gdb=8.2.1-2+b3 \
        git=1:2.25.1-1ubuntu3.4 \
        gnupg2=2.2.19-3ubuntu2.1 \
        iwyu=8.0-3build1 \
        libllvm12=1:12.0.0-3ubuntu1~20.04.5 \
        libxml2=2.9.10+dfsg-5ubuntu0.20.04.3 \
        lsb-release=11.1.0ubuntu2 \
        ninja-build=1.10.0-1build1 \
        python3=3.8.2-0ubuntu2 \
        python3-pip=20.0.2-5ubuntu1.6 \
        wget=1.20.3-1ubuntu2 \
 && rm -rf /var/lib/apt/lists/*

# Include the Cisco Umbrella PKI Root
RUN wget -qO /usr/local/share/ca-certificates/Cisco_Umbrella_Root_CA.crt https://www.cisco.com/security/pki/certs/ciscoumbrellaroot.pem \
 && update-ca-certificates

# Install cmake via pip to get a recent version; install gcovr
RUN python3 -m pip --no-cache-dir install cmake==3.22.4 \
 && python3 -m pip --no-cache install gcovr==5.0

# Install mull-cxx mutation testing compiler driver
RUN wget -qO /tmp/mull.deb https://github.com/mull-project/mull/releases/download/0.18.0/Mull-12-0.18.0-LLVM-12.0-ubuntu-20.04.deb \
 && dpkg -i /tmp/mull.deb \
 && rm /tmp/mull.deb

# Install docker-cli for Docker-from-Docker tools
RUN wget -qO - "https://download.docker.com/linux/$(lsb_release -is | tr '[:upper:]' '[:lower:]')/gpg" | apt-key add - 2>/dev/null \
 && echo "deb [arch=amd64] https://download.docker.com/linux/$(lsb_release -is | tr '[:upper:]' '[:lower:]') $(lsb_release -cs) stable" | tee /etc/apt/sources.list.d/docker.list \
 && apt-get update && apt-get install -y --no-install-recommends docker-ce-cli=5:20.10.17~3-0~ubuntu-focal \
 && rm -rf /var/lib/apt/lists/*

# Update all tools alternatives to the correct version
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 10 \
 && update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-8 20 \
 && update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-8 20 \
 && update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-12 10 \
 && update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-12 10
