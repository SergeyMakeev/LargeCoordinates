# Large World Coordinates (LWC)

[![CI](https://github.com/SergeyMakeev/LargeCoordinates/workflows/CI/badge.svg)](https://github.com/SergeyMakeev/LargeCoordinates/actions)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

---

## Goal
Enable robust support for **Large World Coordinates** in a game engine, allowing simulation and rendering at **planetary or solar system scales**, while preserving **sub-millimeter-level precision**.

---

## Problem Statement

Typically most game engine subsystems - including simulation, physics, rendering, and animation - operate using **32-bit floating-point (FP32)** coordinates.
However, FP32 precision is fundamentally limited by the number of bits in the mantissa (23 bits), which results in:

* Only ~1 mm precision within +/-2 km from the origin.
* Rapid loss of precision beyond +/-8 km.
* Severe floating-point artifacts such as jittering, Z-fighting, and broken collision at large distances.

It is fair to define the **"safe operating zone"** for FP32 as **+/-2 km**, where precision errors are negligible for most real-time applications.
This is very limiting for large-scale games involving cities, planets, or space simulations.

---

## Vision

Rather than rewriting all engine systems to use **double-precision (FP64)** math, which would be extremely risky, time-consuming, expensive, and often slower or unsupported by hardware:

1. **Retain FP32 as the core numeric type** for most engine subsystems (rendering, physics, audio, etc.)
2. **Introduce a dual coordinate system** that allows high-precision positioning using a combination of integer-based cell coordinates and FP32 local offsets.
3. **Ensure all relative computations are performed in a local space**, near a shared origin, to preserve FP32 precision.
4. Use extended precision positions only for internal high-level position bookkeeping, not for mass computation.

---

## Proposal

### Large Coordinate Representation

The LargePosition system implements a **dual-layer coordinate approach** that combines:

- **Global coordinates** (`int3 global`): Integer cell indices representing spatial regions 
- **Local coordinates** (`float3 local`): High-precision FP32 offsets within the current cell

Each spatial cell is a cubic region of **2048 units** (CELL_SIZE) centered on `global * CELL_SIZE`.  
The absolute world position is calculated as:
```
world_position = global * CELL_SIZE + local
```

This approach ensures that:
- **All FP32 calculations operate within a +/-1024 unit range** (half cell size), maintaining maximum precision
- **Integer arithmetic handles large-scale positioning** without precision loss
- **The system scales to astronomical distances** (+/-29.3 AU) while preserving sub-millimeter accuracy
- **Existing FP32-based engine subsystems require minimal changes** - they work with local coordinates in familiar ranges

Note: 29.3 AU = 4,383,200,000,000 meters = 14,429,286,447,034.12 ft

#### Precision Characteristics

The system maintains **consistent precision across the entire supported range**:
- **Typical precision**: 0.000244 meters (FP32 ULP at CELL_SIZE)
- **Minimum precision**: 0.000488 meters (worst case at maximum local offset)
- **No precision degradation**: Sub-millimeter accuracy even at 29.3 AU distances
- **Range validation**: Input coordinates are validated to prevent integer overflow

#### Key Design Principles

1. **Center-based cells**: Each cell is centered on `global * CELL_SIZE`
2. **Minimal precision loss**: Local coordinates never exceed +/-CELL_SIZE in normal operation
3. **Automatic cell assignment**: Constructor from world coordinates chooses optimal cell center
4. **Overflow protection**: Range validation prevents coordinate values beyond system limits

### Cell Partitioning with Hysteresis

To avoid jitter from rapid cell changes at boundaries, we implement **hysteresis-based active cell selection**.
This adds a dead zone buffer, ensuring that an object only switches cells when it moves a significant distance from the current one.

#### Implementation Details

The hysteresis system works as follows:

- **Natural cell boundary**: +/-CELL_SIZE/2 (+/-1024 units from cell center)
- **Hysteresis threshold**: +/-0.75 * CELL_SIZE (+/-1536 units from cell center)  
- **Extended tolerance**: Local coordinates can extend up to +/-CELL_SIZE during transitions

When updating position via `from_float3()`:

1. **Within threshold**: If the new local coordinates are within +/-1536 units, keep the current cell
2. **Beyond threshold**: If coordinates exceed +/-1536 units, assign a new optimal cell
3. **Smooth transitions**: Objects can temporarily have local coordinates beyond +/-1024 units during movement

#### Benefits

- **Eliminates boundary jitter**: Objects moving near cell edges don't rapidly switch between cells
- **Reduces computational overhead**: Fewer cell reassignments during continuous movement
- **Maintains precision**: Local coordinates remain within reasonable FP32 precision ranges
- **Predictable behavior**: Clear threshold rules for when cell switching occurs

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

### Real-World Use Cases

#### Space Simulation Games
- **Planetary orbits**: Position planets at accurate distances (Mars at 1.5 AU) with sub-millimeter precision
- **Spacecraft navigation**: Navigate between planets while maintaining precision for docking maneuvers
- **Asteroid fields**: Precisely position thousands of asteroids across solar system scales
- **Surface operations**: Land on planets and maintain sub-millimeter precision for surface activities

#### Large Open-World Games  
- **Continental maps**: Create entire continents with precision for both aerial and ground gameplay
- **Multiple planets**: Support games spanning multiple worlds without precision loss
- **Seamless transitions**: Move between space and planetary surface without coordinate system changes
- **Multiplayer synchronization**: Maintain consistent positioning across clients at any scale

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

// Work with astronomical distances
double au_distance = LargePosition::AU_DISTANCE; // 1 AU in meters
LargePosition mars_orbit(double3(1.5 * au_distance, 0.0, 0.0)); // Mars at 1.5 AU

// Convert back to world coordinates for display/calculations
double3 mars_world_pos = mars_orbit.to_double3();
printf("Mars position: %.3f AU\n", mars_world_pos.x / au_distance);

// Precision is maintained even at astronomical scales
LargePosition nearby_mars(double3(1.5 * au_distance + 1000.0, 0.0, 0.0)); // 1km offset
double3 nearby_world = nearby_mars.to_double3();
double distance_diff = nearby_world.x - mars_world_pos.x;
// distance_diff will be exactly 1000.0 meters
``` 

## Rendering Optimizations

The LargePosition system enables significant rendering optimizations by leveraging the fact that cells are large (>1km) and most rendered objects are typically near the camera.

### Suggested Camera-Local Rendering Strategy

Since each cell spans over 1 kilometer, the majority of visible objects are likely within the same cell as the camera. This enables a two-tier rendering approach:

#### Tier 1: Same-Cell Objects (Zero Cost)
When the camera and objects are in the same cell, rendering uses **local coordinates directly**:

```cpp
// Camera and object in same cell - use local coordinates directly
if (object.global == camera.global) {
    float3 object_position = object.local;
    float3 camera_position = camera.local;
    
    // Standard FP32 rendering pipeline - IDENTICAL to traditional approach
    // No coordinate conversion overhead whatsoever
    render_object(object_position, camera_position);
}
```

**Benefits:**
- **Zero computational overhead** - identical to traditional FP32 rendering
- **Maximum precision** - all calculations within +/-1024 unit range
- **No pipeline changes** - existing rendering code works unchanged
- **Optimal performance** - covers majority of visible objects

#### Tier 2: Cross-Cell Objects (Minimal Cost)
For objects in different cells, use the camera's cell as origin:

```cpp
// Object in different cell - convert to camera's reference frame
else {
    float3 object_relative = object.to_float3(camera.global);
    float3 camera_relative = camera.local;
    
    // Still using FP32 pipeline, just with converted coordinates
    render_object(object_relative, camera_relative);
}
```

**Benefits:**
- **Fast conversion** - `to_float3()` is simple arithmetic
- **High precision** - we keep our sub-millimeter precision guarantees
- **LOD integration** - natural distance-based level-of-detail opportunities

### Performance Characteristics

- **Same-cell objects**: 0% overhead vs traditional FP32 rendering
- **Cross-cell objects**: Single coordinate conversion per object
- **Natural culling**: Objects beyond ~3km automatically filtered out
- **Memory efficiency**: No need to store converted coordinates
- **Cache friendly**: Local coordinates stay within tight numeric ranges

This approach ensures that the rendering system gets the best of both worlds: **zero-cost precision for nearby objects** and **automatic distance management for far objects**, while maintaining full compatibility with existing FP32 rendering pipelines.

## Alternative Solutions

### GPU-Side Extended Precision (`df64`)

Reference: [Andrew Thall's Paper on Extended-Precision](https://andrewthall.org/papers/df64_qf128.pdf)

Some systems emulate high precision on the GPU using dual-float formats such as `df64`, which represent a value as a pair of FP32s `(hi, lo)`. This method:

* Enables GPU-side math at near-FP64 precision
* Avoids full switch to FP64 and retains GPU performance
* Requires custom arithmetic operations (addition, subtraction, etc.)

**Limitations**:

* Complex to integrate into existing pipelines
* Precision is not uniform; accuracy depends on value magnitude
* Only useful for positional data; not a full solution for interaction or physics

## Conclusion

The proposed **cell-relative coordinate system with hysteresis** enables massive, high-precision virtual worlds without sacrificing compatibility or performance. It allows the use of existing FP32-based systems by isolating them to **stable, local spaces**, and leverages integer-based cells for long-range stability.

This architecture balances scalability, accuracy, and integration cost - making it a practical choice for large-scale simulations across planets, space, or procedurally generated terrain. 




