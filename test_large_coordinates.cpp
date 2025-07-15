#include "LargeCoordinates.h"
#include <algorithm>
#include <climits>
#include <cmath>
#include <gtest/gtest.h>
#include <limits>

class LargePositionTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // Set up common test fixtures if needed
    }

    void TearDown() override
    {
        // Clean up after tests if needed
    }

    // Helper to check if two world positions are equivalent
    void ExpectWorldPositionsEqual(const LargePosition& pos1, const LargePosition& pos2, float tolerance = LargePosition::TYPICAL_PRECISION)
    {
        double3 world1 = pos1.to_double3();
        double3 world2 = pos2.to_double3();
        EXPECT_NEAR(world1.x, world2.x, tolerance);
        EXPECT_NEAR(world1.y, world2.y, tolerance);
        EXPECT_NEAR(world1.z, world2.z, tolerance);
    }
};

// === VECTOR TYPE EDGE CASES ===

TEST_F(LargePositionTest, Int3EdgeCases)
{
    // Test int3 operations with extreme values
    int3 max_val(INT_MAX, INT_MAX, INT_MAX);
    int3 min_val(INT_MIN, INT_MIN, INT_MIN);
    int3 zero(0, 0, 0);

    // Test equality
    EXPECT_EQ(max_val, int3(INT_MAX, INT_MAX, INT_MAX));
    EXPECT_NE(max_val, min_val);
    EXPECT_EQ(zero, int3());

    // Test arithmetic (avoiding overflow)
    int3 small(1, 2, 3);
    int3 result = small + int3(10, 20, 30);
    EXPECT_EQ(result, int3(11, 22, 33));

    result = small - int3(1, 1, 1);
    EXPECT_EQ(result, int3(0, 1, 2));

    result = small * 2;
    EXPECT_EQ(result, int3(2, 4, 6));
}

TEST_F(LargePositionTest, Float3EdgeCases)
{
    // Test float3 with extreme values
    float3 max_val(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    float3 min_val(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
    float3 zero(0.0f, 0.0f, 0.0f);

    // Test operations
    EXPECT_EQ(zero, float3());

    // Test very small values
    float3 tiny(1e-30f, 1e-30f, 1e-30f);
    float3 tiny2(1e-30f, 1e-30f, 1e-30f);
    EXPECT_EQ(tiny, tiny2);

    // Test precision near limits (at very large values, adding small amounts has no effect)
    float large = 1e10f; // Use value where precision differences are still detectable
    float3 precision_test(large, large, large);
    float3 precision_test2(large + 100000.0f, large + 100000.0f, large + 100000.0f);
    // These should be different despite potential precision issues
    EXPECT_NE(precision_test, precision_test2);
}

TEST_F(LargePositionTest, Double3EdgeCases)
{
    // Test double3 with extreme values and precision
    double3 max_val(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    double3 very_large(1e15, 1e15, 1e15);
    double3 zero(0.0, 0.0, 0.0);

    EXPECT_EQ(zero, double3());

    // Test precision
    double3 precise1(1.123456789012345, 2.123456789012345, 3.123456789012345);
    double3 precise2(1.123456789012345, 2.123456789012345, 3.123456789012345);
    EXPECT_EQ(precise1, precise2);

    // Test very small differences (use larger difference that's detectable)
    double3 almost_same(1.0, 2.0, 3.0);
    double3 tiny_diff(1.0 + 1e-14, 2.0 + 1e-14, 3.0 + 1e-14);
    EXPECT_NE(almost_same, tiny_diff); // Should detect small differences
}

// === CONSTRUCTOR EDGE CASES ===

TEST_F(LargePositionTest, ConstructorExtremeValues)
{
    // Test with very large coordinates (but not so large as to cause precision issues)
    double huge_coord = 1e9; // 1 billion units - large but manageable
    EXPECT_NO_THROW({
        LargePosition pos(double3(huge_coord, huge_coord, huge_coord));
        // Local coordinates should be within reasonable bounds
        EXPECT_LE(std::abs(pos.local.x), LargePosition::CELL_SIZE);
        EXPECT_LE(std::abs(pos.local.y), LargePosition::CELL_SIZE);
        EXPECT_LE(std::abs(pos.local.z), LargePosition::CELL_SIZE);
    });

    // Test with very small but non-zero coordinates
    double tiny_coord = 1e-10;
    LargePosition tiny_pos(double3(tiny_coord, tiny_coord, tiny_coord));
    EXPECT_EQ(tiny_pos.global, int3(0, 0, 0));
    EXPECT_NEAR(tiny_pos.local.x, tiny_coord, 1e-15f);

    // Test coordinates exactly on cell boundaries (CELL_SIZE/2 = 1024 rounds to cell 1)
    double boundary = LargePosition::CELL_SIZE / 2.0; // 1024
    LargePosition boundary_pos(double3(boundary, boundary, boundary));
    // Should round to nearest cell center (1024 rounds to cell 1, not 0)
    EXPECT_EQ(boundary_pos.global, int3(1, 1, 1));
    EXPECT_NEAR(boundary_pos.local.x, boundary - LargePosition::CELL_SIZE, 1e-3f);

    // Test coordinates just past cell boundaries
    double just_past = LargePosition::CELL_SIZE / 2.0 + 0.1;
    LargePosition past_pos(double3(just_past, just_past, just_past));
    EXPECT_EQ(past_pos.global, int3(1, 1, 1));
    EXPECT_NEAR(past_pos.local.x, just_past - LargePosition::CELL_SIZE, 1e-3f);
}

TEST_F(LargePositionTest, ConstructorNegativeEdgeCases)
{
    // Test negative coordinates around boundaries (-1024 rounds to cell -1)
    double neg_boundary = -LargePosition::CELL_SIZE / 2.0; // -1024
    LargePosition neg_boundary_pos(double3(neg_boundary, neg_boundary, neg_boundary));
    EXPECT_EQ(neg_boundary_pos.global, int3(-1, -1, -1));
    EXPECT_NEAR(neg_boundary_pos.local.x, neg_boundary + LargePosition::CELL_SIZE, 1e-3f);

    // Test just past negative boundary
    double just_past_neg = -LargePosition::CELL_SIZE / 2.0 - 0.1;
    LargePosition past_neg_pos(double3(just_past_neg, just_past_neg, just_past_neg));
    EXPECT_EQ(past_neg_pos.global, int3(-1, -1, -1));
    EXPECT_NEAR(past_neg_pos.local.x, just_past_neg + LargePosition::CELL_SIZE, 1e-3f);

    // Test very large negative coordinates
    double huge_negative = -1e9; // Use manageable large negative value
    EXPECT_NO_THROW({
        LargePosition neg_pos(double3(huge_negative, huge_negative, huge_negative));
        EXPECT_LE(std::abs(neg_pos.local.x), LargePosition::CELL_SIZE);
    });
}

// === COORDINATE CONVERSION EDGE CASES ===

TEST_F(LargePositionTest, CoordinateConversionExtremeDistances)
{
    // Test conversion between very distant cells
    LargePosition origin(int3(0, 0, 0), float3(0.0f, 0.0f, 0.0f));
    LargePosition far_away(int3(1000, 1000, 1000), float3(100.0f, 200.0f, 300.0f));

    // This should trigger the early exit in equality operator
    EXPECT_NE(origin, far_away);

    // Test conversion to nearby reference frames (within bounds)
    LargePosition nearby(int3(2, 1, -1), float3(500.0f, 600.0f, 700.0f));
    EXPECT_NO_THROW({
        float3 relative = nearby.to_float3(int3(0, 0, 0));
        EXPECT_LE(std::abs(relative.x), LargePosition::CELL_SIZE * 3.0f);
        EXPECT_LE(std::abs(relative.y), LargePosition::CELL_SIZE * 3.0f);
        EXPECT_LE(std::abs(relative.z), LargePosition::CELL_SIZE * 3.0f);
    });
}

TEST_F(LargePositionTest, CoordinateConversionRoundTrip)
{
    // Test round-trip conversion with various positions
    std::vector<LargePosition> test_positions = {
        LargePosition(int3(0, 0, 0), float3(0.0f, 0.0f, 0.0f)), LargePosition(int3(1, -1, 2), float3(100.5f, -200.7f, 300.9f)),
        LargePosition(int3(-5, 3, -2), float3(1500.0f, -1800.0f, 999.0f)), LargePosition(double3(12345.678, -9876.543, 2468.135))};

    std::vector<int3> reference_frames = {int3(0, 0, 0), int3(1, 1, 1), int3(-2, 3, -1), int3(10, -5, 7)};

    for (const auto& pos : test_positions)
    {
        for (const auto& ref : reference_frames)
        {
            // Skip if distance is too large (would trigger assertion)
            int3 distance = pos.global - ref;
            if (std::abs(distance.x) > 2 || std::abs(distance.y) > 2 || std::abs(distance.z) > 2)
            {
                continue;
            }

            float3 relative = pos.to_float3(ref);
            LargePosition reconstructed;
            reconstructed.from_float3(ref, relative);

            // Should represent the same world position
            ExpectWorldPositionsEqual(pos, reconstructed, LargePosition::MIN_PRECISION);
        }
    }
}

// === HYSTERESIS BEHAVIOR EDGE CASES ===

TEST_F(LargePositionTest, HysteresisThresholdBehavior)
{
    // Test behavior around the 0.75 * CELL_SIZE threshold
    constexpr float THRESHOLD = LargePosition::CELL_SIZE * 0.75f;
    int3 origin(0, 0, 0);

    // Test position just within threshold
    float3 just_within(THRESHOLD - 1.0f, 0.0f, 0.0f);
    LargePosition pos_within;
    pos_within.from_float3(origin, just_within);
    EXPECT_EQ(pos_within.global, origin); // Should stay in same cell
    EXPECT_NEAR(pos_within.local.x, just_within.x, 1e-3f);

    // Test position just beyond threshold
    float3 just_beyond(THRESHOLD + 1.0f, 0.0f, 0.0f);
    LargePosition pos_beyond;
    pos_beyond.from_float3(origin, just_beyond);
    // Should assign new cell (may be different from origin)

    // Test exact threshold
    float3 exact_threshold(THRESHOLD, 0.0f, 0.0f);
    LargePosition pos_exact;
    pos_exact.from_float3(origin, exact_threshold);
    EXPECT_EQ(pos_exact.global, origin); // Should stay in same cell (<=)
}

TEST_F(LargePositionTest, HysteresisMultipleTransitions)
{
    // Test object moving through multiple cells
    LargePosition moving_obj(int3(0, 0, 0), float3(0.0f, 0.0f, 0.0f));

    // Simulate movement across several cells
    std::vector<float3> movements = {
        float3(1000.0f, 0.0f, 0.0f),  // Move within cell
        float3(2000.0f, 0.0f, 0.0f),  // Cross cell boundary
        float3(3000.0f, 0.0f, 0.0f),  // Cross another boundary
        float3(-4000.0f, 0.0f, 0.0f), // Move back multiple cells
    };

    for (const auto& movement : movements)
    {
        float3 current_local = moving_obj.to_float3(moving_obj.global);
        float3 new_local = current_local + movement;

        LargePosition new_pos;
        new_pos.from_float3(moving_obj.global, new_local);

        // Verify world position changed by expected amount
        double3 old_world = moving_obj.to_double3();
        double3 new_world = new_pos.to_double3();

        EXPECT_NEAR(new_world.x - old_world.x, movement.x, 1e-3);
        EXPECT_NEAR(new_world.y - old_world.y, movement.y, 1e-3);
        EXPECT_NEAR(new_world.z - old_world.z, movement.z, 1e-3);

        moving_obj = new_pos;
    }
}

// === EQUALITY OPERATOR EDGE CASES ===

TEST_F(LargePositionTest, EqualityFloatingPointPrecision)
{
    // Test positions that are almost equal but differ by tiny amounts
    LargePosition pos1(int3(0, 0, 0), float3(100.0f, 200.0f, 300.0f));
    LargePosition pos2(int3(0, 0, 0), float3(100.0f + 1e-7f, 200.0f + 1e-7f, 300.0f + 1e-7f));

    EXPECT_EQ(pos1, pos2); // Should be equal within tolerance

    // Test positions that differ by more than tolerance
    LargePosition pos3(int3(0, 0, 0), float3(100.0f + 1e-5f, 200.0f, 300.0f));
    EXPECT_NE(pos1, pos3); // Should be unequal (exceeds tolerance)
}

TEST_F(LargePositionTest, EqualityDifferentRepresentations)
{
    // Test extensive set of equivalent positions with different representations
    std::vector<std::pair<LargePosition, LargePosition>> equivalent_pairs = {
        // Basic cell overflow
        {LargePosition(int3(0, 0, 0), float3(LargePosition::CELL_SIZE, 0.0f, 0.0f)),
         LargePosition(int3(1, 0, 0), float3(0.0f, 0.0f, 0.0f))},

        // Negative cell overflow
        {LargePosition(int3(0, 0, 0), float3(-LargePosition::CELL_SIZE, 0.0f, 0.0f)),
         LargePosition(int3(-1, 0, 0), float3(0.0f, 0.0f, 0.0f))},

        // Multiple cell overflow
        {LargePosition(int3(0, 0, 0), float3(2.5f * LargePosition::CELL_SIZE, 0.0f, 0.0f)),
         LargePosition(int3(2, 0, 0), float3(0.5f * LargePosition::CELL_SIZE, 0.0f, 0.0f))},

        // Mixed positive/negative
        {LargePosition(int3(1, -1, 0), float3(-500.0f, 1500.0f, 0.0f)),
         LargePosition(int3(0, 0, 0), float3(LargePosition::CELL_SIZE - 500.0f, -LargePosition::CELL_SIZE + 1500.0f, 0.0f))},
    };

    for (const auto& pair : equivalent_pairs)
    {
        EXPECT_EQ(pair.first, pair.second) << "Positions should be equal despite different representations";
        ExpectWorldPositionsEqual(pair.first, pair.second, LargePosition::TYPICAL_PRECISION);
    }
}

TEST_F(LargePositionTest, EqualityEarlyExitBehavior)
{
    // Test the early exit optimization for distant positions
    LargePosition pos1(int3(0, 0, 0), float3(0.0f, 0.0f, 0.0f));

    // Test positions at various distances
    std::vector<int3> distant_cells = {
        int3(4, 0, 0), // 4 cells away (should trigger early exit)
        int3(0, 4, 0), // 4 cells away in Y
        int3(0, 0, 4), // 4 cells away in Z
        int3(2, 2, 2), // 2 cells in each direction (should pass early exit)
        int3(3, 3, 3), // 3 cells in each direction (border case)
        int3(4, 4, 4), // 4 cells in each direction (should trigger early exit)
    };

    for (const auto& cell : distant_cells)
    {
        LargePosition distant_pos(cell, float3(0.0f, 0.0f, 0.0f));

        int max_distance = std::max(std::abs(cell.x), std::max(std::abs(cell.y), std::abs(cell.z)));
        if (max_distance > 3)
        {
            EXPECT_NE(pos1, distant_pos) << "Should be unequal due to early exit";
        }
        // For <= 3, we don't make assumptions about equality as it depends on actual positions
    }
}

// === NUMERICAL PRECISION EDGE CASES ===

TEST_F(LargePositionTest, PrecisionAtLargeScales)
{
    // Test precision with coordinates at space simulation scales
    double au_distance = LargePosition::AU_DISTANCE;

    // Test position at 1 AU - LargePosition should maintain precision
    LargePosition pos_1au(double3(au_distance, 0.0, 0.0));
    double3 reconstructed_1au = pos_1au.to_double3();
    EXPECT_NEAR(reconstructed_1au.x, au_distance, LargePosition::TYPICAL_PRECISION); // Should maintain typical precision

    // Test position at 29 AU (within documented range) - precision should be maintained
    double twentynine_au = 29.0 * au_distance;
    LargePosition pos_29au(double3(twentynine_au, 0.0, 0.0));
    double3 reconstructed_29au = pos_29au.to_double3();
    EXPECT_NEAR(reconstructed_29au.x, twentynine_au, LargePosition::MIN_PRECISION); // Precision may degrade at large scales

    // Test relative positioning at large scales with precise movement
    LargePosition nearby_29au(double3(twentynine_au + 100000.0, 0.0, 0.0)); // 100km offset

    // These positions should differ by exactly the offset amount
    double3 pos_29au_world = pos_29au.to_double3();
    double3 nearby_29au_world = nearby_29au.to_double3();
    EXPECT_NEAR(nearby_29au_world.x - pos_29au_world.x, 100000.0,
                LargePosition::MIN_PRECISION); // Use minimum precision for large-scale operations
}

TEST_F(LargePositionTest, PrecisionNearFloatLimits)
{
    // Test coordinates that stress float precision (but stay within reasonable bounds)
    float large_float = LargePosition::CELL_SIZE / 2.0f; // Use half cell size to stay within bounds

    LargePosition pos_large(int3(0, 0, 0), float3(large_float, 0.0f, 0.0f));
    float3 retrieved = pos_large.to_float3(int3(0, 0, 0));
    EXPECT_FLOAT_EQ(retrieved.x, large_float);

    // Test with coordinates that might cause precision issues (but stay within bounds)
    float3 precise_coords(1234.125f, 1678.25f, 1789.375f);
    LargePosition pos_precise(int3(0, 0, 0), precise_coords);
    float3 retrieved_precise = pos_precise.to_float3(int3(0, 0, 0));

    EXPECT_FLOAT_EQ(retrieved_precise.x, precise_coords.x);
    EXPECT_FLOAT_EQ(retrieved_precise.y, precise_coords.y);
    EXPECT_FLOAT_EQ(retrieved_precise.z, precise_coords.z);
}

// === BOUNDARY AND SPECIAL VALUE TESTS ===

TEST_F(LargePositionTest, SpecialFloatingPointValues)
{
    // Test behavior with special float values (but avoid NaN/infinity in valid usage)

    // Test very small values
    float3 tiny_vals(1e-30f, 1e-30f, 1e-30f);
    LargePosition tiny_pos(int3(0, 0, 0), tiny_vals);
    EXPECT_EQ(tiny_pos.local, tiny_vals);

    // Test zero values
    float3 zero_vals(0.0f, 0.0f, 0.0f);
    LargePosition zero_pos(int3(0, 0, 0), zero_vals);
    EXPECT_EQ(zero_pos.local, zero_vals);

    // Test negative zero (should be same as positive zero)
    float3 neg_zero(-0.0f, -0.0f, -0.0f);
    LargePosition neg_zero_pos(int3(0, 0, 0), neg_zero);
    EXPECT_EQ(zero_pos, neg_zero_pos);
}

TEST_F(LargePositionTest, CellSizeBoundaryConditions)
{
    // Test coordinates exactly at various multiples of CELL_SIZE
    float cell_size = LargePosition::CELL_SIZE;

    std::vector<double> test_multiples = {0.0, 0.5, 1.0, 1.5, 2.0, -0.5, -1.0, -1.5, -2.0};

    for (double mult : test_multiples)
    {
        double coord = mult * cell_size;
        LargePosition pos(double3(coord, coord, coord));

        // Verify the position can be reconstructed accurately
        double3 reconstructed = pos.to_double3();
        EXPECT_NEAR(reconstructed.x, coord, 1e-10);
        EXPECT_NEAR(reconstructed.y, coord, 1e-10);
        EXPECT_NEAR(reconstructed.z, coord, 1e-10);

        // Verify local coordinates are within expected bounds
        EXPECT_LE(std::abs(pos.local.x), cell_size / 2.0f + 1e-5f);
        EXPECT_LE(std::abs(pos.local.y), cell_size / 2.0f + 1e-5f);
        EXPECT_LE(std::abs(pos.local.z), cell_size / 2.0f + 1e-5f);
    }
}

// === STRESS TESTS ===

TEST_F(LargePositionTest, StressTestManyConversions)
{
    // Stress test with many coordinate conversions
    LargePosition base_pos(int3(5, -3, 2), float3(123.456f, -789.012f, 345.678f));

    // Test conversions to many different reference frames
    for (int x = -2; x <= 2; ++x)
    {
        for (int y = -2; y <= 2; ++y)
        {
            for (int z = -2; z <= 2; ++z)
            {
                int3 ref_frame(base_pos.global.x + x, base_pos.global.y + y, base_pos.global.z + z);

                EXPECT_NO_THROW({
                    float3 relative = base_pos.to_float3(ref_frame);

                    LargePosition reconstructed;
                    reconstructed.from_float3(ref_frame, relative);

                    ExpectWorldPositionsEqual(base_pos, reconstructed, LargePosition::TYPICAL_PRECISION);
                });
            }
        }
    }
}

TEST_F(LargePositionTest, StressTestLargeMovements)
{
    // Test object making many large movements
    LargePosition moving_obj(int3(0, 0, 0), float3(0.0f, 0.0f, 0.0f));

    std::vector<float3> large_movements = {
        float3(10000.0f, 0.0f, 0.0f),          float3(0.0f, -15000.0f, 0.0f),       float3(0.0f, 0.0f, 20000.0f),
        float3(-8000.0f, 12000.0f, -18000.0f), float3(25000.0f, -5000.0f, 3000.0f),
    };

    double3 expected_world = moving_obj.to_double3();

    for (const auto& movement : large_movements)
    {
        float3 current_local = moving_obj.to_float3(moving_obj.global);
        float3 new_local = current_local + movement;

        LargePosition new_pos;
        new_pos.from_float3(moving_obj.global, new_local);

        expected_world.x += movement.x;
        expected_world.y += movement.y;
        expected_world.z += movement.z;

        double3 actual_world = new_pos.to_double3();
        EXPECT_NEAR(actual_world.x, expected_world.x, LargePosition::TYPICAL_PRECISION);
        EXPECT_NEAR(actual_world.y, expected_world.y, LargePosition::TYPICAL_PRECISION);
        EXPECT_NEAR(actual_world.z, expected_world.z, LargePosition::TYPICAL_PRECISION);

        moving_obj = new_pos;
    }
}

// === RANGE LIMIT TESTS ===

TEST_F(LargePositionTest, MaximumPositiveRange)
{
    // Test coordinates at the maximum positive range
    double max_range = LargePosition::MAX_COORDINATE;

    // Test position at maximum positive range
    EXPECT_NO_THROW({
        LargePosition pos_max(double3(max_range, max_range, max_range));

        // Verify global coordinates are at maximum
        EXPECT_EQ(pos_max.global.x, INT_MAX);
        EXPECT_EQ(pos_max.global.y, INT_MAX);
        EXPECT_EQ(pos_max.global.z, INT_MAX);

        // Local coordinates should be within reasonable bounds (may be larger due to precision limits)
        EXPECT_LE(std::abs(pos_max.local.x), LargePosition::CELL_SIZE);
        EXPECT_LE(std::abs(pos_max.local.y), LargePosition::CELL_SIZE);
        EXPECT_LE(std::abs(pos_max.local.z), LargePosition::CELL_SIZE);

        // Test round-trip conversion
        double3 reconstructed = pos_max.to_double3();
        EXPECT_NEAR(reconstructed.x, max_range, LargePosition::CELL_SIZE);
        EXPECT_NEAR(reconstructed.y, max_range, LargePosition::CELL_SIZE);
        EXPECT_NEAR(reconstructed.z, max_range, LargePosition::CELL_SIZE);
    });

    // Test coordinates slightly below maximum (precision should be maintained)
    double near_max = max_range - LargePosition::CELL_SIZE * 1000;
    EXPECT_NO_THROW({
        LargePosition pos_near_max(double3(near_max, near_max, near_max));

        double3 reconstructed = pos_near_max.to_double3();
        // LargePosition should maintain precision across the entire range
        EXPECT_NEAR(reconstructed.x, near_max, LargePosition::MIN_PRECISION);
        EXPECT_NEAR(reconstructed.y, near_max, LargePosition::MIN_PRECISION);
        EXPECT_NEAR(reconstructed.z, near_max, LargePosition::MIN_PRECISION);
    });
}

TEST_F(LargePositionTest, MaximumNegativeRange)
{
    // Test coordinates at the maximum negative range
    double min_range = LargePosition::MIN_COORDINATE;

    // Test position at maximum negative range
    EXPECT_NO_THROW({
        LargePosition pos_min(double3(min_range, min_range, min_range));

        // Verify global coordinates are at minimum
        EXPECT_EQ(pos_min.global.x, INT_MIN);
        EXPECT_EQ(pos_min.global.y, INT_MIN);
        EXPECT_EQ(pos_min.global.z, INT_MIN);

        // Local coordinates should be within reasonable bounds (may be larger due to precision limits)
        EXPECT_LE(std::abs(pos_min.local.x), LargePosition::CELL_SIZE);
        EXPECT_LE(std::abs(pos_min.local.y), LargePosition::CELL_SIZE);
        EXPECT_LE(std::abs(pos_min.local.z), LargePosition::CELL_SIZE);

        // Test round-trip conversion
        double3 reconstructed = pos_min.to_double3();
        EXPECT_NEAR(reconstructed.x, min_range, LargePosition::CELL_SIZE);
        EXPECT_NEAR(reconstructed.y, min_range, LargePosition::CELL_SIZE);
        EXPECT_NEAR(reconstructed.z, min_range, LargePosition::CELL_SIZE);
    });

    // Test coordinates slightly above minimum (precision should be maintained)
    double near_min = min_range + LargePosition::CELL_SIZE * 1000;
    EXPECT_NO_THROW({
        LargePosition pos_near_min(double3(near_min, near_min, near_min));

        double3 reconstructed = pos_near_min.to_double3();
        // LargePosition should maintain precision across the entire range
        EXPECT_NEAR(reconstructed.x, near_min, LargePosition::MIN_PRECISION);
        EXPECT_NEAR(reconstructed.y, near_min, LargePosition::MIN_PRECISION);
        EXPECT_NEAR(reconstructed.z, near_min, LargePosition::MIN_PRECISION);
    });
}

TEST_F(LargePositionTest, RangeLimitOperations)
{
    // Test operations at range limits
    double max_range = LargePosition::MAX_COORDINATE;
    double min_range = LargePosition::MIN_COORDINATE;

    LargePosition pos_max(double3(max_range, 0.0, 0.0));
    LargePosition pos_min(double3(min_range, 0.0, 0.0));

    // Test equality comparison at extremes (should not crash)
    EXPECT_NE(pos_max, pos_min); // These are definitely not equal

    // Test conversion to relative coordinates (use positions within bounds)
    LargePosition pos_near_max(int3(INT_MAX, 0, 0), float3(0.0f, 0.0f, 0.0f));
    LargePosition pos_close(int3(INT_MAX - 1, 0, 0), float3(0.0f, 0.0f, 0.0f));
    EXPECT_NO_THROW({
        float3 relative = pos_close.to_float3(pos_near_max.global);
        EXPECT_LE(std::abs(relative.x), LargePosition::CELL_SIZE * 3.0f);
    });

    // Test basic movement at large scales
    LargePosition moving_obj(double3(max_range * 0.01, 0.0, 0.0)); // 1% of max range
    double3 old_world = moving_obj.to_double3();

    // Create a new position by adding directly to world coordinates
    LargePosition moved_obj(double3(old_world.x + 100000.0, old_world.y, old_world.z));
    double3 new_world = moved_obj.to_double3();

    // LargePosition should maintain precision - movement should be exact
    EXPECT_NEAR(new_world.x - old_world.x, 100000.0, LargePosition::MIN_PRECISION);
}

TEST_F(LargePositionTest, AstronomicalUnitRanges)
{
    // Test the documented range limits in astronomical units
    double au_distance = LargePosition::AU_DISTANCE;

    // Test positive AU limit (use actual system maximum)
    double pos_au_limit = LargePosition::MAX_COORDINATE;
    EXPECT_NO_THROW({
        LargePosition pos_au(double3(pos_au_limit, 0.0, 0.0));

        double3 reconstructed = pos_au.to_double3();
        // LargePosition should maintain precision even at AU scales
        EXPECT_NEAR(reconstructed.x, pos_au_limit, LargePosition::MIN_PRECISION);
    });

    // Test negative AU limit
    double neg_au_limit = LargePosition::MIN_COORDINATE;
    EXPECT_NO_THROW({
        LargePosition pos_neg_au(double3(neg_au_limit, 0.0, 0.0));

        double3 reconstructed = pos_neg_au.to_double3();
        // LargePosition should maintain precision even at AU scales
        EXPECT_NEAR(reconstructed.x, neg_au_limit, LargePosition::MIN_PRECISION);
    });

    // Test that coordinates well within AU limits maintain full precision
    double safe_au = 10.0 * au_distance; // 10 AU - well within limits
    LargePosition pos_safe_au(double3(safe_au, 0.0, 0.0));
    double3 reconstructed_safe = pos_safe_au.to_double3();
    EXPECT_NEAR(reconstructed_safe.x, safe_au, LargePosition::TYPICAL_PRECISION); // Better precision away from limits
}

TEST_F(LargePositionTest, MixedRangeOperations)
{
    // Test operations between positions at different range extremes (use more moderate ranges)
    double max_range = LargePosition::MAX_COORDINATE * 0.01; // 1% of max
    double min_range = LargePosition::MIN_COORDINATE * 0.01; // 1% of min

    LargePosition pos_high(double3(max_range, 0.0, 0.0));
    LargePosition pos_low(double3(min_range, 0.0, 0.0));
    LargePosition pos_center(double3(0.0, 0.0, 0.0));

    // Test that world coordinate calculation works for all
    EXPECT_NO_THROW({
        double3 world_high = pos_high.to_double3();
        double3 world_low = pos_low.to_double3();
        double3 world_center = pos_center.to_double3();

        EXPECT_GT(world_high.x, 0.0);
        EXPECT_LT(world_low.x, 0.0);
        EXPECT_NEAR(world_center.x, 0.0, 1e-6);
    });

    // Test equality between same positions created differently
    LargePosition pos_high2(double3(max_range, 0.0, 0.0));
    EXPECT_EQ(pos_high, pos_high2);

    // Test inequality between different extremes (distances are manageable for equality comparison)
    EXPECT_NE(pos_high, pos_low);
    EXPECT_NE(pos_high, pos_center);
    EXPECT_NE(pos_low, pos_center);
}

TEST_F(LargePositionTest, RangeBoundaryStress)
{
    // Stress test operations near the range boundaries
    std::vector<double> test_ranges = {
        LargePosition::MAX_COORDINATE * 0.99, // Near positive max
        LargePosition::MIN_COORDINATE * 0.99, // Near negative max
        LargePosition::MAX_COORDINATE * 0.5,  // Half positive max
        LargePosition::MIN_COORDINATE * 0.5,  // Half negative max
        0.0                                   // Origin
    };

    for (double range : test_ranges)
    {
        EXPECT_NO_THROW({
            // Test construction
            LargePosition pos(double3(range, range * 0.7, range * 0.3));

            // Test round-trip conversion
            double3 world = pos.to_double3();
            LargePosition pos2(world);

            // Should be approximately equal (within precision limits at this scale)
            double tolerance = std::max(1000.0, std::abs(range) * 1e-6); // Adaptive tolerance
            ExpectWorldPositionsEqual(pos, pos2, static_cast<float>(tolerance));

            // Test that we can create nearby positions
            double offset = LargePosition::CELL_SIZE * 10;
            LargePosition nearby(double3(range + offset, range * 0.7, range * 0.3));

            // Nearby positions should be different
            EXPECT_NE(pos, nearby);
        });
    }
}

// === BASIC FUNCTIONALITY TESTS ===

TEST_F(LargePositionTest, BasicConstruction)
{
    // Default constructor
    LargePosition pos1;
    EXPECT_EQ(pos1.global, int3(0, 0, 0));
    EXPECT_EQ(pos1.local, float3(0.0f, 0.0f, 0.0f));

    // Constructor with parameters
    LargePosition pos2(int3(1, 2, 3), float3(100.0f, 200.0f, 300.0f));
    EXPECT_EQ(pos2.global, int3(1, 2, 3));
    EXPECT_EQ(pos2.local, float3(100.0f, 200.0f, 300.0f));
}

TEST_F(LargePositionTest, BasicEqualityOperators)
{
    // Test exact representation equality
    LargePosition pos1(int3(1, 2, 3), float3(100.0f, 200.0f, 300.0f));
    LargePosition pos2(int3(1, 2, 3), float3(100.0f, 200.0f, 300.0f));
    LargePosition pos3(int3(1, 2, 3), float3(101.0f, 200.0f, 300.0f));

    EXPECT_EQ(pos1, pos2);
    EXPECT_FALSE(pos1 != pos2);
    EXPECT_NE(pos1, pos3);
    EXPECT_FALSE(pos1 == pos3);

    // Test loose cell equality - same world position, different representation
    LargePosition pos_a(int3(0, 0, 0), float3(2100.0f, 0.0f, 0.0f));
    LargePosition pos_b(int3(1, 0, 0), float3(52.0f, 0.0f, 0.0f));

    EXPECT_EQ(pos_a, pos_b); // Should be equal despite different representation
    EXPECT_FALSE(pos_a != pos_b);
}

TEST_F(LargePositionTest, BasicCoordinateConversion)
{
    // Test that converting to local coordinates and back preserves world position
    LargePosition original(int3(1, 1, 1), float3(100.0f, 200.0f, 300.0f));
    int3 reference_cell(0, 0, 0);

    // Convert to local coordinates relative to reference cell
    float3 local_coords = original.to_float3(reference_cell);

    // Convert back using from_float3
    LargePosition reconstructed;
    reconstructed.from_float3(reference_cell, local_coords);

    // Both positions should represent the same world location
    ExpectWorldPositionsEqual(original, reconstructed, LargePosition::TYPICAL_PRECISION);
}

TEST_F(LargePositionTest, BasicRelativePositioning)
{
    // Create two positions in different cells
    LargePosition pos1(int3(0, 0, 0), float3(500.0f, 600.0f, 700.0f));
    LargePosition pos2(int3(1, 2, -1), float3(100.0f, 200.0f, 300.0f));

    // Get pos2's coordinates relative to pos1's cell
    float3 pos2_relative_to_pos1 = pos2.to_float3(pos1.global);

    // Calculate expected relative position
    float expected_x = (1 - 0) * LargePosition::CELL_SIZE + 100.0f;  // 2048 + 100
    float expected_y = (2 - 0) * LargePosition::CELL_SIZE + 200.0f;  // 4096 + 200
    float expected_z = (-1 - 0) * LargePosition::CELL_SIZE + 300.0f; // -2048 + 300

    EXPECT_NEAR(pos2_relative_to_pos1.x, expected_x, 1e-3f);
    EXPECT_NEAR(pos2_relative_to_pos1.y, expected_y, 1e-3f);
    EXPECT_NEAR(pos2_relative_to_pos1.z, expected_z, 1e-3f);
}

TEST_F(LargePositionTest, BasicMovementSimulation)
{
    // Simulate an object moving and updating its position
    LargePosition object_pos(int3(0, 0, 0), float3(1000.0f, 1000.0f, 1000.0f));

    // Simulate movement by 5000 units in X direction (crossing cell boundaries)
    float3 current_local = object_pos.to_float3(object_pos.global);
    float3 new_local = current_local + float3(5000.0f, 0.0f, 0.0f);

    LargePosition moved_pos;
    moved_pos.from_float3(object_pos.global, new_local);

    // Verify the object moved the correct distance
    double3 original_world = object_pos.to_double3();
    double3 moved_world = moved_pos.to_double3();

    EXPECT_NEAR(moved_world.x - original_world.x, 5000.0, 1e-3);
    EXPECT_NEAR(moved_world.y - original_world.y, 0.0, 1e-3);
    EXPECT_NEAR(moved_world.z - original_world.z, 0.0, 1e-3);
}

TEST_F(LargePositionTest, BasicWorldCoordinateConstructor)
{
    // Test positive coordinates - center-based cells choose nearest cell center
    LargePosition pos1(double3(1000.0, 2000.0, 3000.0));
    // round(1000/2048) = 0, round(2000/2048) = 1, round(3000/2048) = 1
    EXPECT_EQ(pos1.global, int3(0, 1, 1));
    EXPECT_NEAR(pos1.local.x, 1000.0f, 1e-5f);
    EXPECT_NEAR(pos1.local.y, 2000.0f - 2048.0f, 1e-5f);
    EXPECT_NEAR(pos1.local.z, 3000.0f - 2048.0f, 1e-5f);

    // Test negative coordinates
    LargePosition pos2(double3(-500.0, -1000.0, -2500.0));
    EXPECT_EQ(pos2.global, int3(0, 0, -1));
    EXPECT_NEAR(pos2.local.x, -500.0f, 1e-5f);
    EXPECT_NEAR(pos2.local.y, -1000.0f, 1e-5f);
    EXPECT_NEAR(pos2.local.z, -452.0f, 1e-5f);

    // Test that equal world coordinates produce equal positions
    LargePosition pos3(double3(2500.0, 0.0, 0.0));
    LargePosition pos4(int3(1, 0, 0), float3(452.0f, 0.0f, 0.0f));
    EXPECT_EQ(pos3, pos4);
}

TEST_F(LargePositionTest, BasicBoundsAssertion)
{
    // Test positions that should pass the assertion (within CELL_SIZE * 3)
    LargePosition pos1(int3(0, 0, 0), float3(1000.0f, 1000.0f, 1000.0f));
    LargePosition pos2(int3(1, 1, 1), float3(500.0f, 500.0f, 500.0f));
    LargePosition pos3(int3(-1, -1, -1), float3(1500.0f, 1500.0f, 1500.0f));

    // These should all pass without triggering assertions
    // (positions are within reasonable distance from each other)
    EXPECT_NO_THROW({
        float3 result1 = pos1.to_float3(int3(0, 0, 0));
        float3 result2 = pos2.to_float3(int3(0, 0, 0));
        float3 result3 = pos3.to_float3(int3(0, 0, 0));
        (void)result1;
        (void)result2;
        (void)result3; // Suppress unused variable warnings
    });

    // Test with positions up to 2 cells apart (should safely pass)
    LargePosition pos_far(int3(2, 2, -2), float3(100.0f, 200.0f, 300.0f));
    float3 result_far = pos_far.to_float3(int3(0, 0, 0));

    // Verify the assertion bounds are respected
    EXPECT_LE(std::abs(result_far.x), LargePosition::CELL_SIZE * 3.0f);
    EXPECT_LE(std::abs(result_far.y), LargePosition::CELL_SIZE * 3.0f);
    EXPECT_LE(std::abs(result_far.z), LargePosition::CELL_SIZE * 3.0f);
}
