#include "suites.h"

#include "../test_assertions.h"

void run_point_strict_is_between_suite(TestRunSummary& summary){
    const std::string suite_name = "Point::strict_is_between";

    Point a(0.0, 0.0);
    Point b(10.0, 10.0);

    expect_true(summary,
                Point(5.0, 5.0).strict_is_between(a, b),
                suite_name,
                "strict interior",
                "Interior collinear point should be strictly between endpoints.");

    expect_true(summary,
                !a.strict_is_between(a, b),
                suite_name,
                "left endpoint excluded",
                "Left endpoint should not be strictly between.");

    expect_true(summary,
                !b.strict_is_between(a, b),
                suite_name,
                "right endpoint excluded",
                "Right endpoint should not be strictly between.");

    expect_true(summary,
                !Point(12.0, 12.0).strict_is_between(a, b),
                suite_name,
                "outside collinear",
                "Collinear point outside segment bounds should not be strictly between.");

    expect_true(summary,
                !Point(5.0, 4.5).strict_is_between(a, b),
                suite_name,
                "non-collinear",
                "Non-collinear point should not be strictly between.");
}
