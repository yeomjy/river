FROM ubuntu:20.04
ARG VERSION=unknown

# LABEL maintainer="eduard.c.staniloiu@gmail.com" \
#       name="UB UPB River Implementation" \
#       version="${VERSION}"

WORKDIR /src

ENV DEBIAN_FRONTEND="noninteractive"

# Install missing packages and required dependencies
RUN apt-get update
RUN apt-get install -y wget vim ssh git python3 python3-pip \
        libboost-all-dev libz3-dev cmake

# Use pip to install latest capstone and lief libraries
RUN pip3 install setuptools --upgrade
RUN pip3 install tensorflow
RUN pip3 install lief==0.11.5
RUN pip3 install capstone==4.0.2
RUN pip3 install z3-solver

# Clone River and it's submodules
RUN git clone --recursive https://github.com/unibuc-cs/river

# Build Triton from source and install it in the default paths
# Will extract the path to where pip installes packages so it can tell Triton where to find libcapstone
# Triton expects the library name to be libcapstone.so.4, so it will also create a symlink with this name
# This RUN command has to be invoked through `/bin/bash -c ...` so it can expand the PYTHON_PACKAGES var
RUN ["/bin/bash", "-c", "cd /src/river/River3/ExternalTools/Triton && mkdir build && cd build && \
    PYTHON_PACKAGES=$(pip3 show capstone | grep Location | cut -f 2 -d ' ') && \
    CAPSTONE_INCLUDE_DIRS=${PYTHON_PACKAGES}/capstone/include/ \
    CAPSTONE_LIBRARIES=${PYTHON_PACKAGES}/capstone/lib/libcapstone.so \
    cmake .. && \
    make -j12 && \
    make install && \
    ln -s ${PYTHON_PACKAGES}/capstone/lib/libcapstone.so ${PYTHON_PACKAGES}/capstone/lib/libcapstone.so.4"]

RUN PYV=`python3 -c "import platform;print(platform.python_version()[:3])"` && \
    PYP="/usr/lib/python$PYV/site-packages" && \
    echo export PYTHONPATH="$PYP:\$PYTHONPATH" >> /etc/bash.bashrc && \
    python3 -c "import z3; print('Z3 version:', z3.get_version_string())" && \
    PYTHONPATH="$PYP" python3 -c "from triton import *; ctx=TritonContext(ARCH.X86_64);print(ctx)"
 
    
ENV PYTHONPATH="/usr/lib/python3.8/site-packages"

# Run example test when container starts
CMD cd /src/river/River3/TestPrograms && \
    gcc -g -O0 -o crackme_sample crackme_sample.c && \
    cd ../python && \
    python3 concolic_GenerationalSearch2.py --binaryPath "../TestPrograms/crackme_sample" \
                                            --architecture x64 \
                                            --maxLen 5 \
                                            --logLevel CRITICAL \
                                            --secondsBetweenStats 10 \
                                            --outputType textual && \
    python3 concolic_RLGenerationalSearch.py --binaryPath "../TestPrograms/crackme_sample" \
                                             --architecture x64 \
                                             --maxLen 5 \
                                             --logLevel CRITICAL \
                                             --secondsBetweenStats 10 \
                                             --outputType textual

