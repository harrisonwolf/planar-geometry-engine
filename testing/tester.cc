#include <iostream>

#include "tdd_suite.h"

int main(){
    TestRunSummary summary = run_geometry_tdd_suite();

    std::cout << "Ran " << summary.assertions_run << " assertions.\n";

    if(summary.assertions_failed == 0){
        std::cout << "All tests passed.\n";
        return 0;
    }

    std::cout << summary.assertions_failed << " assertions failed.\n";
    for(const TestFailure& failure: summary.failures){
        std::cout << "[" << failure.suite << "] " << failure.name
                  << ": " << failure.message << "\n";
    }

    return 1;
}
