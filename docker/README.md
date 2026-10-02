# Docker build

Compiles the library against the Arduino ESP32 core 3.x in a container, so a
change can be checked without a board or a local toolchain. PlatformIO prints
the static RAM and flash use of each target, which makes before/after size
comparisons easy.

```
docker build -f docker/Dockerfile -t esp32-audioi2s-build .
docker run --rm -v "$PWD":/repo esp32-audioi2s-build            # esp32s3 and esp32p4
docker run --rm -v "$PWD":/repo esp32-audioi2s-build esp32      # classic ESP32
```

The firmware is left in `docker/.pio/build/<env>/`. The sketch in
`docker/sketch/` only links the library in; it is not meant to be flashed
as is.
