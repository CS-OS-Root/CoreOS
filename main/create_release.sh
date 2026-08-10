#!/bin/bash
set -e

VERSION="${1:-1.0.0}"
RELEASE_DIR="${2:-release/${VERSION}}"

echo "========================================================================"
echo "Creating release version: ${VERSION}"
echo "Output directory: ${RELEASE_DIR}"
echo "========================================================================"

mkdir -p "${RELEASE_DIR}"

BUILD_DIR="build/release_${VERSION}"
cmake -G Ninja -B "${BUILD_DIR}" -DMLA_APP_VERSION="${VERSION}"
cmake --build "${BUILD_DIR}" -j$(nproc)

if [ -f "${BUILD_DIR}/MLA_C_App_Linux" ]; then
    cp "${BUILD_DIR}/MLA_C_App_Linux" "${RELEASE_DIR}/app"
fi

if [ -f "${BUILD_DIR}/MLA_C_Test_Linux_Single_Thread" ]; then
    cp "${BUILD_DIR}/MLA_C_Test_Linux_Single_Thread" "${RELEASE_DIR}/test_runner"
fi

echo "${VERSION}" > "${RELEASE_DIR}/version"

echo "Release successfully created at: ${RELEASE_DIR}"
