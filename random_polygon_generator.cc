//implementation file for random_polygon_generator.h
#include "random_polygon_generator.h"
#include "logger.h"
#include <random>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

namespace {

constexpr double kDefaultBound = 100.0;
constexpr double kEpsilonX = 1e-6;

Polygon generate_random_polygon_with_bounds(int n, double min_coord, double max_coord){
	DBG("Entered generate_random_polygon function\n");
	if(n < 3){
		cout << "Error: Called generate_random_polygon with n less than 3. Exiting.\n";
		exit(1);
	}
	if(min_coord >= max_coord){
		cout << "Error: invalid coordinate bounds for random polygon generator.\n";
		exit(1);
	}

	random_device rd;
	mt19937 gen(rd());

	double coord_span = max_coord - min_coord;
	double center_x = (min_coord + max_coord) / 2.0;
	double center_y = (min_coord + max_coord) / 2.0;
	double max_radius = 0.45 * coord_span;
	double min_radius = 0.10 * coord_span;

	const double two_pi = 2.0 * acos(-1.0);
	uniform_real_distribution<double> angle_dist(0.0, two_pi);
	uniform_real_distribution<double> radius_dist(min_radius, max_radius);

	vector<double> angles;
	angles.reserve(n);
	for(int i = 0; i < n; ++i){
		angles.push_back(angle_dist(gen));
	}
	sort(angles.begin(), angles.end());

	vector<Point> generated;
	generated.reserve(n);
	for(int i = 0; i < n; ++i){
		double r = radius_dist(gen);
		double x = center_x + r * cos(angles[i]);
		double y = center_y + r * sin(angles[i]);

		// avoid identical x-values to keep line constructors happy
		for(size_t j = 0; j < generated.size(); ++j){
			if(fabs(generated[j].get_x() - x) < kEpsilonX){
				x += (j + 1) * kEpsilonX;
			}
		}

		generated.emplace_back(x, y);
	}

	list<Point> points(generated.begin(), generated.end());
	DBG_TAG("poly.rand_gen", ANSI::BOLD_GREEN << "Successfully finished gen function!\n" << ANSI::RESET);
	return Polygon(points);
}

} // namespace

Polygon generate_random_polygon(){
	uniform_int_distribution<int> v_dist(4, 99);
	random_device rd;
	mt19937 gen(rd());
	return generate_random_polygon(v_dist(gen));
}

Polygon generate_random_polygon(int n){
	return generate_random_polygon_with_bounds(n, -kDefaultBound, kDefaultBound);
}

Polygon generate_random_polygon(int n, double c){
	if(c <= 0){
		cout << "Error: coordinate bound must be positive.\n";
		exit(1);
	}
	return generate_random_polygon_with_bounds(n, -c, c);
}

Polygon generate_random_polygon(int n, double c1, double c2){
	return generate_random_polygon_with_bounds(n, c1, c2);
}
