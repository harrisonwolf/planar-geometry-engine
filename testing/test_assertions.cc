#include "test_assertions.h"

#include <cmath>
#include <sstream>

void expect_true(TestRunSummary& summary,
                 bool condition,
                 const std::string& suite,
                 const std::string& name,
                 const std::string& message){
    summary.assertions_run++;
    if(!condition){
        summary.assertions_failed++;
        summary.failures.push_back({suite, name, message});
    }
}

void expect_near(TestRunSummary& summary,
                 double actual,
                 double expected,
                 double epsilon,
                 const std::string& suite,
                 const std::string& name,
                 const std::string& message){
    std::ostringstream details;
    details << message << " (expected " << expected << ", got " << actual << ")";
    expect_true(summary,
                std::fabs(actual - expected) <= epsilon,
                suite,
                name,
                details.str());
}

Point interpolate(const Point& a, const Point& b, double t){
    return Point(
        a.get_x() + (b.get_x() - a.get_x()) * t,
        a.get_y() + (b.get_y() - a.get_y()) * t
    );
}
