# Builder stage
FROM python:3.9-slim-bookworm as builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-arm-linux-gnueabihf \
    python3-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy the ARMv7 version of the library
COPY libfwlib32-linux-armv7.so.1.0.5 /usr/local/lib/
RUN ln -s /usr/local/lib/libfwlib32-linux-armv7.so.1.0.5 /usr/local/lib/libfwlib32.so && \
    ln -s /usr/local/lib/libfwlib32-linux-armv7.so.1.0.5 /usr/local/lib/libfwlib32.so.1 && \
    ldconfig

# Install Python build dependencies
RUN python -m pip install setuptools wheel click

WORKDIR /usr/src/fwlib
COPY ./raspberry/fwlib.c ./raspberry/setup.py ./fwlib32.h ./

# Modify setup.py to ensure ARM compilation
ENV ARCHFLAGS="-arch arm"
ENV CFLAGS="-march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard"
RUN python setup.py bdist_wheel

# Final stage
FROM python:3.9-slim-bookworm

# Copy the ARMv7 version of the library
COPY libfwlib32-linux-armv7.so.1.0.5 /usr/local/lib/
# Set up library links
RUN ln -s /usr/local/lib/libfwlib32-linux-armv7.so.1.0.5 /usr/local/lib/libfwlib32.so && \
    ln -s /usr/local/lib/libfwlib32-linux-armv7.so.1.0.5 /usr/local/lib/libfwlib32.so.1 && \
    ldconfig

WORKDIR /app

COPY --from=builder /usr/src/fwlib/dist/*.whl /app/

RUN pip install /app/*.whl && rm /app/*.whl

COPY ./raspberry/main.py ./
