#include "suites.h"

#include <cmath>
#include <random>

#include "../test_assertions.h"
#include "helper.h"

void run_helper_collides_suite(TestRunSummary& summary){
    const std::string suite_name = "helper::collides";
    std::mt19937 rng(4242);
    std::uniform_real_distribution<double> coord_dist(-20.0, 20.0);
    std::uniform_real_distribution<double> dir_dist(-3.0, 3.0);
    std::uniform_real_distribution<double> half_len_dist(0.5, 4.0);

    for(int i=0; i<200; ++i){
        Point intersection(coord_dist(rng), coord_dist(rng));

        Point d1(dir_dist(rng), dir_dist(rng));
        Point d2(dir_dist(rng), dir_dist(rng));

        if(std::fabs(cross(d1, d2)) < 0.25){
            d2 = Point(d2.get_x() + 2.0, d2.get_y() - 1.5);
        }

        if(std::fabs(d1.get_x()) < 0.15){
            d1 = Point(d1.get_x() + 0.5, d1.get_y());
        }
        if(std::fabs(d2.get_x()) < 0.15){
            d2 = Point(d2.get_x() - 0.5, d2.get_y());
        }

        double t1 = half_len_dist(rng);
        double t2 = half_len_dist(rng);

        Point s1a(intersection.get_x() - d1.get_x() * t1, intersection.get_y() - d1.get_y() * t1);
        Point s1b(intersection.get_x() + d1.get_x() * t1, intersection.get_y() + d1.get_y() * t1);
        Point s2a(intersection.get_x() - d2.get_x() * t2, intersection.get_y() - d2.get_y() * t2);
        Point s2b(intersection.get_x() + d2.get_x() * t2, intersection.get_y() + d2.get_y() * t2);

        expect_true(summary,
                    collides({s1a, s1b}, {s2a, s2b}),
                    suite_name,
                    "interior intersection",
                    "Segments sharing interior intersection were marked as not colliding.");
    }

    Point a1(0.0, 0.0);
    Point a2(1.0, 0.0);
    Point b1(2.0, -1.0);
    Point b2(3.0, 0.0);
    expect_true(summary,
                !collides({a1, a2}, {b1, b2}),
                suite_name,
                "line intersection outside segment",
                "Segments whose infinite lines intersect outside segment bounds should not collide.");

    Point c1(0.0, 0.0);
    Point c2(2.0, 2.0);
    Point d1(2.0, 2.0);
    Point d2(4.0, 1.0);
    expect_true(summary,
                collides({c1, c2}, {d1, d2}),
                suite_name,
                "shared endpoint",
                "Segments sharing an endpoint should collide.");
}
