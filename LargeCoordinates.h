#pragma once

#include <cassert>
#include <cmath>

struct int3
{
    int x, y, z;

    int3()
        : x(0)
        , y(0)
        , z(0)
    {
    }
    int3(int x_, int y_, int z_)
        : x(x_)
        , y(y_)
        , z(z_)
    {
    }

    int3 operator+(const int3& other) const { return int3(x + other.x, y + other.y, z + other.z); }
    int3 operator-(const int3& other) const { return int3(x - other.x, y - other.y, z - other.z); }
    int3 operator*(int scalar) const { return int3(x * scalar, y * scalar, z * scalar); }

    bool operator==(const int3& other) const { return x == other.x && y == other.y && z == other.z; }
    bool operator!=(const int3& other) const { return !(*this == other); }
};

struct float3
{
    float x, y, z;

    float3()
        : x(0.0f)
        , y(0.0f)
        , z(0.0f)
    {
    }
    float3(float x_, float y_, float z_)
        : x(x_)
        , y(y_)
        , z(z_)
    {
    }

    float3 operator+(const float3& other) const { return float3(x + other.x, y + other.y, z + other.z); }
    float3 operator-(const float3& other) const { return float3(x - other.x, y - other.y, z - other.z); }
    float3 operator*(float scalar) const { return float3(x * scalar, y * scalar, z * scalar); }
    float3 operator/(float scalar) const { return float3(x / scalar, y / scalar, z / scalar); }

    bool operator==(const float3& other) const
    {
        return std::abs(x - other.x) < 1e-6f && std::abs(y - other.y) < 1e-6f && std::abs(z - other.z) < 1e-6f;
    }
    bool operator!=(const float3& other) const { return !(*this == other); }
};

struct double3
{
    double x, y, z;

    double3()
        : x(0.0)
        , y(0.0)
        , z(0.0)
    {
    }
    double3(double x_, double y_, double z_)
        : x(x_)
        , y(y_)
        , z(z_)
    {
    }

    double3 operator+(const double3& other) const { return double3(x + other.x, y + other.y, z + other.z); }
    double3 operator-(const double3& other) const { return double3(x - other.x, y - other.y, z - other.z); }
    double3 operator*(double scalar) const { return double3(x * scalar, y * scalar, z * scalar); }
    double3 operator/(double scalar) const { return double3(x / scalar, y / scalar, z / scalar); }

    bool operator==(const double3& other) const
    {
        return std::abs(x - other.x) < 1e-15 && std::abs(y - other.y) < 1e-15 && std::abs(z - other.z) < 1e-15;
    }
    bool operator!=(const double3& other) const { return !(*this == other); }
};

/*

The LargePosition struct represents a high-precision position in a large,
cell-partitioned 3D world using a double-layer coordinate system.

It combines:
  global: an int3 representing the center coordinates of the spatial cell
  local: a float3 representing the local offset within the current cell.

Each cell is a cubic region of size CELL_SIZE (2048 units by default) centered on global * CELL_SIZE.
The cell at global(0,0,0) covers world coordinates [-CELL_SIZE/2, CELL_SIZE/2) in each dimension.
The absolute world position is: `world_position = global * CELL_SIZE + local`

PRECISION CHARACTERISTICS:
- Maintains consistent precision across the entire supported range (+/-29.3 AU)
- Typical precision: 0.000244 meters (FP32 ULP at CELL_SIZE)
- Minimum precision: 0.000488 meters (worst case at maximum local offset)
- Range limits: MIN_COORDINATE to MAX_COORDINATE (~+/-4.398e12 meters)

Note: The system uses loose cell partitioning with hysteresis
Local coordinates can extend beyond the natural cell boundary (+/-CELL_SIZE/2) up to +/-CELL_SIZE
to reduce jitter and avoid frequent cell switching when an object hovers near a boundary.

The dual coordinate system prevents precision loss that would occur with naive 
large-coordinate approaches, maintaining sub-meter accuracy even at astronomical scales.

*/
struct LargePosition
{
    // Usable range = -4,398,046,509,056 to 4,398,046,509,056 meters, which is -29.3au to 29.3au

    // FP32 ULP at 2048.0 = 0.000244
    static constexpr float CELL_SIZE = 2048.0f;

    // System range limits (usable coordinate range)
    static constexpr double MIN_COORDINATE = static_cast<double>(INT_MIN) * CELL_SIZE;  // ~-4.398e12 meters (~-29.3 AU)
    static constexpr double MAX_COORDINATE = static_cast<double>(INT_MAX) * CELL_SIZE;  // ~+4.398e12 meters (~+29.3 AU)
    
    // Precision characteristics (consistent across entire supported range)
    static constexpr float MIN_PRECISION = 0.000488f;  // Worst-case precision at maximum local offset (FP32 ULP at 6144.0)
    static constexpr float TYPICAL_PRECISION = 0.000244f;  // Typical precision at CELL_SIZE (FP32 ULP at 2048.0)
    
    // Useful astronomical constant for space simulation
    static constexpr double AU_DISTANCE = 149597870700.0;  // 1 Astronomical Unit in meters

    // global coordinates (cell center)
    int3 global;

    // local coordinates (offset from cell center)
    float3 local;

    // Default constructor
    LargePosition()
        : global(0, 0, 0)
        , local(0.0f, 0.0f, 0.0f)
    {
    }

    // Constructor from global and local coordinates
    LargePosition(const int3& global_, const float3& local_) { from_float3(global_, local_); }

    LargePosition(const double3& val) { from_double3(val); }

    // Set position from world coordinates (double precision for large values)
    // Automatically assigns to the nearest cell center to minimize local offset
    void from_double3(const double3& val)
    {
        // Validate input coordinates are within supported range
        assert(val.x >= MIN_COORDINATE && val.x <= MAX_COORDINATE && 
               "X coordinate exceeds supported range (~+/-29.3 AU)");
        assert(val.y >= MIN_COORDINATE && val.y <= MAX_COORDINATE && 
               "Y coordinate exceeds supported range (~+/-29.3 AU)");
        assert(val.z >= MIN_COORDINATE && val.z <= MAX_COORDINATE && 
               "Z coordinate exceeds supported range (~+/-29.3 AU)");

        // Find nearest cell center (rounds to nearest integer)
        global.x = static_cast<int>(std::round(val.x / CELL_SIZE));
        global.y = static_cast<int>(std::round(val.y / CELL_SIZE));
        global.z = static_cast<int>(std::round(val.z / CELL_SIZE));

        // Calculate local offset from the chosen cell center
        local.x = static_cast<float>(val.x - global.x * double(CELL_SIZE));
        local.y = static_cast<float>(val.y - global.y * double(CELL_SIZE));
        local.z = static_cast<float>(val.z - global.z * double(CELL_SIZE));
    }

    // Convert to world coordinates as double precision
    double3 to_double3() const
    {
        return double3(global.x * double(CELL_SIZE) + local.x, global.y * double(CELL_SIZE) + local.y, global.z * double(CELL_SIZE) + local.z);
    }

    // Convert this position to local coordinates relative to the specified origin cell center
    // Returns the offset from origin's cell center to this position
    float3 to_float3(const int3& origin) const
    {
        int3 d = global - origin;
        float3 local_pos = local + float3(d.x * CELL_SIZE, d.y * CELL_SIZE, d.z * CELL_SIZE);

        // Assert that local position doesn't exceed reasonable bounds for relative positioning
        // This catches logic errors where positions are too far from reference frame
        // With center-based cells and hysteresis, reasonable bound is ~3 cell sizes

        // FP32 ULP at 6144.0 = 0.000488
        assert(std::abs(local_pos.x) <= CELL_SIZE * 3.0f &&
               "The distance to the provided origin is too large to be represented as a float3.");
        assert(std::abs(local_pos.y) <= CELL_SIZE * 3.0f &&
               "The distance to the provided origin is too large to be represented as a float3.");
        assert(std::abs(local_pos.z) <= CELL_SIZE * 3.0f &&
               "The distance to the provided origin is too large to be represented as a float3.");
        return local_pos;
    }

    // Set this position from local coordinates relative to the specified origin cell center
    // local_pos is the offset from origin's cell center to the desired world position
    void from_float3(const int3& origin, const float3& local_)
    {
        // Hysteresis-based cell selection to reduce switching near boundaries
        // Natural cell boundary is +/-CELL_SIZE/2, but allow extension to +/-CELL_SIZE*0.75
        constexpr float THRESHOLD = CELL_SIZE * 0.75f;

        if (std::abs(local_.x) <= THRESHOLD && std::abs(local_.y) <= THRESHOLD && std::abs(local_.z) <= THRESHOLD)
        {
            // Position is within hysteresis threshold - keep using the origin cell
            global = origin;
            local = local_;
            return;
        }

        // Position exceeds hysteresis threshold assign a new cell
        double world_x = origin.x * double(CELL_SIZE) + local_.x;
        double world_y = origin.y * double(CELL_SIZE) + local_.y;
        double world_z = origin.z * double(CELL_SIZE) + local_.z;
        from_double3({world_x, world_y, world_z});
    }

    // Equality operators - compare actual world positions, not internal representation
    // With center-based cells and hysteresis, same world position can have different (global, local) pairs
    bool operator==(const LargePosition& other) const
    {
        // Early exit: if cell centers are too far apart, they can't represent the same position
        // With hysteresis threshold of CELL_SIZE from center, positions can differ by ~3 cells max
        int3 global_diff = global - other.global;
        if (std::abs(global_diff.x) > 3 || std::abs(global_diff.y) > 3 || std::abs(global_diff.z) > 3)
        {
            return false;
        }

        // Convert both positions to this object's reference frame and compare
        // Using this->global avoids triggering to_float3() bounds assertion
        const float3& this_local = local;
        float3 other_local = other.to_float3(global);

        // Use small tolerance for floating point comparison
        constexpr float tolerance = 1e-6f;
        return std::abs(this_local.x - other_local.x) < tolerance && std::abs(this_local.y - other_local.y) < tolerance &&
               std::abs(this_local.z - other_local.z) < tolerance;
    }

    bool operator!=(const LargePosition& other) const { return !(*this == other); }
};
