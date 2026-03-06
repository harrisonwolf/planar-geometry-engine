#include "suites.h"

#include "../test_assertions.h"
#include "helper.h"

void run_helper_is_inside_suite(TestRunSummary& summary){
    const std::string suite_name = "helper::is_inside";
    Point a(0.0, 0.0);
    Point b(6.0, 0.0);
    Point c(0.0, 6.0);
    Triangle t(a, b, c);

    Point inside(1.0, 1.0);
    Point edge(3.0, 0.0);
    Point outside(4.0, 4.0);

    expect_true(summary,
                is_inside(inside, t),
                suite_name,
                "triangle overload interior",
                "Point strictly inside triangle should be inside.");
    expect_true(summary,
                is_inside(edge, t),
                suite_name,
                "triangle overload edge",
                "Point on triangle edge should be treated as inside.");
    expect_true(summary,
                !is_inside(outside, t),
                suite_name,
                "triangle overload exterior",
                "Point outside triangle should be outside.");

    expect_true(summary,
                is_inside(inside, a, b, c),
                suite_name,
                "vertex overload interior",
                "Point strictly inside triangle should be inside (vertex overload).");
    expect_true(summary,
                is_inside(edge, a, b, c),
                suite_name,
                "vertex overload edge",
                "Point on triangle edge should be inside (vertex overload).");
    expect_true(summary,
                !is_inside(outside, a, b, c),
                suite_name,
                "vertex overload exterior",
                "Point outside triangle should be outside (vertex overload).");
}
