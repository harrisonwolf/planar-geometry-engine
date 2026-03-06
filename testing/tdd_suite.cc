#include "tdd_suite.h"

#include "suites/suites.h"

TestRunSummary run_geometry_tdd_suite(){
    TestRunSummary summary;
    run_point_is_between_suite(summary);
    run_point_strict_is_between_suite(summary);
    run_helper_collides_suite(summary);
    run_helper_strict_collides_suite(summary);
    run_helper_is_inside_suite(summary);
    run_triangle_geometry_suite(summary);
    run_polygon_geometry_suite(summary);
    run_random_polygon_generator_suite(summary);
    run_ear_clipping_triangulation_suite(summary);
    return summary;
}
