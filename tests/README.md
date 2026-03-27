# Test Infrastructure

Minimal test framework for validating 3D Gaussian Splatting changes.

## Design Philosophy

This test infrastructure is intentionally **lightweight and pragmatic**:

- ✅ **No external dependencies** (GoogleTest, Catch2, etc.)
- ✅ **Simple registration macro** for adding tests
- ✅ **Minimal boilerplate** - focus on test logic
- ✅ **Fast compilation** - tests compile in seconds
- ✅ **Easy to extend** - add new test files with same pattern

## Structure

```
tests/
├── TestRunner.h              # Minimal test framework
├── ChunkSchedulerTest.cpp    # Change 1: Threshold tests
├── ChunkDispatchTest.cpp     # Change 3: Dispatch tests
└── CMakeLists.txt.fragment   # CMake config (copy to main CMakeLists.txt)
```

## Writing a Test

### Basic Test

```cpp
#include "TestRunner.h"

namespace gs::test {

    void my_test_function() {
        int result = computeSomething();
        assert_equal(42, result, "Expected 42");
    }

    // Register test to run automatically
    REGISTER_TEST(MyTestName, my_test_function);

} // namespace gs::test

// Main function
int main() {
    return gs::test::TestRunner::instance().run_all();
}
```

### Available Assertions

```cpp
// Boolean assertion
assert_true(condition, "Optional message");

// Equality assertion (works with integers, size_t, etc.)
assert_equal(expected, actual, "Context description");

// Throws std::runtime_error on failure
```

### Test Registration

```cpp
// This macro auto-registers your test
REGISTER_TEST(TestName, test_function);

// TestName: Identifier for the test
// test_function: Function with signature void()
```

## Running Tests

### Build Tests
```powershell
cmake -B build -DBUILD_TESTS=ON
cmake --build build --config Debug --target chunk_scheduler_test
```

### Run Single Test
```powershell
.\build\Debug\chunk_scheduler_test.exe
```

### Run All Tests
```powershell
.\scripts\run_tests.ps1
```

## Adding a New Test File

1. **Create test file** `tests/MyFeatureTest.cpp`:

```cpp
#include "TestRunner.h"
#include "render/MyFeature.h"

namespace gs::test {

    void test_feature_basic() {
        MyFeature feature;
        assert_equal(0, feature.getValue());
    }

    void test_feature_advanced() {
        MyFeature feature;
        feature.setValue(42);
        assert_equal(42, feature.getValue());
    }

    REGISTER_TEST(FeatureBasic, test_feature_basic);
    REGISTER_TEST(FeatureAdvanced, test_feature_advanced);

} // namespace gs::test

int main() {
    std::cout << "=== My Feature Tests ===\n\n";
    return gs::test::TestRunner::instance().run_all();
}
```

2. **Add to CMake** (in `tests/CMakeLists.txt.fragment`):

```cmake
add_executable(my_feature_test
    tests/MyFeatureTest.cpp
    src/render/MyFeature.cpp  # If implementation exists
)
target_include_directories(my_feature_test PRIVATE src tests)
```

3. **Add to test runner script** (`scripts/run_tests.ps1`):

```powershell
$testExecutables = @(
    "chunk_scheduler_test.exe",
    "chunk_dispatch_test.exe",
    "my_feature_test.exe"  # ADD HERE
)
```

## TDD Workflow

### Red → Green → Refactor

1. **RED**: Write a failing test
   ```powershell
   .\build\Debug\my_feature_test.exe
   # Expected output: FAIL - MyFeature doesn't exist
   ```

2. **GREEN**: Write minimal code to pass
   ```cpp
   // Implement just enough to pass the test
   ```
   ```powershell
   cmake --build build --target my_feature_test
   .\build\Debug\my_feature_test.exe
   # Expected output: PASS
   ```

3. **REFACTOR**: Improve code, tests stay green
   ```powershell
   # Tests should still pass after refactoring
   .\build\Debug\my_feature_test.exe
   ```

## GPU Tests (Special Case)

For GPU-side validation (e.g., compute shaders):

### Option 1: Mock GPU Logic in CPU Test
Test the *algorithm* without actual GPU calls:

```cpp
void test_compaction_algorithm() {
    std::vector<uint32_t> input = {0, 1, 2, 3, 4};
    std::vector<bool> visible = {true, false, true, false, true};
    
    auto result = compact_indices_cpu(input, visible);
    
    assert_equal<size_t>(3, result.size());
    assert_equal<uint32_t>(0, result[0]);
    assert_equal<uint32_t>(2, result[1]);
    assert_equal<uint32_t>(4, result[2]);
}
```

### Option 2: Minimal GPU Test Harness
Requires OpenGL context initialization (see `VALIDATION_STRATEGY.md`).

## Best Practices

### ✅ DO:
- Write tests **before** implementation (TDD)
- Keep tests **focused** - one concept per test
- Use **descriptive names** - `test_threshold_at_boundary()` not `test1()`
- Test **edge cases** - zero, negative, maximum values
- **Assert** preconditions and postconditions

### ❌ DON'T:
- Write tests after the code is "done"
- Test implementation details (internal state)
- Share state between tests (no global variables)
- Ignore failing tests ("I'll fix it later")
- Skip edge cases ("that won't happen")

## Debugging Failed Tests

### Visual Studio
```powershell
# Build with debug symbols
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build build --config Debug

# Open in Visual Studio
devenv build\gaussian_splatting_gl.sln

# Set breakpoint in test, run with F5
```

### Command Line
```powershell
# Add debug prints
void test_something() {
    std::cout << "Input: " << value << "\n";  # Add this
    assert_equal(expected, value);
}
```

## Coverage Tracking (Optional)

For coverage metrics (when needed):

### MSVC
```powershell
# Install OpenCppCoverage (chocolatey)
choco install opencppcoverage

# Run with coverage
OpenCppCoverage --sources src `
                --excluded_sources libs `
                -- .\build\Debug\chunk_scheduler_test.exe
```

### Manual Coverage Check
Just read your test file and implementation side-by-side:
- ✅ All public methods have tests?
- ✅ All branches (if/else) covered?
- ✅ All error conditions tested?

## Integration with CI/CD

### GitHub Actions Example
```yaml
- name: Build Tests
  run: |
    cmake -B build -DBUILD_TESTS=ON
    cmake --build build --config Release

- name: Run Tests
  run: .\scripts\run_tests.ps1
```

## Why Not GoogleTest/Catch2?

**Pros of external frameworks:**
- More features (fixtures, parameterized tests, etc.)
- Better reporting
- Industry standard

**Cons for this project:**
- +5-10 minutes build time
- Additional dependencies
- Overkill for ~10 targeted validation tests
- Slows down iteration

**This framework:**
- Compiles in <5 seconds
- Zero dependencies
- Sufficient for incremental validation
- Can migrate to GoogleTest later if test suite grows

## When to Graduate to a Full Framework

Consider upgrading when:
- More than 50 test files
- Need fixtures, parameterized tests, or complex mocking
- Multiple developers need standardized test infrastructure
- Test execution time becomes a bottleneck

Until then, **simple is better**.

## Questions?

See:
- `QUICK_START_VALIDATION.md` - Step-by-step workflows
- `VALIDATION_STRATEGY.md` - Comprehensive technical strategy
- Existing tests - `ChunkSchedulerTest.cpp`, `ChunkDispatchTest.cpp`
