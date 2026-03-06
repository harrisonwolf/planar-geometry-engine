#include "suites.h"

#include <list>
#include <vector>

#include "../test_assertions.h"
#include "../../ear_clipping_triangulation.h"

namespace {

bool same_point(const Point& a, const Point& b){
    return a == b;
}

bool same_triangle(Triangle t, Triangle expected){
    return same_point(t.get_a(), expected.get_a()) &&
           same_point(t.get_b(), expected.get_b()) &&
           same_point(t.get_c(), expected.get_c());
}

} // namespace

void run_ear_clipping_triangulation_suite(TestRunSummary& summary){
    const std::string suite_name = "ear_clipping_triangulation";

    Polygon polygon(std::list<Point>{
        Point(-3, 0),
        Point(-1, 2),
        Point(1, 1),
        Point(3, 2),
        Point(4, 0),
        Point(0, -2)
    });

    std::vector<Triangle> triangulation = triangulate(polygon);
    std::vector<Triangle> expected{
        Triangle(Point(0, -2), Point(-3, 0), Point(-1, 2)),
        Triangle(Point(0, -2), Point(-1, 2), Point(1, 1)),
        Triangle(Point(0, -2), Point(1, 1), Point(3, 2)),
        Triangle(Point(3, 2), Point(4, 0), Point(0, -2))
    };

    expect_true(summary,
                triangulation.size() == expected.size(),
                suite_name,
                "triangle count",
                "Ear clipping should generate n-2 triangles.");

    bool identical = triangulation.size() == expected.size();
    for(size_t i = 0; i < triangulation.size() && i < expected.size(); ++i){
        if(!same_triangle(triangulation.at(i), expected.at(i))){
            identical = false;
            break;
        }
    }
    expect_true(summary,
                identical,
                suite_name,
                "deterministic ear clipping order",
                "Triangulation output did not match the expected ear clipping triangle sequence.");
}
