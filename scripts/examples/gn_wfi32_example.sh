#!/usr/bin/env bash

#
#    Copyright (c) 2021 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

set -e
# Build script for GN PIC32MZW1 examples GitHub workflow.
source "$(dirname "$0")/../../scripts/activate.sh"

set -x
env

# Build steps
EXAMPLE_DIR=$1
shift
OUTPUT_DIR=out/example_app

if [[ ! -z "$1" ]]; then
    OUTPUT_DIR=$1
    shift
fi

GN_ARGS=()
NINJA_ARGS=()
for arg; do
    case $arg in
        -v)
            NINJA_ARGS+=(-v)
            ;;
        *=*)
            GN_ARGS+=("$arg")
            ;;
        *import*)
            GN_ARGS+=("$arg")
            ;;
        *)
            echo >&2 "invalid argument: $arg"
            exit 2
            ;;
    esac
done

if [ ! -f "lwip_update" ]; then
    echo "modify lwip mem_free"
    find ./third_party/lwip/ -type f -exec sed -i 's/mem_free(/mem_free_l(/g' {} +
    find ./third_party/lwip/ -type f -exec sed -i 's/mem_free,/mem_free_l,/g' {} +
    touch lwip_update
fi

gn -v gen --check --fail-on-unused-args "$OUTPUT_DIR" --args="home_dir=\"$HOME\" dfp_version=\"1.6.220\" ${GN_ARGS[*]}" --root="$EXAMPLE_DIR"
ninja -C "$OUTPUT_DIR" "${NINJA_ARGS[@]}"

#more debug log command
#ninja -v -C "$OUTPUT_DIR" "${NINJA_ARGS[@]}"

xc32-bin2hex "$OUTPUT_DIR"/*.elf
