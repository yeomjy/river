ARG parent_image
FROM $parent_image

# Set up the directory for the build artifacts.
# Can't use the WORKDIR command as it will break other images
ENV OUT /out
ENV WORKDIR /workspace
RUN mkdir -p $WORKDIR

RUN  apt-get update && \
  apt-get upgrade -y && \
  apt-get install -y git sudo build-essential unzip libssl-dev wget libboost-all-dev && \
  rm -rf /var/lib/apt/lists/*

# Setup cmake
RUN cd $WORKDIR && wget https://github.com/Kitware/CMake/releases/download/v3.19.1/cmake-3.19.1-Linux-x86_64.tar.gz && \
  tar -zxvf cmake-3.19.1-Linux-x86_64.tar.gz && \
  ln -sf $WORKDIR/cmake-3.19.1-Linux-x86_64/bin/* /usr/local/bin/

# Install deps
RUN pip3 install --upgrade pip && \
  pip3 install --upgrade setuptools && \
  pip3 install lief capstone tensorflow z3-solver

# Clone river code
RUN cd $WORKDIR && git clone --recursive https://github.com/unibuc-cs/river.git

# Build Triton from source and install it in the default paths
# Will extract the path to where pip installes packages so it can tell Triton where to find libcapstone
# Triton expects the library name to be libcapstone.so.4, so it will also create a symlink with this name
# This RUN command has to be invoked through `/bin/bash -c ...` so it can expand the PYTHON_PACKAGES var
RUN ["/bin/bash", "-c", "cd $WORKDIR/river/River3/ExternalTools/Triton && mkdir build && cd build && \
    PYTHON_PACKAGES=$(pip3 show capstone | grep Location | cut -f 2 -d ' ') && \
    CAPSTONE_INCLUDE_DIRS=${PYTHON_PACKAGES}/capstone/include/ \
    CAPSTONE_LIBRARIES=${PYTHON_PACKAGES}/capstone/lib/libcapstone.so \
    Z3_INCLUDE_DIRS=${PYTHON_PACKAGES}/z3/include/ \
    Z3_LIBRARIES=${PYTHON_PACKAGES}/z3/lib/libz3.so \
    cmake .. && \
    make -j2 && \
    sudo make install && \
    sudo ln -s ${PYTHON_PACKAGES}/capstone/lib/libcapstone.so ${PYTHON_PACKAGES}/capstone/lib/libcapstone.so.4"]

# Use shim libfuzzer as afl
RUN ["/bin/bash", "-c", "cd $WORKDIR && \
    echo $'#include <stdio.h>\n#include <string.h>\n#include <unistd.h>\n#include <stdlib.h>\n#include <stdint.h>\n\
    extern \"C\" {\n\
    int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size);\n\
    __attribute__((weak)) int LLVMFuzzerInitialize(int *argc, char ***argv);\n\
    }\n\
    // Input buffer.\n\
    static const size_t kMaxInputSize = 1 << 20;\n\
    static uint8_t inputBuf[kMaxInputSize];\n\
    int main(int argc, char** argv){\n\
        if (LLVMFuzzerInitialize)\n\
            LLVMFuzzerInitialize(&argc, &argv);\n\
        while (1) {\n\
        ssize_t n_read = read(0, inputBuf, kMaxInputSize);\n\
        if (n_read > 0) {\n\
            size_t river_in_len = (size_t) inputBuf[0];\n\
            uint8_t *copy = new uint8_t[river_in_len + 1];\n\
            memcpy(copy, inputBuf + 1, river_in_len);\n\
            copy[river_in_len] = 0;\n\
            LLVMFuzzerTestOneInput(copy, river_in_len);\n\
            delete[] copy;\n\
        }\n\
        }\n\
    }\n' > river_shim.cpp"]

RUN cd $WORKDIR && \
    clang++ -stdlib=libc++ -std=c++11 -O2 -c river_shim.cpp -o river_shim.o
