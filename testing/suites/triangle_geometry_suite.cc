#include "suites.h"

#include "../test_assertions.h"

void run_triangle_geometry_suite(TestRunSummary& summary){
    const std::string suite_name = "Triangle";
    Triangle right_triangle(Point(0.0, 0.0), Point(4.0, 0.0), Point(0.0, 3.0));

    expect_near(summary,
                right_triangle.calculate_area(),
                6.0,
                1e-9,
                suite_name,
                "calculate_area right triangle",
                "Right-triangle area should match base*height/2.");

    expect_true(summary,
                right_triangle.contains(Point(0.75, 0.75)),
                suite_name,
                "contains interior",
                "Interior point should be contained in triangle.");
    expect_true(summary,
                right_triangle.contains(Point(2.0, 0.0)),
                suite_name,
                "contains edge",
                "Point on an edge should be contained.");
    expect_true(summary,
                !right_triangle.contains(Point(4.0, 3.0)),
                suite_name,
                "contains exterior",
                "Exterior point should not be contained.");

    Triangle translated = right_triangle;
    translated.transpose(5.0, -2.0);
    expect_near(summary,
                translated.get_area(),
                6.0,
                1e-9,
                suite_name,
                "transpose preserves area",
                "Transposing a triangle should not change area.");
}
