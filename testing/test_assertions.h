#ifndef TEST_ASSERTIONS_H
#define TEST_ASSERTIONS_H

#include <string>

#include "tdd_suite.h"
#include "point.h"

void expect_true(TestRunSummary& summary,
                 bool condition,
                 const std::string& suite,
                 const std::string& name,
                 const std::string& message);

void expect_near(TestRunSummary& summary,
                 double actual,
                 double expected,
                 double epsilon,
                 const std::string& suite,
                 const std::string& name,
                 const std::string& message);

Point interpolate(const Point& a, const Point& b, double t);

#endif
