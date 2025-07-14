#!/bin/bash
set -e

# === Config ===
WASI_VERSION="25.0"
WASI_MAJOR_VERSION="${WASI_VERSION%%.*}"  # Extract '25' from '25.0'

# === Paths ===
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="${SCRIPT_DIR}/.."
EXTERNAL_DIR="${PROJECT_ROOT}/external"
SDK_DIR="${EXTERNAL_DIR}/wasi-sdk"
ARCHIVE_NAME="wasi-sdk-${WASI_VERSION}.tar.gz"

# === Detect platform ===
UNAME=$(uname -s)
ARCH=$(uname -m)

if [[ "$UNAME" == "Darwin" ]]; then
  PLATFORM="macos"
elif [[ "$UNAME" == "Linux" ]]; then
  case "$ARCH" in
    x86_64)  PLATFORM="x86_64-linux" ;;
    aarch64|arm64) PLATFORM="arm64-linux" ;;
    *)
      echo "❌ Unsupported Linux architecture: $ARCH"
      exit 1
      ;;
  esac
elif [[ "$UNAME" =~ MINGW|MSYS|CYGWIN ]]; then
  PLATFORM="mingw"
else
  echo "❌ Unsupported OS: $UNAME"
  exit 1
fi

WASI_FILENAME="wasi-sdk-${WASI_VERSION}-${PLATFORM}.tar.gz"
WASI_URL="https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-${WASI_MAJOR_VERSION}/${WASI_FILENAME}"

echo "📦 Preparing to download WASI SDK v${WASI_VERSION} for platform: ${PLATFORM}"
echo "🌐 URL: $WASI_URL"

mkdir -p "$EXTERNAL_DIR"

# === Download ===
echo "⬇️  Downloading..."
if ! curl -L --fail "$WASI_URL" -o "${PROJECT_ROOT}/${ARCHIVE_NAME}"; then
  echo "❌ Failed to download WASI SDK from:"
  echo "   $WASI_URL"
  echo "🔎 Please check that this version and platform are supported."
  exit 1
fi

# === Extract ===
echo "📂 Extracting to $SDK_DIR..."
rm -rf "$SDK_DIR"
mkdir -p "$SDK_DIR"
tar -xzf "${PROJECT_ROOT}/${ARCHIVE_NAME}" --strip-components=1 -C "$SDK_DIR"
rm "${PROJECT_ROOT}/${ARCHIVE_NAME}"

echo "✅ WASI SDK is ready at: $SDK_DIR"
