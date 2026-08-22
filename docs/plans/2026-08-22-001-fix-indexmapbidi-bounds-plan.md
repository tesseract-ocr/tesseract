---
title: Fix IndexMapBiDi::SetMap sparse-index bounds - Plan
type: fix
date: 2026-08-22
artifact_contract: ce-unified-plan/v1
artifact_readiness: implementation-ready
execution: code
product_contract_source: ce-plan-bootstrap
---

## Goal

Prevent `IndexMapBiDi::SetMap` from writing outside `sparse_map_` when given an invalid sparse index, while preserving every valid mapping operation and adding focused regression coverage.

## Problem Frame

`IndexMapBiDi::Init` creates `sparse_map_` with valid indices `[0, size)`, but `SetMap` currently uses its caller-provided `sparse_index` directly as a `std::vector` subscript. Negative values and values at or above the initialized size therefore invoke undefined behavior and can produce an out-of-bounds write. Issue #3584 reproduces this with `Init(13, false)` followed by `SetMap(1324324, true)`; AddressSanitizer reports the write in `src/ccutil/indexmapbidi.cpp`.

`SetMap` is a `void` API and has no error result to return. The repository commonly protects externally-derived indices with an early return (for example, `src/ccutil/unicharset.h` and `src/ccutil/unicharcompress.cpp`), while debug assertions are used for internal preconditions. Since malformed or fuzzer-provided input must not remain unsafe in release builds, this fix uses a release-safe no-op for an invalid index rather than an assertion-only guard.

## Requirements

- **R1:** `SetMap` must not read or write `sparse_map_` for any negative index or any index greater than or equal to its current size.
- **R2:** An invalid call must return without changing any existing map entry; because the API returns `void`, no exception, status return, or signature change is introduced.
- **R3:** Valid boundary indices `0` and `SparseSize() - 1` must retain their existing mapped/unmapped behavior.
- **R4:** Existing valid `Init` → `SetMap` → `Setup`, many-to-one merge, copy, and serialization behavior must remain unchanged.
- **R5:** Add focused regression coverage in the existing `IndexMapBiDi` unit; exercise the ASan-enabled build so the pre-fix OOB write is detectable.
- **R6:** Keep the fix localized: no public API, build-system, unrelated caller, or unrelated test changes.

## Context / Constraints

- `SetMap` is documented for use after `Init`; an uninitialized map has an empty `sparse_map_`, so every index is invalid and must be ignored safely.
- `mapped == true` currently stores the temporary value `0`, and `mapped == false` stores `-1`; `Setup` assigns final compact indices. The guard must precede this assignment and must not alter either valid path.
- Check negative values before converting to `size_t`, then compare the nonnegative value with `sparse_map_.size()` to avoid signed/unsigned issues and to handle the issue's large positive value.
- Follow `CONTRIBUTING.md`: build and test the change, use the existing CMake test targets/build directories where available, and leave unrelated pre-existing files untouched.
- Do not add branding or badges to the implementation, tests, plan, or any eventual PR content.

## Assumptions

- Invalid `sparse_index` values are rejected by returning from `SetMap` without mutation. This is the selected behavior because it is safe in release builds and compatible with a `void` setter; callers that need diagnostics remain responsible for validating their own inputs.
- `Init` itself is outside this fix; tests use nonnegative sizes and the documented initialization sequence.
- Existing unchecked read APIs (`SparseToCompact`, `CompactToSparse`, `Merge`, and `MapFeatures`) and deserialization validation are separate concerns and are not widened into this issue.
- The current `build-asan` configuration has `BUILD_TESTS=ON`, AddressSanitizer enabled, and `DISABLED_LEGACY_ENGINE=OFF`; implementations should verify equivalent prerequisites if using another build directory.

## Key Technical Decisions

- **KTD1 — Runtime early return, not assert-only checking:** Validate `sparse_index < 0` first, then reject values whose nonnegative representation is outside `sparse_map_.size()`, and return before subscripting. An assertion alone would disappear under `NDEBUG` and leave the reported memory-safety defect in release builds.
- **KTD2 — Preserve valid storage semantics:** Do not resize the vector, clamp an invalid value, or change the `mapped ? 0 : -1` assignment. Invalid input is ignored; valid input is processed exactly as before.
- **KTD3 — Extend the existing legacy unit:** Add the regression to `unittest/indexmapbidi_test.cc`, which already covers `Init`, `SetMap`, `Setup`, and `ManyToOne`, rather than creating a new test target or changing CMake/Autotools lists.

## Implementation Units

### U1. Add out-of-range `SetMap` regression coverage

- **Requirements:** R1, R2, R3, R4, R5
- **Dependencies:** None; author and run this against the current implementation first where practical to establish the ASan failure.
- **File:** `unittest/indexmapbidi_test.cc`
- **Changes:**
  1. Add a focused test (for example, `IndexMapBiDiTest.SetMapOutOfBounds`) beside the existing `Primes` and `ManyToOne` tests.
  2. Initialize a small map, issue invalid calls for a negative index, the exclusive upper bound (`size`), and a substantially oversized positive index such as the issue reproducer. Include both `mapped` values where useful so the guard protects the complete setter operation.
  3. Call `Setup`, then assert that the map still has the initialized sparse size, contains only the deliberately valid mapping, and has the expected compact size. This proves invalid calls do not mutate valid entries or grow the map.
  4. Exercise valid boundary indices (`0` and `size - 1`) in the same test or a narrowly scoped companion test, asserting their expected compact mappings after `Setup`.
  5. Leave the existing `Primes` and `ManyToOne` scenarios unchanged; they are the regression checks for normal and many-to-one behavior.
- **Acceptance:** The new test fails on the unfixed ASan build because the oversized/negative call reaches the vector subscript, then passes without sanitizer diagnostics after U2. In a non-sanitized release build, the test passes after U2 and verifies the invalid calls are no-ops.

### U2. Add a release-safe bounds guard to `SetMap`

- **Requirements:** R1, R2, R3, R4, R6
- **Dependencies:** U1 (the regression should exist before implementation verification; implementation can be developed in parallel with the test).
- **File:** `src/ccutil/indexmapbidi.cpp`
- **Changes:**
  1. At the start of `IndexMapBiDi::SetMap`, return immediately when `sparse_index` is negative.
  2. For nonnegative values, compare a safe unsigned/size representation with `sparse_map_.size()` and return when it is outside the valid range.
  3. Keep the existing assignment unchanged for valid indices, with no vector resize, clamping, logging requirement, exception, or API signature change.
  4. Add a short implementation comment only if needed to explain that invalid external indices are ignored to prevent an out-of-bounds write; avoid unrelated refactoring.
- **Acceptance:** All invalid inputs in U1 complete without touching the vector; valid `0` and `size - 1` calls retain their mappings; the file remains warning-clean under the repository's normal compiler flags.

## Files to Modify

- `src/ccutil/indexmapbidi.cpp` - Add the early-return bounds validation to `IndexMapBiDi::SetMap`.
- `unittest/indexmapbidi_test.cc` - Add invalid-index and valid-boundary regression scenarios.

## New Files

- `docs/plans/2026-08-22-001-fix-indexmapbidi-bounds-plan.md` - This implementation plan; no new source, test, or build files are required.

## Test Scenarios

1. **Negative index:** After `Init(13, false)`, `SetMap(-1, true)` and/or `SetMap(-1, false)` returns without crashing or changing the map.
2. **Exclusive upper bound:** `SetMap(13, true)` on a size-13 map is ignored; no thirteenth entry is created.
3. **Large positive index / issue reproducer:** `SetMap(1324324, true)` is ignored and produces no ASan heap-buffer-overflow.
4. **Uninitialized/empty map:** If covered in the focused test, `SetMap(0, true)` on a default-constructed map returns safely; otherwise record this as a small edge assertion in the same test without calling `Setup` until after the check.
5. **Lower valid boundary:** `Init(2, false); SetMap(0, true); Setup()` produces one compact entry mapping sparse `0` to compact `0`.
6. **Upper valid boundary:** `Init(2, false); SetMap(1, true); Setup()` produces one compact entry mapping sparse `1` to compact `0`.
7. **Valid mapped/unmapped behavior:** A valid `SetMap(i, true)` followed by `Setup` maps `i`; a valid `SetMap(i, false)` leaves it unmapped, as before.
8. **Existing behavior:** `IndexMapBiDiTest.Primes` and `IndexMapBiDiTest.ManyToOne` continue to pass, including compact/sparse sizes and merge results.
9. **Sanitizer check:** Run the focused test under AddressSanitizer and confirm no out-of-bounds diagnostic for any invalid-index scenario.

## Verification

1. Confirm the baseline regression is meaningful by building/running the focused test from the existing ASan configuration before U2 where practical; the oversized invalid call should trigger ASan on the unfixed implementation. Do not treat a non-ASan baseline crash or hang as the required signal.
2. Build the focused target: `cmake --build build-asan --target indexmapbidi_test`.
3. Run the focused ASan test: `ctest --test-dir build-asan --output-on-failure -R '^indexmapbidi_test$'`.
4. Build and run the corresponding non-ASan target to prove release-safe behavior is not dependent on debug assertions: `cmake --build build --target indexmapbidi_test` followed by `ctest --test-dir build --output-on-failure -R '^indexmapbidi_test$'`.
5. Run the broader configured test suite (`ctest --test-dir build-asan --output-on-failure`, or the repository's configured `make check` per `CONTRIBUTING.md`) when practical; investigate any unrelated failures rather than altering scope.
6. Run `git diff --check` and inspect `git status --short --untracked-files=all`; only the intended implementation files and the intended plan/branch artifacts may be present, and no pre-existing stray file may be modified or staged.

## Scope Boundaries

### In scope

- A bounds guard in `IndexMapBiDi::SetMap`.
- Regression tests for negative, upper-out-of-range, large positive, empty-map (if retained), and valid boundary indices.
- Focused ASan/non-ASan build and test verification.

### Out of scope

- Changing `SetMap` to return a status, throw, assert-only, resize, or clamp.
- Changing the public header signature or adding a new API.
- Hardening `Init`, `SparseToCompact`, `CompactToSparse`, `Merge`, `MapFeatures`, or `DeSerialize` in this issue.
- Auditing every `IndexMapBiDi` caller or fixing unrelated index validation defects.
- Build-system, generated-file, dependency, formatting-only, or documentation changes beyond the minimal implementation/test comments needed to explain the guard.

## Risks / Open Questions

- **Resolved decision:** Invalid indices are ignored at runtime. This avoids release-mode memory corruption, matches the `void` API, and is preferable to an assertion that would not protect production builds.
- **Signed/unsigned conversion:** Casting a negative `int` to an unsigned type before checking it could turn `-1` into a huge positive value; the negative check must be evaluated first.
- **Legacy target availability:** `indexmapbidi_test` is under `LEGACY_TESTS` and can be omitted when `DISABLED_LEGACY_ENGINE=ON`; enable the legacy engine for this verification or explicitly report the skipped target.
- **Pre-fix sanitizer behavior:** The unfixed test is expected to terminate under ASan on the invalid write, so baseline-red evidence must be recorded separately from post-fix green verification.
- **Silent invalid input:** Ignoring malformed indices can hide a caller bug, but `SetMap` has no diagnostic return and memory safety takes precedence; caller-level validation is a follow-up concern, not a reason to retain UB.

## Definition of Done

- `SetMap` rejects every negative and out-of-range sparse index before vector access in both debug and release builds.
- Invalid calls leave all map state unchanged and do not resize or clamp the map.
- Valid index `0`, valid index `SparseSize() - 1`, mapped/unmapped values, `Setup`, and existing prime/many-to-one behavior remain correct.
- Focused regression tests cover the issue reproducer and required boundary cases.
- `indexmapbidi_test` passes in ASan and non-ASan configurations; the full configured test gate has no regression or any unrelated failure is documented.
- No public API, build-system, dependency, or unrelated caller changes are introduced.
- Only intended files are modified; pre-existing stray files remain untouched and no files are staged unexpectedly.
