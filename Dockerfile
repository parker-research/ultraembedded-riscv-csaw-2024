# This Dockerfile sets up an environment to build and test the ultraembedded RISC-V project.
# To run it, mount the project directory into the container, e.g.,
#
# Bash:
# docker run -it --rm -v $(pwd):/project riscv_build
#
# Fish:
# docker build -t riscv_build .
# docker run -it --rm -v (pwd):/project riscv_build

# FROM ubuntu:22.04
# FROM verilator/verilator:latest
FROM verilator/verilator:4.038
# Versions: https://hub.docker.com/r/verilator/verilator
# Use version 4.038, per https://github.com/ultraembedded/riscv/issues/17

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    make \
    libelf-dev \
    binutils-dev \
    # systemc-dev \
    # verilator \
    git \
    fish \
    sudo \
    wget

# Change the default shell to fish
RUN echo "fish" >> ~/.bashrc
RUN echo "echo 'Fish exited.' && exit" >> ~/.bashrc

# Install SystemC.
# Guide: https://gist.github.com/bagheriali2001/0736fabf7da95fb02bbe6777d53fabf7
# SystemC version 2.3.x is good, per note in the following Makefiles (mentioning version 2.3.3):
    # ./top_tcm_axi/tb/makefile.build_sysc_tb
    # ./top_cache_axi/tb/makefile.build_verilated
WORKDIR /systemc
RUN wget https://accellera.org/images/downloads/standards/systemc/systemc-2.3.3.tar.gz
RUN tar -xvzf systemc-2.3.3.tar.gz
# cd systemc-2.3.3 ; mkdir objdir ; cd objdir
WORKDIR /systemc/systemc-2.3.3/objdir
# export CXX=g++
ENV CXX=g++
RUN ../configure
RUN make -j $(nproc)
RUN make install
WORKDIR /systemc/systemc-2.3.3
RUN rm -rf objdir

# Set important environment variables.
ENV SYSTEMC_INCLUDE=/usr/local/systemc/include
ENV SYSTEMC_LIBDIR=/usr/local/systemc/lib-linux64
ENV SYSTEMC_HOME=/systemc/systemc-2.3.3

ENV C_INCLUDE_PATH=/usr/local/share/verilator/include/:/usr/local/share/verilator/include/vltstd:/usr/local/systemc/include
ENV CPLUS_INCLUDE_PATH=/usr/local/share/verilator/include/:/usr/local/share/verilator/include/vltstd:/usr/local/systemc/include

# The following line must contain `verilated.cpp` file.
ENV VERILATOR_SRC=/usr/local/share/verilator/share

# Create a working directory
WORKDIR /project

# Instructions for building and running the testbench
# Run the testbench using:
# ```
# cd top_tcm_axi/tb
# make
# make run
# ```

# If any environment variables or dependencies are specific to your system, refer to the documentation
# at https://github.com/ultraembedded/riscv

ENTRYPOINT ["/usr/bin/fish"]
