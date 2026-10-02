#!/bin/sh
# Builds the sketch in docker/sketch against the library in /repo for each
# environment named on the command line (default: esp32s3 and esp32p4) and
# leaves the firmware under /repo/docker/.pio/build/<env>/.
set -e
cd /repo/docker
[ "$#" -gt 0 ] || set -- esp32s3 esp32p4
for env in "$@"; do
    pio run -e "$env"
done
