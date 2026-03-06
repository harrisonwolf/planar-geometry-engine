#include "suites.h"

#include <cmath>
#include <random>

#include "../test_assertions.h"

void run_point_is_between_suite(TestRunSummary& summary){
    const std::string suite_name = "Point::is_between";
    std::mt19937 rng(1337);
    std::uniform_real_distribution<double> coord_dist(-50.0, 50.0);
    std::uniform_real_distribution<double> t_inside_dist(0.0, 1.0);
    std::uniform_real_distribution<double> outside_choice(0.0, 1.0);
    std::uniform_real_distribution<double> offset_dist(0.5, 10.0);

    for(int i=0; i<200; ++i){
        Point p1(coord_dist(rng), coord_dist(rng));
        Point p2(coord_dist(rng), coord_dist(rng));

        if(p1 == p2){
            p2 = Point(p2.get_x() + 1.0, p2.get_y() + 1.0);
        }

        Point on_segment = interpolate(p1, p2, t_inside_dist(rng));
        expect_true(summary,
                    on_segment.is_between(p1, p2),
                    suite_name,
                    "on-segment interpolation",
                    "Interpolated point was not recognized as between endpoints.");

        double t_outside = outside_choice(rng) < 0.5
            ? -t_inside_dist(rng) - 0.1
            : 1.1 + t_inside_dist(rng);
        Point outside_collinear = interpolate(p1, p2, t_outside);
        expect_true(summary,
                    !outside_collinear.is_between(p1, p2),
                    suite_name,
                    "collinear outside bounds",
                    "Collinear point outside segment bounds was marked between.");

        Point direction = p2 - p1;
        Point normal(-direction.get_y(), direction.get_x());
        double normal_mag = std::sqrt(normal.get_x() * normal.get_x() + normal.get_y() * normal.get_y());
        if(normal_mag == 0.0){
            continue;
        }
        double scale = offset_dist(rng) / normal_mag;
        Point midpoint = interpolate(p1, p2, 0.5);
        Point off_segment(midpoint.get_x() + normal.get_x() * scale,
                          midpoint.get_y() + normal.get_y() * scale);

        expect_true(summary,
                    !off_segment.is_between(p1, p2),
                    suite_name,
                    "non-collinear midpoint offset",
                    "Non-collinear point was marked between.");
    }

    Point a(-3.25, 7.5);
    Point b(8.75, -4.5);
    expect_true(summary, a.is_between(a, b), suite_name, "left endpoint", "Endpoint should be between.");
    expect_true(summary, b.is_between(a, b), suite_name, "right endpoint", "Endpoint should be between.");
}
