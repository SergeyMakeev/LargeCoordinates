#include "LargeCoordinates.h"
#include <cassert>
#include <iostream>
#include <cmath>

void test_basic_construction() {
    std::cout << "Testing basic construction..." << std::endl;
    
    // Default constructor
    LargePosition pos1;
    assert(pos1.global == int3(0, 0, 0));
    assert(pos1.local == float3(0.0f, 0.0f, 0.0f));
    
    // Constructor with parameters
    LargePosition pos2(int3(1, 2, 3), float3(100.0f, 200.0f, 300.0f));
    assert(pos2.global == int3(1, 2, 3));
    assert(pos2.local == float3(100.0f, 200.0f, 300.0f));
    
    std::cout << "PASS: Basic construction tests passed" << std::endl;
}

void test_equality_operators() {
    std::cout << "Testing equality operators..." << std::endl;
    
    // Test exact representation equality
    LargePosition pos1(int3(1, 2, 3), float3(100.0f, 200.0f, 300.0f));
    LargePosition pos2(int3(1, 2, 3), float3(100.0f, 200.0f, 300.0f));
    LargePosition pos3(int3(1, 2, 3), float3(101.0f, 200.0f, 300.0f));
    
    assert(pos1 == pos2);
    assert(!(pos1 != pos2));
    assert(pos1 != pos3);
    assert(!(pos1 == pos3));
    
    // Test loose cell equality - same world position, different representation
    // Position A: global=(0,0,0), local=(2100, 0, 0)
    // Position B: global=(1,0,0), local=(52, 0, 0) [2100 - 2048 = 52]
    // These represent the same world position
    LargePosition pos_a(int3(0, 0, 0), float3(2100.0f, 0.0f, 0.0f));
    LargePosition pos_b(int3(1, 0, 0), float3(52.0f, 0.0f, 0.0f));
    
    assert(pos_a == pos_b);  // Should be equal despite different representation
    assert(!(pos_a != pos_b));
    
    // Test with negative coordinates - smaller values to stay within bounds
    LargePosition pos_c(int3(0, 0, 0), float3(-200.0f, 0.0f, 0.0f));
    LargePosition pos_d(int3(-1, 0, 0), float3(1848.0f, 0.0f, 0.0f)); // -200 + 2048 = 1848
    
    assert(pos_c == pos_d);  // Should be equal despite different representation
    
    // Test early exit for positions that are too far apart
    LargePosition pos_far1(int3(0, 0, 0), float3(100.0f, 0.0f, 0.0f));
    LargePosition pos_far2(int3(5, 0, 0), float3(100.0f, 0.0f, 0.0f)); // 5 cells apart
    
    assert(pos_far1 != pos_far2);  // Should be unequal due to distance
    
    std::cout << "PASS: Equality operator tests passed" << std::endl;
}

void test_coordinate_conversion_accuracy() {
    std::cout << "Testing coordinate conversion accuracy..." << std::endl;
    
    // Test that converting to local coordinates and back preserves world position
    LargePosition original(int3(1, 1, 1), float3(100.0f, 200.0f, 300.0f));
    int3 reference_cell(0, 0, 0);
    
    // Convert to local coordinates relative to reference cell
    float3 local_coords = original.get_local(reference_cell);
    
    // Convert back using set_from_local
    LargePosition reconstructed;
    reconstructed.set_from_local(reference_cell, local_coords);
    
    // Both positions should represent the same world location
    // Test by converting both to the same reference frame
    float3 original_world = original.get_local(int3(0, 0, 0));
    float3 reconstructed_world = reconstructed.get_local(int3(0, 0, 0));
    
    // Use reasonable tolerance for floating point precision
    float tolerance = 1.0f;  // More lenient tolerance for round-trip conversion
    assert(std::abs(original_world.x - reconstructed_world.x) < tolerance);
    assert(std::abs(original_world.y - reconstructed_world.y) < tolerance);
    assert(std::abs(original_world.z - reconstructed_world.z) < tolerance);
    
    std::cout << "PASS: Coordinate conversion accuracy tests passed" << std::endl;
}

void test_relative_positioning() {
    std::cout << "Testing relative positioning..." << std::endl;
    
    // Create two positions in different cells
    LargePosition pos1(int3(0, 0, 0), float3(500.0f, 600.0f, 700.0f));
    LargePosition pos2(int3(1, 2, -1), float3(100.0f, 200.0f, 300.0f));
    
    // Get pos2's coordinates relative to pos1's cell
    float3 pos2_relative_to_pos1 = pos2.get_local(pos1.global);
    
    // Calculate expected relative position
    float expected_x = (1 - 0) * LargePosition::CELL_SIZE + 100.0f;  // 2048 + 100
    float expected_y = (2 - 0) * LargePosition::CELL_SIZE + 200.0f;  // 4096 + 200
    float expected_z = (-1 - 0) * LargePosition::CELL_SIZE + 300.0f; // -2048 + 300
    
    assert(std::abs(pos2_relative_to_pos1.x - expected_x) < 1e-3f);
    assert(std::abs(pos2_relative_to_pos1.y - expected_y) < 1e-3f);
    assert(std::abs(pos2_relative_to_pos1.z - expected_z) < 1e-3f);
    
    std::cout << "PASS: Relative positioning tests passed" << std::endl;
}



void test_movement_simulation() {
    std::cout << "Testing movement simulation..." << std::endl;
    
    // Simulate an object moving and updating its position
    LargePosition object_pos(int3(0, 0, 0), float3(1000.0f, 1000.0f, 1000.0f));
    
    // Simulate movement by 5000 units in X direction (crossing cell boundaries)
    float3 current_local = object_pos.get_local(object_pos.global);
    float3 new_local = current_local + float3(5000.0f, 0.0f, 0.0f);
    
    LargePosition moved_pos;
    moved_pos.set_from_local(object_pos.global, new_local);
    
    // Verify the object moved the correct distance
    float3 original_world = object_pos.get_local(int3(0, 0, 0));
    float3 moved_world = moved_pos.get_local(int3(0, 0, 0));
    
    float distance_moved = moved_world.x - original_world.x;
    assert(std::abs(distance_moved - 5000.0f) < 1e-3f);
    
    // Y and Z should remain unchanged
    assert(std::abs(moved_world.y - original_world.y) < 1e-3f);
    assert(std::abs(moved_world.z - original_world.z) < 1e-3f);
    
    std::cout << "PASS: Movement simulation tests passed" << std::endl;
}

void test_get_local_bounds() {
    std::cout << "Testing get_local() bounds assertion..." << std::endl;
    
    // Test positions that should pass the assertion (within CELL_SIZE * 3)
    LargePosition pos1(int3(0, 0, 0), float3(1000.0f, 1000.0f, 1000.0f));
    LargePosition pos2(int3(1, 1, 1), float3(500.0f, 500.0f, 500.0f));
    LargePosition pos3(int3(-1, -1, -1), float3(1500.0f, 1500.0f, 1500.0f));
    
    // These should all pass without triggering assertions
    // (positions are within reasonable distance from each other)
    float3 result1 = pos1.get_local(int3(0, 0, 0));
    float3 result2 = pos2.get_local(int3(0, 0, 0));
    float3 result3 = pos3.get_local(int3(0, 0, 0));
    
    // Test with positions up to 2 cells apart (should safely pass)
    LargePosition pos_far(int3(2, 2, -2), float3(100.0f, 200.0f, 300.0f));
    float3 result_far = pos_far.get_local(int3(0, 0, 0));
    
    // Verify the assertion bounds are respected
    assert(std::abs(result_far.x) <= LargePosition::CELL_SIZE * 3.0f);
    assert(std::abs(result_far.y) <= LargePosition::CELL_SIZE * 3.0f);
    assert(std::abs(result_far.z) <= LargePosition::CELL_SIZE * 3.0f);
    
    std::cout << "PASS: get_local() bounds assertion tests passed" << std::endl;
}

void test_world_coordinate_constructor() {
    std::cout << "Testing world coordinate constructor..." << std::endl;
    
    // Test positive coordinates - center-based cells choose nearest cell center
    LargePosition pos1(1000.0, 2000.0, 3000.0);
    // round(1000/2048) = 0, round(2000/2048) = 1, round(3000/2048) = 1
    assert(pos1.global == int3(0, 1, 1)); 
    assert(std::abs(pos1.local.x - 1000.0f) < 1e-5f);        // 1000 - 0*2048 = 1000
    assert(std::abs(pos1.local.y - (2000.0f - 2048.0f)) < 1e-5f); // 2000 - 1*2048 = -48
    assert(std::abs(pos1.local.z - (3000.0f - 2048.0f)) < 1e-5f); // 3000 - 1*2048 = 952
    
    // Test coordinates that cross cell boundaries
    LargePosition pos2(2500.0, 0.0, 0.0);
    // round(2500/2048) = round(1.22) = 1, round(0/2048) = 0
    assert(pos2.global == int3(1, 0, 0)); 
    assert(std::abs(pos2.local.x - (2500.0f - 2048.0f)) < 1e-5f); // 2500 - 1*2048 = 452
    assert(std::abs(pos2.local.y - 0.0f) < 1e-5f);               // 0 - 0*2048 = 0
    assert(std::abs(pos2.local.z - 0.0f) < 1e-5f);               // 0 - 0*2048 = 0
    
    // Test negative coordinates
    LargePosition pos3(-500.0, -1000.0, -2500.0);
    // round(-500/2048) = 0, round(-1000/2048) = 0, round(-2500/2048) = -1
    assert(pos3.global == int3(0, 0, -1)); 
    assert(std::abs(pos3.local.x - (-500.0f)) < 1e-5f);   // -500 - 0*2048 = -500
    assert(std::abs(pos3.local.y - (-1000.0f)) < 1e-5f);  // -1000 - 0*2048 = -1000
    assert(std::abs(pos3.local.z - (-452.0f)) < 1e-5f);   // -2500 - (-1)*2048 = -452
    
    // Test large coordinates (space sim scale) - just verify it doesn't crash
    double large_coord = 1e10; // 10 billion units
    LargePosition pos4(large_coord, 0.0, 0.0);
    // Should not crash and should have reasonable global/local values
    // With center-based cells, local coordinates can be +/-CELL_SIZE/2 (or extended to +/-CELL_SIZE with hysteresis)
    assert(std::abs(pos4.local.x) <= LargePosition::CELL_SIZE);


    double cellSize = double(LargePosition::CELL_SIZE);
    double offset = cellSize * 10000.0;
    
    // Verify world position 
    LargePosition pos5(offset + 1000.0, offset + 500.0, offset + 1500.0);
    int3 origin = int3(10000, 10000, 10000);
    float3 reconstructed = pos5.get_local(origin);
    assert(std::abs(reconstructed.x - 1000.0f) < 1e-3f);
    assert(std::abs(reconstructed.y - 500.0f) < 1e-3f);
    assert(std::abs(reconstructed.z - 1500.0f) < 1e-3f);
    
    // Test that equal world coordinates produce equal positions
    LargePosition pos6(2500.0, 0.0, 0.0); // Should be global=(1,0,0), local=(452,0,0) with center-based cells
    LargePosition pos7(int3(1, 0, 0), float3(452.0f, 0.0f, 0.0f)); // Manual construction
    assert(pos6 == pos7); // Should be equal
    
    std::cout << "PASS: World coordinate constructor tests passed" << std::endl;
}

int main() {
    std::cout << "Running LargePosition unit tests..." << std::endl;
    std::cout << "======================================" << std::endl;
    
    try {
        test_basic_construction();
        test_equality_operators();
        test_coordinate_conversion_accuracy();
        test_relative_positioning();
        test_movement_simulation();
        test_get_local_bounds();
        test_world_coordinate_constructor();
        
        std::cout << "======================================" << std::endl;
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cout << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "Test failed with unknown exception" << std::endl;
        return 1;
    }
} 