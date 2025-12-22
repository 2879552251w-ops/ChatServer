#!/bin/bash

set -e   # 任一命令失败就退出

ROOT_DIR=$(cd "$(dirname "$0")" && pwd)
BUILD_DIR="$ROOT_DIR/build"

echo "==> Clean build directory"
rm -rf "$BUILD_DIR"

echo "==> Create build directory"
mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR"

echo "==> Run cmake"
cmake ..

echo "==> Build"
make -j$(nproc)

echo "==> Build finished successfully"
