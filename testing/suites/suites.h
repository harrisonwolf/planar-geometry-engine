#ifndef TEST_SUITES_H
#define TEST_SUITES_H

#include "../tdd_suite.h"
#include "point.h"
#include "triangle.h"

void run_point_is_between_suite(TestRunSummary& summary);
void run_point_strict_is_between_suite(TestRunSummary& summary);
void run_helper_collides_suite(TestRunSummary& summary);
void run_helper_strict_collides_suite(TestRunSummary& summary);
void run_helper_is_inside_suite(TestRunSummary& summary);
void run_triangle_geometry_suite(TestRunSummary& summary);
void run_polygon_geometry_suite(TestRunSummary& summary);
void run_random_polygon_generator_suite(TestRunSummary& summary);
void run_ear_clipping_triangulation_suite(TestRunSummary& summary);

#endif
