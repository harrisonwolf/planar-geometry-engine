#include "suites.h"

#include <iostream>
#include <list>
#include <sstream>

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

    std::ostringstream desmos_output;
    std::streambuf* original_output = std::cout.rdbuf(desmos_output.rdbuf());
    polygon.print_triangulation_desmos();
    std::cout.rdbuf(original_output);
    const std::string rendered = desmos_output.str();
    size_t expression_count = 0;
    size_t position = 0;
    while((position = rendered.find("polygon(", position)) != std::string::npos){
        ++expression_count;
        position += 8;
    }
    expect_true(summary,
                expression_count == polygon.get_triangulation().size(),
                suite_name,
                "Desmos triangulation output count",
                "The Desmos printer should emit one polygon expression per triangle.");
    expect_true(summary,
                rendered.find("FIXME") == std::string::npos,
                suite_name,
                "Desmos triangulation has no placeholder",
                "The supported formatter should never emit the historical placeholder.");
}
