# LargeCoordinates

[![CI](https://github.com/SergeyMakeev/LargeCoordinates/workflows/CI/badge.svg)](https://github.com/SergeyMakeev/LargeCoordinates/actions)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

A high-precision coordinate system for large-scale 3D worlds using cell-based partitioning.

## Building and Testing

### Prerequisites
- CMake 3.14 or higher
- C++17 compatible compiler
- Internet connection (for automatic download of Google Test via FetchContent)

### Build Instructions

**Quick Option (Windows):**
```cmd
build_and_test.bat  # Build and run tests
clean.bat           # Clean build directory
```

**Manual Steps:**

1. Create a build directory:
```bash
mkdir build
cd build
```

2. Configure and build:
```bash
cmake ..
cmake --build .
```

### Running Tests

The project uses **Google Test** framework for unit testing. Google Test is automatically downloaded and built via CMake's FetchContent feature.

**Quick Option (Windows):**
```cmd
build_and_test.bat
```

**Manual Steps:**

Run the unit tests directly:
```bash
./Debug/test_large_coordinates.exe  # Windows
./test_large_coordinates            # Linux/Mac
```

Or use CTest:
```bash
ctest -C Debug --output-on-failure  # Windows
ctest --output-on-failure           # Linux/Mac
```

**Google Test Features:**
```bash
# Run specific test
./test_large_coordinates --gtest_filter=LargePositionTest.BasicConstruction

# List all tests  
./test_large_coordinates --gtest_list_tests

# Run tests with verbose output
./test_large_coordinates --gtest_filter="*" --gtest_brief=0
```

## LargePosition Features

- **Dual coordinate system**: Combines integer cell indices with local float offsets
- **Consistent precision**: Maintains 0.244-0.488 millimeter accuracy across entire +/-29.3 AU range
- **No precision degradation**: Sub-millimeter accuracy even at astronomical scales
- **Hysteresis**: Reduces cell boundary jitter with 0.75x cell size threshold
- **Range validation**: Input validation prevents integer overflow beyond supported range
- **Simple API**: Easy coordinate conversion between reference frames

### Precision Characteristics

The LargePosition system maintains consistent precision across its entire supported range:

| Metric | Value | Description |
|--------|-------|-------------|
| **Supported Range** | +/-29.3 AU | ~+/-4.398e12 meters (MIN_COORDINATE to MAX_COORDINATE) |
| **Typical Precision** | 0.000244 meters | Standard accuracy at cell centers |
| **Minimum Precision** | 0.000488 meters | Worst-case accuracy at maximum local offsets |
| **Cell Size** | 2048 meters | Spatial partitioning granularity |
| **Hysteresis Threshold** | 1536 meters | 0.75 * CELL_SIZE boundary switching tolerance |

**Key Benefits:**
- No precision loss at large scales (unlike naive float coordinates)
- Consistent sub-millimeter accuracy from origin to 29.3 AU
- Input validation prevents coordinates beyond supported range
- Designed for space simulation requirements

### Usage Example

```cpp
#include "LargeCoordinates.h"

// Create positions using different constructors
LargePosition ship(int3(100, 50, -25), float3(1500.0f, 800.0f, 1200.0f)); // From global/local
LargePosition station(double3(2500.0, 1000.0, -5000.0)); // From world coordinates (double precision)
LargePosition origin; // Default constructor (0,0,0)

// Get ship position relative to station's cell
float3 relative_pos = ship.to_float3(station.global);

// Move ship and update position
LargePosition new_ship_pos;
new_ship_pos.from_float3(ship.global, relative_pos + float3(100.0f, 0.0f, 0.0f));

// Create from large world coordinates (space sim scale)
LargePosition distant_object(double3(1e9, -5e8, 2e9)); // 1 billion units away

// Work with precision constants
double au_distance = LargePosition::AU_DISTANCE; // 1 AU in meters
LargePosition mars_orbit(double3(1.5 * au_distance, 0.0, 0.0)); // Mars at 1.5 AU

// Precision-aware distance checking
double3 pos1_world = ship.to_double3();
double3 pos2_world = station.to_double3();
double distance = std::sqrt(std::pow(pos1_world.x - pos2_world.x, 2) + 
                           std::pow(pos1_world.y - pos2_world.y, 2) + 
                           std::pow(pos1_world.z - pos2_world.z, 2));

// Use appropriate precision for comparisons
bool positions_equal = distance < LargePosition::TYPICAL_PRECISION;

// Check range limits before construction
if (coordinate_x >= LargePosition::MIN_COORDINATE && 
    coordinate_x <= LargePosition::MAX_COORDINATE) {
    LargePosition safe_pos(double3(coordinate_x, coordinate_y, coordinate_z));
}
``` 