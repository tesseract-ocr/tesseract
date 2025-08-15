# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

################################################################################
#
# Build Optimizations Module
#
# This module provides functions to apply modern CMake build optimizations
# to targets for faster and incremental builds.
#
################################################################################

#
# Function: apply_modern_optimizations
# Apply build optimizations to a target
#
# Parameters:
#   target_name - Name of the target to optimize
#   PCH_HEADERS - Optional list of headers for precompiled headers
#
function(apply_modern_optimizations target_name)
    # Parse arguments
    set(oneValueArgs )
    set(multiValueArgs PCH_HEADERS)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Apply Unity Build if enabled
    if(ENABLE_UNITY_BUILD)
        set_target_properties(${target_name} PROPERTIES UNITY_BUILD ON)
        # Use smaller batch sizes for libraries with many files
        get_target_property(target_type ${target_name} TYPE)
        if(target_type STREQUAL "STATIC_LIBRARY" OR target_type STREQUAL "SHARED_LIBRARY")
            set_target_properties(${target_name} PROPERTIES UNITY_BUILD_BATCH_SIZE 16)
        else()
            set_target_properties(${target_name} PROPERTIES UNITY_BUILD_BATCH_SIZE 8)
        endif()
        message(STATUS "Unity build enabled for ${target_name}")
    endif()

    # Apply Precompiled Headers if enabled and headers provided
    if(ENABLE_PRECOMPILED_HEADERS)
        if(ARG_PCH_HEADERS)
            target_precompile_headers(${target_name} PRIVATE ${ARG_PCH_HEADERS})
            message(STATUS "Precompiled headers enabled for ${target_name}")
        else()
            # Use common standard library headers as default
            target_precompile_headers(${target_name} PRIVATE
                <vector>
                <string>
                <memory>
                <algorithm>
                <iostream>
                <cstdlib>
                <cstring>
                <cmath>
            )
            message(STATUS "Default precompiled headers enabled for ${target_name}")
        endif()
    endif()

    # Configure build pools for Ninja
    if(ENABLE_NINJA_POOL AND CMAKE_GENERATOR STREQUAL "Ninja")
        set_target_properties(${target_name} PROPERTIES JOB_POOL_COMPILE compile)
        set_target_properties(${target_name} PROPERTIES JOB_POOL_LINK link)
    endif()

    # Apply compiler-specific optimizations
    if(MSVC)
        # Enable parallel compilation for MSVC if not already enabled
        get_target_property(target_compile_options ${target_name} COMPILE_OPTIONS)
        if(NOT target_compile_options MATCHES "/MP")
            target_compile_options(${target_name} PRIVATE "/MP")
        endif()

        # Enable function-level linking for better optimization
        target_compile_options(${target_name} PRIVATE "/Gy")

        # Enable intrinsic functions for better performance
        target_compile_options(${target_name} PRIVATE "/Oi")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # Enable split debug info for faster incremental builds
        if(CMAKE_BUILD_TYPE MATCHES Debug)
            target_compile_options(${target_name} PRIVATE "-gsplit-dwarf")
        endif()

        # Enable function sections for better dead code elimination
        target_compile_options(${target_name} PRIVATE "-ffunction-sections" "-fdata-sections")
    endif()
endfunction()

#
# Function: apply_training_optimizations
# Apply optimizations specific to training tools
#
function(apply_training_optimizations target_name)
    apply_modern_optimizations(${target_name}
        PCH_HEADERS
            <vector>
            <string>
            <memory>
            <iostream>
            <fstream>
            <cstdlib>
            <cstring>
    )

    # Training tools usually build faster, so smaller unity batches are fine
    if(ENABLE_UNITY_BUILD)
        set_target_properties(${target_name} PROPERTIES UNITY_BUILD_BATCH_SIZE 4)
    endif()
endfunction()

#
# Function: apply_test_optimizations
# Apply optimizations specific to test targets
#
function(apply_test_optimizations target_name)
    # Tests often have different compilation patterns
    if(ENABLE_PRECOMPILED_HEADERS)
        target_precompile_headers(${target_name} PRIVATE
            <gtest/gtest.h>
            <vector>
            <string>
            <memory>
            <iostream>
        )
        message(STATUS "Test precompiled headers enabled for ${target_name}")
    endif()

    # Tests benefit from unity builds but smaller batches
    if(ENABLE_UNITY_BUILD)
        set_target_properties(${target_name} PROPERTIES UNITY_BUILD ON)
        set_target_properties(${target_name} PROPERTIES UNITY_BUILD_BATCH_SIZE 8)
        message(STATUS "Unity build enabled for test ${target_name}")
    endif()

    # Configure Ninja pools
    if(ENABLE_NINJA_POOL AND CMAKE_GENERATOR STREQUAL "Ninja")
        set_target_properties(${target_name} PROPERTIES JOB_POOL_COMPILE compile)
        set_target_properties(${target_name} PROPERTIES JOB_POOL_LINK link)
    endif()
endfunction()
