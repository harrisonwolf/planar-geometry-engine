// Implementation of the terrain analysis routines declared in terrain.h.

#include "terrain.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <set>
#include <utility>
#include <vector>

using namespace std;

namespace {

constexpr double kFlatSlopeEpsilon = 1e-12;
constexpr double kInputGeometryEpsilon = 1e-9;
constexpr double kPi = 3.14159265358979323846;

struct FacetNormal {
	double nx = 0.0;
	double ny = 0.0;
	double nz = 0.0;
};

FacetNormal facet_normal(const Point& a, double za,
                         const Point& b, double zb,
                         const Point& c, double zc){
	double ux = b.get_x() - a.get_x();
	double uy = b.get_y() - a.get_y();
	double uz = zb - za;
	double vx = c.get_x() - a.get_x();
	double vy = c.get_y() - a.get_y();
	double vz = zc - za;

	return FacetNormal{
		uy * vz - uz * vy,
		uz * vx - ux * vz,
		ux * vy - uy * vx,
	};
}

double azimuth_cw_from_north(double dx, double dy){
	if(std::fabs(dx) < kFlatSlopeEpsilon && std::fabs(dy) < kFlatSlopeEpsilon){
		return -1.0;
	}
	double degrees = std::atan2(dx, dy) * 180.0 / kPi;
	return degrees < 0.0 ? degrees + 360.0 : degrees;
}

bool triangle_indices_valid(const Tin& tin, const IndexedTriangle& triangle){
	int site_count = static_cast<int>(tin.mesh.sites.size());
	if(triangle.a < 0 || triangle.b < 0 || triangle.c < 0) return false;
	if(triangle.a >= site_count || triangle.b >= site_count || triangle.c >= site_count) return false;
	return static_cast<int>(tin.z.size()) == site_count;
}

struct HeightVertex {
	double x = 0.0;
	double y = 0.0;
	double h = 0.0;
};

HeightVertex interpolate_zero(const HeightVertex& inside, const HeightVertex& outside){
	double t = inside.h / (inside.h - outside.h);
	return HeightVertex{
		inside.x + t * (outside.x - inside.x),
		inside.y + t * (outside.y - inside.y),
		0.0,
	};
}

double positive_volume(const std::array<HeightVertex, 3>& facet){
	std::vector<HeightVertex> kept;
	kept.reserve(4);
	for(int i = 0; i < 3; ++i){
		const HeightVertex& current = facet[i];
		const HeightVertex& next = facet[(i + 1) % 3];
		bool current_inside = current.h >= 0.0;
		bool next_inside = next.h >= 0.0;

		if(current_inside){
			kept.push_back(current);
		}
		if(current_inside != next_inside){
			kept.push_back(interpolate_zero(current_inside ? current : next,
			                                current_inside ? next : current));
		}
	}

	if(kept.size() < 3){
		return 0.0;
	}

	double volume = 0.0;
	for(size_t i = 1; i + 1 < kept.size(); ++i){
		double area2 = (kept[i].x - kept[0].x) * (kept[i + 1].y - kept[0].y) -
		               (kept[i].y - kept[0].y) * (kept[i + 1].x - kept[0].x);
		double area = 0.5 * std::fabs(area2);
		volume += area * (kept[0].h + kept[i].h + kept[i + 1].h) / 3.0;
	}
	return volume;
}

std::vector<std::vector<int>> build_adjacency(const Tin& tin){
	size_t site_count = tin.mesh.sites.size();
	std::vector<std::set<int>> neighbor_sets(site_count);
	for(const IndexedTriangle& triangle : tin.mesh.triangles){
		int corners[3] = {triangle.a, triangle.b, triangle.c};
		for(int i = 0; i < 3; ++i){
			for(int j = i + 1; j < 3; ++j){
				neighbor_sets.at(corners[i]).insert(corners[j]);
				neighbor_sets.at(corners[j]).insert(corners[i]);
			}
		}
	}

	std::vector<std::vector<int>> adjacency(site_count);
	for(size_t i = 0; i < site_count; ++i){
		adjacency[i].assign(neighbor_sets[i].begin(), neighbor_sets[i].end());
	}
	return adjacency;
}

} // namespace

TerrainInputStatus validate_terrain_input(
	const std::vector<Point>& ground_points,
	const std::vector<double>& elevations){
	if(ground_points.size() != elevations.size()){
		return TerrainInputStatus::point_elevation_count_mismatch;
	}
	if(ground_points.size() < 3){
		return TerrainInputStatus::insufficient_points;
	}

	std::set<std::pair<double, double>> seen;
	for(size_t i = 0; i < ground_points.size(); ++i){
		double x = ground_points[i].get_x();
		double y = ground_points[i].get_y();
		if(!std::isfinite(x) || !std::isfinite(y)){
			return TerrainInputStatus::non_finite_coordinate;
		}
		if(!std::isfinite(elevations[i])){
			return TerrainInputStatus::non_finite_elevation;
		}
		if(!seen.insert({x, y}).second){
			return TerrainInputStatus::duplicate_xy;
		}
	}

	const Point& base = ground_points.front();
	const Point& second = ground_points.at(1);
	bool has_area = false;
	for(size_t i = 2; i < ground_points.size(); ++i){
		if(std::fabs(orient2d(base, second, ground_points[i])) > kInputGeometryEpsilon){
			has_area = true;
			break;
		}
	}
	return has_area ? TerrainInputStatus::ok : TerrainInputStatus::collinear_xy;
}

const char* terrain_input_status_code(TerrainInputStatus status){
	switch(status){
		case TerrainInputStatus::ok: return "ok";
		case TerrainInputStatus::point_elevation_count_mismatch: return "point_elevation_count_mismatch";
		case TerrainInputStatus::insufficient_points: return "insufficient_points";
		case TerrainInputStatus::non_finite_coordinate: return "non_finite_coordinate";
		case TerrainInputStatus::non_finite_elevation: return "non_finite_elevation";
		case TerrainInputStatus::duplicate_xy: return "duplicate_xy";
		case TerrainInputStatus::collinear_xy: return "collinear_xy";
	}
	return "unknown_validation_status";
}

Tin build_tin(const std::vector<Point>& ground_points,
              const std::vector<double>& elevations){
	Tin tin;
	if(validate_terrain_input(ground_points, elevations) != TerrainInputStatus::ok){
		return tin;
	}

	tin.mesh = bowyer_watson_triangulate(ground_points);
	if(tin.mesh.sites.empty()){
		return tin;
	}

	std::map<std::pair<double, double>, double> elevation_by_coord;
	for(size_t i = 0; i < ground_points.size(); ++i){
		elevation_by_coord.insert({{ground_points[i].get_x(), ground_points[i].get_y()},
		                           elevations[i]});
	}

	tin.z.reserve(tin.mesh.sites.size());
	for(const Point& site : tin.mesh.sites){
		auto found = elevation_by_coord.find({site.get_x(), site.get_y()});
		if(found == elevation_by_coord.end()){
			return Tin{};
		}
		tin.z.push_back(found->second);
	}
	return tin;
}

TriangleMetrics triangle_metrics(const Tin& tin, const IndexedTriangle& triangle){
	TriangleMetrics metrics;
	if(!triangle_indices_valid(tin, triangle)){
		return metrics;
	}

	const Point& a = tin.mesh.sites.at(triangle.a);
	const Point& b = tin.mesh.sites.at(triangle.b);
	const Point& c = tin.mesh.sites.at(triangle.c);
	FacetNormal normal = facet_normal(a, tin.z.at(triangle.a),
	                                  b, tin.z.at(triangle.b),
	                                  c, tin.z.at(triangle.c));

	double abs_nz = std::fabs(normal.nz);
	metrics.planar_area = 0.5 * abs_nz;
	metrics.surface_area = 0.5 * std::sqrt(normal.nx * normal.nx +
	                                       normal.ny * normal.ny +
	                                       normal.nz * normal.nz);
	if(abs_nz <= kFlatSlopeEpsilon){
		return metrics;
	}

	double dz_dx = -normal.nx / normal.nz;
	double dz_dy = -normal.ny / normal.nz;
	metrics.slope = std::sqrt(dz_dx * dz_dx + dz_dy * dz_dy);
	metrics.slope_radians = std::atan(metrics.slope);
	metrics.aspect_degrees = metrics.slope <= kFlatSlopeEpsilon
		? -1.0
		: azimuth_cw_from_north(-dz_dx, -dz_dy);
	return metrics;
}

std::vector<TriangleMetrics> all_triangle_metrics(const Tin& tin){
	std::vector<TriangleMetrics> metrics;
	metrics.reserve(tin.mesh.triangles.size());
	for(const IndexedTriangle& triangle : tin.mesh.triangles){
		metrics.push_back(triangle_metrics(tin, triangle));
	}
	return metrics;
}

SlopeStats slope_stats(const Tin& tin){
	SlopeStats stats;
	double weighted_slope = 0.0;
	double total_area = 0.0;
	bool seen = false;

	for(const IndexedTriangle& triangle : tin.mesh.triangles){
		TriangleMetrics metrics = triangle_metrics(tin, triangle);
		if(!seen){
			stats.min_slope = metrics.slope;
			stats.max_slope = metrics.slope;
			seen = true;
		}else{
			stats.min_slope = std::min(stats.min_slope, metrics.slope);
			stats.max_slope = std::max(stats.max_slope, metrics.slope);
		}
		weighted_slope += metrics.slope * metrics.planar_area;
		total_area += metrics.planar_area;
	}

	if(total_area > 0.0){
		stats.mean_slope = weighted_slope / total_area;
	}
	return stats;
}

double balanced_pad_elevation(const Tin& tin){
	double weighted_height = 0.0;
	double total_area = 0.0;
	for(const IndexedTriangle& triangle : tin.mesh.triangles){
		if(!triangle_indices_valid(tin, triangle)){
			continue;
		}
		FacetNormal normal = facet_normal(tin.mesh.sites.at(triangle.a), tin.z.at(triangle.a),
		                                  tin.mesh.sites.at(triangle.b), tin.z.at(triangle.b),
		                                  tin.mesh.sites.at(triangle.c), tin.z.at(triangle.c));
		double area = 0.5 * std::fabs(normal.nz);
		double mean_height = (tin.z.at(triangle.a) + tin.z.at(triangle.b) + tin.z.at(triangle.c)) / 3.0;
		weighted_height += area * mean_height;
		total_area += area;
	}
	return total_area > 0.0 ? weighted_height / total_area : 0.0;
}

CutFillResult cut_fill(const Tin& tin, double pad_elevation){
	CutFillResult result;
	result.balanced_elevation = balanced_pad_elevation(tin);

	for(const IndexedTriangle& triangle : tin.mesh.triangles){
		if(!triangle_indices_valid(tin, triangle)){
			continue;
		}

		const Point& a = tin.mesh.sites.at(triangle.a);
		const Point& b = tin.mesh.sites.at(triangle.b);
		const Point& c = tin.mesh.sites.at(triangle.c);
		double ha = tin.z.at(triangle.a) - pad_elevation;
		double hb = tin.z.at(triangle.b) - pad_elevation;
		double hc = tin.z.at(triangle.c) - pad_elevation;

		FacetNormal normal = facet_normal(a, tin.z.at(triangle.a),
		                                  b, tin.z.at(triangle.b),
		                                  c, tin.z.at(triangle.c));
		double planar_area = 0.5 * std::fabs(normal.nz);
		double signed_volume = planar_area * (ha + hb + hc) / 3.0;
		std::array<HeightVertex, 3> facet{
			HeightVertex{a.get_x(), a.get_y(), ha},
			HeightVertex{b.get_x(), b.get_y(), hb},
			HeightVertex{c.get_x(), c.get_y(), hc},
		};
		double cut = positive_volume(facet);
		double fill = cut - signed_volume;

		result.cut += std::max(cut, 0.0);
		result.fill += std::max(fill, 0.0);
	}
	return result;
}

FlowResult flow_accumulation(const Tin& tin, double exponent){
	FlowResult result;
	size_t site_count = tin.mesh.sites.size();
	if(site_count == 0 || tin.z.size() != site_count){
		return result;
	}

	result.accumulation.assign(site_count, 1.0);
	result.steepest_descent_neighbor.assign(site_count, -1);
	result.steepest_descent_azimuth.assign(site_count, -1.0);
	std::vector<std::vector<int>> adjacency = build_adjacency(tin);

	for(size_t v = 0; v < site_count; ++v){
		double steepest = 0.0;
		for(int neighbor : adjacency[v]){
			double drop = tin.z[v] - tin.z[neighbor];
			if(drop <= 0.0) continue;
			double dx = tin.mesh.sites[neighbor].get_x() - tin.mesh.sites[v].get_x();
			double dy = tin.mesh.sites[neighbor].get_y() - tin.mesh.sites[v].get_y();
			double distance = std::hypot(dx, dy);
			if(distance <= 0.0) continue;
			double gradient = drop / distance;
			if(gradient > steepest){
				steepest = gradient;
				result.steepest_descent_neighbor[v] = neighbor;
				result.steepest_descent_azimuth[v] = azimuth_cw_from_north(dx, dy);
			}
		}
	}

	std::vector<int> order(site_count);
	for(size_t i = 0; i < site_count; ++i){
		order[i] = static_cast<int>(i);
	}
	std::stable_sort(order.begin(), order.end(), [&tin](int lhs, int rhs){
		return tin.z[lhs] > tin.z[rhs];
	});

	for(int v : order){
		std::vector<std::pair<int, double>> downslope;
		double weight_sum = 0.0;
		for(int neighbor : adjacency[v]){
			double drop = tin.z[v] - tin.z[neighbor];
			if(drop <= 0.0) continue;
			double dx = tin.mesh.sites[neighbor].get_x() - tin.mesh.sites[v].get_x();
			double dy = tin.mesh.sites[neighbor].get_y() - tin.mesh.sites[v].get_y();
			double distance = std::hypot(dx, dy);
			if(distance <= 0.0) continue;
			double weight = std::pow(drop / distance, exponent);
			downslope.push_back({neighbor, weight});
			weight_sum += weight;
		}

		if(weight_sum <= 0.0) continue;
		for(const auto& entry : downslope){
			result.accumulation[entry.first] += result.accumulation[v] * entry.second / weight_sum;
		}
	}
	return result;
}
