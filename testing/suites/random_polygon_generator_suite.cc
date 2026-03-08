#include "suites.h"

#include <cmath>
#include <list>
#include <vector>

#include "../test_assertions.h"
#include "random_polygon_generator.h"

namespace {

double orient2d(const Point& a, const Point& b, const Point& c){
    return (b.get_x() - a.get_x()) * (c.get_y() - a.get_y()) -
           (b.get_y() - a.get_y()) * (c.get_x() - a.get_x());
}

bool on_segment(const Point& a, const Point& b, const Point& p){
    const double eps = 1e-9;
    if(std::fabs(orient2d(a, b, p)) > eps) return false;
    double min_x = std::min(a.get_x(), b.get_x()) - eps;
    double max_x = std::max(a.get_x(), b.get_x()) + eps;
    double min_y = std::min(a.get_y(), b.get_y()) - eps;
    double max_y = std::max(a.get_y(), b.get_y()) + eps;
    return p.get_x() >= min_x && p.get_x() <= max_x && p.get_y() >= min_y && p.get_y() <= max_y;
}

bool segments_intersect(const Point& p1, const Point& p2, const Point& q1, const Point& q2){
    const double eps = 1e-9;
    double o1 = orient2d(p1, p2, q1);
    double o2 = orient2d(p1, p2, q2);
    double o3 = orient2d(q1, q2, p1);
    double o4 = orient2d(q1, q2, p2);

    if(((o1 > eps && o2 < -eps) || (o1 < -eps && o2 > eps)) &&
       ((o3 > eps && o4 < -eps) || (o3 < -eps && o4 > eps))){
        return true;
    }

    if(std::fabs(o1) <= eps && on_segment(p1, p2, q1)) return true;
    if(std::fabs(o2) <= eps && on_segment(p1, p2, q2)) return true;
    if(std::fabs(o3) <= eps && on_segment(q1, q2, p1)) return true;
    if(std::fabs(o4) <= eps && on_segment(q1, q2, p2)) return true;
    return false;
}

bool is_simple_polygon(const std::vector<Point>& vertices){
    int n = static_cast<int>(vertices.size());
    if(n < 3) return false;

    for(int i = 0; i < n; ++i){
        Point a1 = vertices.at(i);
        Point a2 = vertices.at((i + 1) % n);

        for(int j = i + 1; j < n; ++j){
            if(j == i || (i + 1) % n == j || (j + 1) % n == i) continue;

            Point b1 = vertices.at(j);
            Point b2 = vertices.at((j + 1) % n);
            if(segments_intersect(a1, a2, b1, b2)) return false;
        }
    }

    return true;
}

} // namespace

void run_random_polygon_generator_suite(TestRunSummary& summary){
    const std::string suite_name = "random_polygon_generator";

    for(int i = 0; i < 30; ++i){
        const int vertex_count = 8;
        Polygon polygon = generate_random_polygon(vertex_count, 40.0);
        std::list<Point> vertex_list = polygon.get_vertex_list();
        std::vector<Point> vertices(vertex_list.begin(), vertex_list.end());

        expect_true(summary,
                    static_cast<int>(vertices.size()) == vertex_count,
                    suite_name,
                    "vertex count",
                    "Generated polygon should have exactly requested number of vertices.");

        bool all_unique = true;
        for(size_t j = 0; j < vertices.size(); ++j){
            for(size_t k = j + 1; k < vertices.size(); ++k){
                if(vertices.at(j) == vertices.at(k)){
                    all_unique = false;
                    break;
                }
            }
            if(!all_unique) break;
        }
        expect_true(summary,
                    all_unique,
                    suite_name,
                    "unique vertices",
                    "Generated polygon should not repeat vertices.");

        expect_true(summary,
                    is_simple_polygon(vertices),
                    suite_name,
                    "simple polygon",
                    "Generated polygon should have no self-intersecting edges.");

        expect_true(summary,
                    polygon.get_area() > 1e-9,
                    suite_name,
                    "positive area",
                    "Generated polygon should have strictly positive area.");

        expect_true(summary,
                    polygon.get_triangulation().size() == vertices.size() - 2,
                    suite_name,
                    "triangulation cardinality",
                    "Any simple n-gon should triangulate to n-2 triangles.");
    }
}
