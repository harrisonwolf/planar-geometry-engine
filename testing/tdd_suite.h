#ifndef TDD_SUITE_H
#define TDD_SUITE_H

#include <string>
#include <vector>

struct TestFailure {
    std::string suite;
    std::string name;
    std::string message;
};

struct TestRunSummary {
    int assertions_run = 0;
    int assertions_failed = 0;
    std::vector<TestFailure> failures;
};

TestRunSummary run_geometry_tdd_suite();

#endif
