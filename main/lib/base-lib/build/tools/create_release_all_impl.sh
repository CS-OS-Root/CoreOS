#!/usr/bin/env bash

# Resolve workspace directory
if [ -z "$WORKSPACE_DIR" ]; then
    WORKSPACE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
fi

PRODUCT_NAME="${PRODUCT_NAME:-$(basename "$WORKSPACE_DIR")}"
APP_TARGET="${APP_TARGET:-${PRODUCT_NAME}-app}"
RELEASES_SERVER_URL="https://releases.home.schlegel.ovh"

target_config=""
do_publish=false
specified_version=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --publish)
            do_publish=true
            shift
            ;;
        --version|-v)
            specified_version="$2"
            shift 2
            ;;
        --config|-c)
            target_config="$2"
            shift 2
            ;;
        --help|-h)
            echo "Usage: $0 [options] [target_config]"
            echo ""
            echo "Options:"
            echo "  --config <name>, -c <name>   Build release for a specific configuration"
            echo "  --publish                    Upload release binaries to Miniserve and create git tag"
            echo "  --version <ver>, -v <ver>    Specify release version (e.g. 0.0.2). If omitted, auto-increments from server"
            echo "  --help, -h                   Show this help message"
            return 0 2>/dev/null || exit 0
            ;;
        -*)
            echo "Error: Unknown option '$1'"
            return 1 2>/dev/null || exit 1
            ;;
        *)
            if [ -z "$target_config" ]; then
                target_config="$1"
            fi
            shift
            ;;
    esac
done

# Verify configuration exists if specified
if [ -n "$target_config" ]; then
    config_found=false
    for config in "${BUILD_CONFIGS[@]}"; do
        IFS=';' read -r config_name c_compiler cxx_compiler extra_flags custom_platform <<< "$config"
        if [ "$target_config" = "$config_name" ]; then
            config_found=true
            break
        fi
    done
    if [ "$config_found" = false ]; then
        echo "Error: Unknown configuration '$target_config'"
        echo "Available configurations: "
        for config in "${BUILD_CONFIGS[@]}"; do
            IFS=';' read -r config_name c_compiler cxx_compiler extra_flags custom_platform <<< "$config"
            echo "  - $config_name"
        done
        return 1 2>/dev/null || exit 1
    fi
fi

# Detect platform for a given configuration
detect_platform() {
    local cfg_name="$1"
    local c_comp="$2"
    local custom_plat="$3"

    if [ -n "$custom_plat" ]; then
        echo "$custom_plat"
        return
    fi

    if [[ "$cfg_name" == *"wasm"* ]] || [[ "$c_comp" == *"emcc"* ]]; then
        echo "wasm"
        return
    fi

    local os_type
    os_type="$(uname -s | tr '[:upper:]' '[:lower:]')"
    local arch
    arch="$(uname -m)"

    case "$os_type" in
        linux*)
            echo "linux_${arch}"
            ;;
        darwin*)
            echo "darwin_${arch}"
            ;;
        msys*|mingw*|cygwin*|windows*)
            echo "windows64"
            ;;
        *)
            echo "${os_type}_${arch}"
            ;;
    esac
}

built_count=0
failed_configs=()
skipped_configs=()
processed_release_dirs=()

# Remove old release output root if doing a fresh build
# mkdir -p "$WORKSPACE_DIR/release"

for config in "${BUILD_CONFIGS[@]}"; do
    IFS=';' read -r config_name c_compiler cxx_compiler extra_flags custom_platform <<< "$config"
    
    # If a specific configuration was requested, skip others
    if [ -n "$target_config" ] && [ "$target_config" != "$config_name" ]; then
        continue
    fi
    
    # Check if compiler exists
    eval_c_compiler=$(eval echo "$c_compiler")
    eval_cxx_compiler=$(eval echo "$cxx_compiler")
    
    if [[ "$eval_c_compiler" == /* ]] || [[ "$eval_c_compiler" == ./* ]] || [[ "$eval_c_compiler" == ../* ]]; then
        if [ ! -x "$eval_c_compiler" ]; then
            echo "Warning: Compiler executable not found at $eval_c_compiler. Skipping $config_name."
            skipped_configs+=("$config_name")
            continue
        fi
    else
        if ! command -v "$eval_c_compiler" >/dev/null 2>&1; then
            echo "Warning: Compiler $eval_c_compiler not found in PATH. Skipping $config_name."
            skipped_configs+=("$config_name")
            continue
        fi
    fi
    
    platform=$(detect_platform "$config_name" "$eval_c_compiler" "$custom_platform")

    echo "========================================================================"
    echo "Building Release Configuration: $config_name"
    echo "Platform:                      $platform"
    echo "C Compiler:                    $eval_c_compiler"
    echo "C++ Compiler:                  $eval_cxx_compiler"
    echo "Target Executable:             $APP_TARGET"
    echo "========================================================================"
    
    build_dir="$WORKSPACE_DIR/build/$config_name"
    mkdir -p "$build_dir"
    
    # Run cmake configure for Release
    cmake -G Ninja \
          -DCMAKE_C_COMPILER="$eval_c_compiler" \
          -DCMAKE_CXX_COMPILER="$eval_cxx_compiler" \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_CXX_FLAGS="-DMLA_APP_VERSION=\\\"${specified_version:-0.0.1}\\\"" \
          $extra_flags \
          -S "$WORKSPACE_DIR" \
          -B "$build_dir"
          
    if [ $? -ne 0 ]; then
        echo "Error: CMake configuration failed for $config_name"
        failed_configs+=("$config_name")
        continue
    fi
    
    # Run cmake build for app target specifically or release build
    if cmake --build "$build_dir" --target "$APP_TARGET" -j$(nproc) 2>/dev/null; then
        echo "Target '$APP_TARGET' built successfully."
    else
        cmake --build "$build_dir" -j$(nproc)
    fi

    if [ $? -ne 0 ]; then
        echo "Error: Build failed for $config_name"
        failed_configs+=("$config_name")
        continue
    fi
    
    # Create target release directory structure: release/{platform}/{compiler}/
    release_dir="$WORKSPACE_DIR/release/${platform}/${config_name}"
    mkdir -p "$release_dir"

    # Find and copy ONLY the app binary (and associated runtime files if wasm), excluding test binaries
    copied_any=false
    
    # Search for files matching APP_TARGET or executables in build_dir excluding test binaries
    while IFS= read -r -d '' binary_file; do
        bname=$(basename "$binary_file")
        # Explicitly exclude test binaries
        if [[ "$bname" == *"test"* ]] || [[ "$bname" == *"Test"* ]]; then
            continue
        fi
        
        # Check if file is executable or app target match
        if [[ "$bname" == "$APP_TARGET"* ]] || [ -x "$binary_file" ]; then
            cp -f "$binary_file" "$release_dir/"
            echo "Copied app binary: $bname -> release/${platform}/${config_name}/"
            copied_any=true
        fi
    done < <(find "$build_dir" -maxdepth 2 -type f \( -name "$APP_TARGET" -o -name "${APP_TARGET}.exe" -o -name "${APP_TARGET}.wasm" -o -name "${APP_TARGET}.js" \) -print0)

    # Fallback search if direct match wasn't found above
    if [ "$copied_any" = false ]; then
        while IFS= read -r -d '' binary_file; do
            bname=$(basename "$binary_file")
            if [[ "$bname" == *"test"* ]] || [[ "$bname" == *"Test"* ]] || [[ "$bname" == *.a ]] || [[ "$bname" == *.o ]] || [[ "$bname" == *.ninja* ]] || [[ "$bname" == CMake* ]]; then
                continue
            fi
            if [ -x "$binary_file" ]; then
                cp -f "$binary_file" "$release_dir/"
                echo "Copied app binary: $bname -> release/${platform}/${config_name}/"
                copied_any=true
            fi
        done < <(find "$build_dir" -maxdepth 1 -type f -executable -print0 2>/dev/null)
    fi

    if [ "$copied_any" = false ]; then
        echo "Warning: No app binary found to stage for configuration $config_name"
    else
        processed_release_dirs+=("${platform};${config_name};${release_dir}")
        built_count=$((built_count + 1))
    fi
done

echo "========================================================================"
echo "Release Build Summary:"
echo "  Successfully built: $built_count configurations"
if [ ${#skipped_configs[@]} -ne 0 ]; then
    echo "  Skipped (compiler missing): ${skipped_configs[*]}"
fi
if [ ${#failed_configs[@]} -ne 0 ]; then
    echo "  Failed: ${failed_configs[*]}"
    return 1 2>/dev/null || exit 1
fi

# Exit early if publish was not requested
if [ "$do_publish" = false ]; then
    echo "Release files staged in: $WORKSPACE_DIR/release/"
    return 0 2>/dev/null || exit 0
fi

echo "========================================================================"
echo "Publishing Release to Miniserve ($RELEASES_SERVER_URL)..."
echo "========================================================================"

# Determine version if not specified
VERSION="$specified_version"
if [ -z "$VERSION" ]; then
    echo "Fetching existing releases for '$PRODUCT_NAME' from $RELEASES_SERVER_URL/$PRODUCT_NAME/ ..."
    server_html=$(curl -sL "${RELEASES_SERVER_URL}/${PRODUCT_NAME}/?raw=true" 2>/dev/null || true)
    
    # Extract version strings like 0.0.1
    versions=($(echo "$server_html" | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | sort -V | uniq))
    
    if [ ${#versions[@]} -gt 0 ]; then
        latest_version="${versions[-1]}"
        echo "Latest version found on server: $latest_version"
        
        IFS='.' read -r major minor patch <<< "$latest_version"
        patch=$((patch + 1))
        VERSION="${major}.${minor}.${patch}"
        echo "Auto-incremented next version: $VERSION"
    else
        VERSION="0.0.1"
        echo "No existing version found on server. Using initial version: $VERSION"
    fi
else
    echo "Using specified version: $VERSION"
fi

# Function to recursively ensure remote subdirectories exist on Miniserve
ensure_remote_dir() {
    local target_dir="$1"
    IFS='/' read -ra parts <<< "$target_dir"
    local current=""
    for part in "${parts[@]}"; do
        if [ -n "$part" ]; then
            curl -s -o /dev/null -F "mkdir=${part}" "${RELEASES_SERVER_URL}/upload?path=${current}/" || true
            current="${current}/${part}"
        fi
    done
}

# Upload binaries for each processed release directory
upload_success=true
for item in "${processed_release_dirs[@]}"; do
    IFS=';' read -r plat cfg rdir <<< "$item"
    
    if [ ! -d "$rdir" ]; then
        continue
    fi

    # Check if local release directory contains any files before proceeding
    shopt -s nullglob
    rfiles=("$rdir"/*)
    shopt -u nullglob

    if [ ${#rfiles[@]} -eq 0 ]; then
        echo "Skipping publish for $plat/$cfg: release directory $rdir is empty."
        continue
    fi

    target_path="/${PRODUCT_NAME}/${VERSION}/${plat}/${cfg}"
    upload_url="${RELEASES_SERVER_URL}/upload?path=${target_path}"
    
    ensure_remote_dir "$target_path"
    
    echo "Uploading release artifacts for $plat/$cfg to $target_path ..."
    for file in "${rfiles[@]}"; do
        if [ -f "$file" ]; then
            fname=$(basename "$file")
            echo "  - Uploading $fname ..."
            res=$(curl -s -w "%{http_code}" -o /dev/null -F "path=@${file}" "$upload_url")
            if [[ "$res" =~ ^2[0-9][0-9]$ ]] || [[ "$res" == "302" ]] || [[ "$res" == "303" ]]; then
                echo "    Uploaded successfully (HTTP $res)."
            else
                echo "    Upload failed for $fname (HTTP $res)."
                upload_success=false
            fi
        fi
    done
done

if [ "$upload_success" = true ]; then
    # Create Git Tag only after successful uploads
    echo "========================================================================"
    echo "Creating Git Release Tag v${VERSION}..."
    echo "========================================================================"

    git_tag="v${VERSION}"
    if git rev-parse "$git_tag" >/dev/null 2>&1; then
        echo "Warning: Git tag '$git_tag' already exists."
    else
        git tag -a "$git_tag" -m "Release $git_tag"
        if [ $? -eq 0 ]; then
            echo "Successfully created git tag '$git_tag'."
            
            if git remote | grep -q 'origin'; then
                echo "Pushing tag '$git_tag' to remote origin..."
                git push origin "$git_tag"
            fi
        else
            echo "Error: Failed to create git tag '$git_tag'."
        fi
    fi
    echo "========================================================================"
    echo "Successfully published release $VERSION to $RELEASES_SERVER_URL/${PRODUCT_NAME}/${VERSION}/"
    echo "========================================================================"
    return 0 2>/dev/null || exit 0
else
    echo "Error: Release publish failed due to upload errors. Git tag creation aborted."
    return 1 2>/dev/null || exit 1
fi
