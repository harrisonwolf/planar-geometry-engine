#include "suites.h"

#include <list>

#include "../test_assertions.h"
#include "polygon.h"

void run_polygon_geometry_suite(TestRunSummary& summary){
    const std::string suite_name = "Polygon";
    Polygon polygon(std::list<Point>{
        Point(-2.0, 0.0),
        Point(-1.0, 1.0),
        Point(1.0, 1.0),
        Point(2.0, 0.0)
    });

    expect_near(summary,
                polygon.calculate_area(),
                3.0,
                1e-6,
                suite_name,
                "calculate_area convex quadrilateral",
                "Area should match the expected trapezoid area.");

    expect_true(summary,
                polygon.contains(Point(0.0, 0.5)),
                suite_name,
                "contains interior",
                "Interior point should be contained in polygon.");
    expect_true(summary,
                polygon.contains(Point(0.0, 1.0)),
                suite_name,
                "contains boundary",
                "Boundary point should be contained in polygon.");
    expect_true(summary,
                !polygon.contains(Point(0.0, 2.0)),
                suite_name,
                "contains exterior",
                "Exterior point should not be contained in polygon.");

    expect_true(summary,
                is_convex(Point(0.0, 0.0), Point(1.0, 0.0), Point(2.0, -1.0)),
                suite_name,
                "is_convex convex turn",
                "Known convex turn should be classified as convex.");
    expect_true(summary,
                !is_convex(Point(0.0, 0.0), Point(1.0, 0.0), Point(2.0, 1.0)),
                suite_name,
                "is_convex reflex turn",
                "Known reflex turn should be classified as non-convex.");

    expect_true(summary,
                polygon.get_triangulation().size() == 2,
                suite_name,
                "triangulation count",
                "A quadrilateral should triangulate into exactly 2 triangles.");
}
