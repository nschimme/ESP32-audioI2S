# Reproducible build of the library against the Arduino ESP32 core, no board
# or local toolchain needed.
#
#   docker build -f docker/Dockerfile -t esp32-audioi2s-build .
#   docker run --rm -v "$PWD":/repo esp32-audioi2s-build            # esp32s3 + esp32p4
#   docker run --rm -v "$PWD":/repo esp32-audioi2s-build esp32s3    # one target
FROM python:3.12-slim

RUN apt-get update \
 && apt-get install -y --no-install-recommends git ca-certificates \
 && rm -rf /var/lib/apt/lists/* \
 && pip install --no-cache-dir platformio==6.2.0

ENV PLATFORMIO_CORE_DIR=/opt/platformio \
    PLATFORMIO_BUILD_CACHE_DIR=/opt/pio-cache

# Warm the toolchain, Arduino core and compiler cache at image build time so
# a run only compiles what changed. The sources copied here are replaced by
# the bind mount at run time.
COPY . /repo
WORKDIR /repo/docker
RUN pio run -e esp32s3 -e esp32p4 \
 && rm -rf /repo/docker/.pio

COPY docker/build.sh /usr/local/bin/build.sh
ENTRYPOINT ["/usr/local/bin/build.sh"]
