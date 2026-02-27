#include "tdd_suite.h"

#include <cmath>
#include <random>
#include <sstream>
#include <utility>

#include "../helper.h"
#include "../point.h"

namespace {

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

Point interpolate(const Point& a, const Point& b, double t){
    return Point(
        a.get_x() + (b.get_x() - a.get_x()) * t,
        a.get_y() + (b.get_y() - a.get_y()) * t
    );
}

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

        // 1) Points generated directly on the segment should pass.
        Point on_segment = interpolate(p1, p2, t_inside_dist(rng));
        expect_true(summary,
                    on_segment.is_between(p1, p2),
                    suite_name,
                    "on-segment interpolation",
                    "Interpolated point was not recognized as between endpoints.");

        // 2) Collinear points outside segment bounds should fail.
        double t_outside = outside_choice(rng) < 0.5
            ? -t_inside_dist(rng) - 0.1
            : 1.1 + t_inside_dist(rng);
        Point outside_collinear = interpolate(p1, p2, t_outside);
        expect_true(summary,
                    !outside_collinear.is_between(p1, p2),
                    suite_name,
                    "collinear outside bounds",
                    "Collinear point outside segment bounds was marked between.");

        // 3) Non-collinear points should fail.
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

    // Explicit endpoint checks (inclusive semantics).
    Point a(-3.25, 7.5);
    Point b(8.75, -4.5);
    expect_true(summary, a.is_between(a, b), suite_name, "left endpoint", "Endpoint should be between.");
    expect_true(summary, b.is_between(a, b), suite_name, "right endpoint", "Endpoint should be between.");
}

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

    // Non-collision case (lines intersect, but outside first segment bounds).
    Point a1(0.0, 0.0);
    Point a2(1.0, 0.0);
    Point b1(2.0, -1.0);
    Point b2(3.0, 0.0);
    expect_true(summary,
                !collides({a1, a2}, {b1, b2}),
                suite_name,
                "line intersection outside segment",
                "Segments whose infinite lines intersect outside segment bounds should not collide.");

    // Endpoint-touching case should count as collision.
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

} // namespace

TestRunSummary run_geometry_tdd_suite(){
    TestRunSummary summary;
    run_point_is_between_suite(summary);
    run_helper_collides_suite(summary);
    return summary;
}
