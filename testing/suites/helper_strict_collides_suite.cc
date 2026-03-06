#include "suites.h"

#include "../test_assertions.h"
#include "../../helper.h"

void run_helper_strict_collides_suite(TestRunSummary& summary){
    const std::string suite_name = "helper::strict_collides";

    Point a1(0.0, 0.0);
    Point a2(4.0, 4.0);
    Point b1(0.0, 4.0);
    Point b2(4.0, 0.0);
    expect_true(summary,
                strict_collides({a1, a2}, {b1, b2}),
                suite_name,
                "interior intersection",
                "Segments with a shared interior intersection should strictly collide.");

    Point c1(0.0, 0.0);
    Point c2(2.0, 2.0);
    Point d1(2.0, 2.0);
    Point d2(3.0, 1.0);
    expect_true(summary,
                !strict_collides({c1, c2}, {d1, d2}),
                suite_name,
                "shared endpoint",
                "Segments touching only at endpoints should not strictly collide.");

    Point e1(0.0, 0.0);
    Point e2(1.0, 0.0);
    Point f1(2.0, -1.0);
    Point f2(3.0, 0.0);
    expect_true(summary,
                !strict_collides({e1, e2}, {f1, f2}),
                suite_name,
                "disjoint segments",
                "Non-intersecting segments should not strictly collide.");
}
