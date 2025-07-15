# LargeCoordinates

A high-precision coordinate system for large-scale 3D worlds using cell-based partitioning.

## Building and Testing

### Prerequisites
- CMake 3.10 or higher
- C++11 compatible compiler

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

## LargePosition Features

- **Dual coordinate system**: Combines integer cell indices with local float offsets
- **High precision**: Maintains accuracy at astronomical scales (+/-30AU)
- **Hysteresis**: Reduces cell boundary jitter
- **Simple API**: Easy coordinate conversion between reference frames

### Usage Example

```cpp
#include "LargeCoordinates.h"

// Create positions using different constructors
LargePosition ship(int3(100, 50, -25), float3(1500.0f, 800.0f, 1200.0f)); // From global/local
LargePosition station(2500.0, 1000.0, -5000.0); // From world coordinates (double precision)
LargePosition origin; // Default constructor (0,0,0)

// Get ship position relative to station's cell
float3 relative_pos = ship.get_local(station.global);

// Move ship and update position
LargePosition new_ship_pos;
new_ship_pos.set_from_local(ship.global, relative_pos + float3(100.0f, 0.0f, 0.0f));

// Create from large world coordinates (space sim scale)
LargePosition distant_object(1e9, -5e8, 2e9); // 1 billion units away
``` 