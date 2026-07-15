#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "build_info.h"
#include "delaunay.h"
#include "point.h"
#include "voronoi.h"

using namespace std;

namespace {

using Clock = chrono::steady_clock;

struct Options {
	int count = 100;
	uint64_t seed = 20260715ULL;
	int coord_max = 1000000;
	bool emit_points = false;
	bool valid = true;
	string error;
};

struct GeneratedInput {
	vector<Point> points;
	vector<pair<int64_t, int64_t>> exact_points;
	uint64_t hash = 1469598103934665603ULL;
};

class SplitMix64 {
private:
	uint64_t state;

public:
	explicit SplitMix64(uint64_t seed) : state(seed) {}

	uint64_t next(){
		uint64_t value = (state += 0x9e3779b97f4a7c15ULL);
		value = (value ^ (value >> 30U)) * 0xbf58476d1ce4e5b9ULL;
		value = (value ^ (value >> 27U)) * 0x94d049bb133111ebULL;
		return value ^ (value >> 31U);
	}

	uint64_t bounded(uint64_t bound){
		const uint64_t threshold = static_cast<uint64_t>(-bound) % bound;
		while(true){
			uint64_t value = next();
			if(value >= threshold){
				return value % bound;
			}
		}
	}
};

double seconds_between(Clock::time_point start, Clock::time_point end){
	return chrono::duration<double>(end - start).count();
}

void fnv_mix_u64(uint64_t& hash, uint64_t value){
	for(int shift = 0; shift < 64; shift += 8){
		hash ^= (value >> shift) & 0xffULL;
		hash *= 1099511628211ULL;
	}
}

string hex_u64(uint64_t value){
	ostringstream output;
	output << hex << setw(16) << setfill('0') << value;
	return output.str();
}

Options parse_options(int argc, char* argv[]){
	Options options;
	for(int i = 1; i < argc; ++i){
		string argument = argv[i];
		try {
			if(argument.rfind("--count=", 0) == 0){
				options.count = stoi(argument.substr(8));
			}else if(argument.rfind("--seed=", 0) == 0){
				options.seed = stoull(argument.substr(7));
			}else if(argument.rfind("--coord-max=", 0) == 0){
				options.coord_max = stoi(argument.substr(12));
			}else if(argument == "--emit-points"){
				options.emit_points = true;
			}else{
				options.valid = false;
				options.error = "unknown argument: " + argument;
				return options;
			}
		} catch(const exception&){
			options.valid = false;
			options.error = "invalid numeric argument: " + argument;
			return options;
		}
	}

	if(options.count < 3){
		options.valid = false;
		options.error = "--count must be at least 3";
	}else if(options.coord_max < 1 || options.coord_max > 1000000000){
		options.valid = false;
		options.error = "--coord-max must be in [1, 1000000000]";
	}else{
		const uint64_t axis = static_cast<uint64_t>(options.coord_max) * 2ULL + 1ULL;
		if(axis * axis < static_cast<uint64_t>(options.count)){
			options.valid = false;
			options.error = "coordinate domain cannot contain the requested unique points";
		}
	}
	return options;
}

GeneratedInput generate_input(const Options& options){
	GeneratedInput result;
	result.points.reserve(static_cast<size_t>(options.count));
	result.exact_points.reserve(static_cast<size_t>(options.count));

	SplitMix64 generator(options.seed);
	const uint64_t axis = static_cast<uint64_t>(options.coord_max) * 2ULL + 1ULL;
	set<pair<int64_t, int64_t>> used;
	while(static_cast<int>(result.points.size()) < options.count){
		int64_t x = static_cast<int64_t>(generator.bounded(axis)) - options.coord_max;
		int64_t y = static_cast<int64_t>(generator.bounded(axis)) - options.coord_max;
		if(!used.insert({x, y}).second){
			continue;
		}
		result.exact_points.push_back({x, y});
		result.points.push_back(Point(static_cast<double>(x), static_cast<double>(y)));
		fnv_mix_u64(result.hash, static_cast<uint64_t>(x));
		fnv_mix_u64(result.hash, static_cast<uint64_t>(y));
	}
	return result;
}

struct Validation {
	bool delaunay = false;
	bool adjacency = false;
	bool euler = false;
	bool voronoi = false;
	bool all = false;
	size_t unique_edges = 0;
	size_t hull_edges = 0;
	size_t internal_edges = 0;
	uint64_t output_hash = 1469598103934665603ULL;
};

Validation validate_result(const DelaunayTriangulation& triangulation,
	                       const VoronoiDiagram& voronoi){
	Validation result;
	result.delaunay = is_delaunay(triangulation);

	set<pair<int, int>> edges;
	bool adjacency_ok = triangulation.neighbors.size() == triangulation.triangles.size();
	for(size_t index = 0; index < triangulation.triangles.size(); ++index){
		const IndexedTriangle& triangle = triangulation.triangles.at(index);
		array<pair<int, int>, 3> triangle_edges{{
			{triangle.a, triangle.b},
			{triangle.b, triangle.c},
			{triangle.c, triangle.a}
		}};
		for(auto edge : triangle_edges){
			if(edge.second < edge.first) swap(edge.first, edge.second);
			edges.insert(edge);
		}

		if(!adjacency_ok) continue;
		for(int neighbor : triangulation.neighbors.at(index)){
			if(neighbor < 0){
				++result.hull_edges;
				continue;
			}
			if(neighbor >= static_cast<int>(triangulation.triangles.size())){
				adjacency_ok = false;
				break;
			}
			const auto& reverse = triangulation.neighbors.at(static_cast<size_t>(neighbor));
			if(find(reverse.begin(), reverse.end(), static_cast<int>(index)) == reverse.end()){
				adjacency_ok = false;
				break;
			}
		}
	}

	result.unique_edges = edges.size();
	result.internal_edges = result.unique_edges >= result.hull_edges
		? result.unique_edges - result.hull_edges
		: 0;
	result.adjacency = adjacency_ok;
	const long long euler = static_cast<long long>(triangulation.sites.size())
		- static_cast<long long>(result.unique_edges)
		+ static_cast<long long>(triangulation.triangles.size());
	result.euler = euler == 1;
	result.voronoi = voronoi.vertices.size() == triangulation.triangles.size()
		&& voronoi.edges.size() == result.internal_edges;

	vector<array<int, 3>> canonical_triangles;
	canonical_triangles.reserve(triangulation.triangles.size());
	for(const IndexedTriangle& triangle : triangulation.triangles){
		array<int, 3> canonical{{triangle.a, triangle.b, triangle.c}};
		sort(canonical.begin(), canonical.end());
		canonical_triangles.push_back(canonical);
	}
	sort(canonical_triangles.begin(), canonical_triangles.end());
	for(const auto& triangle : canonical_triangles){
		fnv_mix_u64(result.output_hash, static_cast<uint64_t>(triangle[0]));
		fnv_mix_u64(result.output_hash, static_cast<uint64_t>(triangle[1]));
		fnv_mix_u64(result.output_hash, static_cast<uint64_t>(triangle[2]));
	}

	result.all = result.delaunay && result.adjacency && result.euler && result.voronoi;
	return result;
}

void print_json(const Options& options,
	            const GeneratedInput& input,
	            const DelaunayTriangulation& triangulation,
	            const VoronoiDiagram& voronoi,
	            const Validation& validation,
	            double generation_seconds,
	            double triangulation_seconds,
	            double voronoi_seconds,
	            double validation_seconds){
	cout << fixed << setprecision(9);
	cout << "{\n";
	cout << "  \"schema_version\": 1,\n";
	cout << "  \"benchmark\": \"planar_delaunay_voronoi\",\n";
	cout << "  \"status\": \"" << (validation.all ? "ok" : "validation_failed") << "\",\n";
	cout << "  \"build\": {\"commit\": \"" << build_info::commit_full()
	     << "\", \"branch\": \"" << build_info::branch()
	     << "\", \"profile\": \"" << build_info::profile()
	     << "\", \"dirty\": " << (build_info::dirty() ? "true" : "false") << "},\n";
	cout << "  \"input\": {\"generator\": \"splitmix64_unique_integer_v1\", \"seed\": "
	     << options.seed << ", \"requested_count\": " << options.count
	     << ", \"coord_max\": " << options.coord_max
	     << ", \"hash\": \"fnv1a64:" << hex_u64(input.hash) << "\"";
	if(options.emit_points){
		cout << ", \"points\": [";
		for(size_t index = 0; index < input.exact_points.size(); ++index){
			if(index != 0) cout << ",";
			cout << "[" << input.exact_points[index].first << ","
			     << input.exact_points[index].second << "]";
		}
		cout << "]";
	}
	cout << "},\n";
	cout << "  \"timing_seconds\": {\"input_generation\": " << generation_seconds
	     << ", \"triangulation\": " << triangulation_seconds
	     << ", \"voronoi\": " << voronoi_seconds
	     << ", \"validation\": " << validation_seconds
	     << ", \"compute_total\": "
	     << (triangulation_seconds + voronoi_seconds + validation_seconds) << "},\n";
	cout << "  \"topology\": {\"sites\": " << triangulation.sites.size()
	     << ", \"triangles\": " << triangulation.triangles.size()
	     << ", \"unique_edges\": " << validation.unique_edges
	     << ", \"hull_edges\": " << validation.hull_edges
	     << ", \"internal_edges\": " << validation.internal_edges
	     << ", \"voronoi_vertices\": " << voronoi.vertices.size()
	     << ", \"voronoi_finite_edges\": " << voronoi.edges.size() << "},\n";
	cout << "  \"validation\": {\"delaunay\": " << (validation.delaunay ? "true" : "false")
	     << ", \"adjacency_symmetric\": " << (validation.adjacency ? "true" : "false")
	     << ", \"euler_planar\": " << (validation.euler ? "true" : "false")
	     << ", \"voronoi_dual_counts\": " << (validation.voronoi ? "true" : "false")
	     << ", \"all_passed\": " << (validation.all ? "true" : "false")
	     << ", \"canonical_output_hash\": \"fnv1a64:" << hex_u64(validation.output_hash)
	     << "\"}\n";
	cout << "}\n";
}

} // namespace

int main(int argc, char* argv[]){
	Options options = parse_options(argc, argv);
	if(!options.valid){
		cerr << "benchmark_driver: " << options.error << "\n";
		return 64;
	}

	const auto generation_start = Clock::now();
	GeneratedInput input = generate_input(options);
	const auto generation_end = Clock::now();

	const auto triangulation_start = Clock::now();
	DelaunayTriangulation triangulation = bowyer_watson_triangulate(input.points);
	const auto triangulation_end = Clock::now();

	const auto voronoi_start = Clock::now();
	VoronoiDiagram voronoi = build_voronoi_diagram(triangulation);
	const auto voronoi_end = Clock::now();

	const auto validation_start = Clock::now();
	Validation validation = validate_result(triangulation, voronoi);
	const auto validation_end = Clock::now();

	print_json(options,
	           input,
	           triangulation,
	           voronoi,
	           validation,
	           seconds_between(generation_start, generation_end),
	           seconds_between(triangulation_start, triangulation_end),
	           seconds_between(voronoi_start, voronoi_end),
	           seconds_between(validation_start, validation_end));
	return validation.all ? 0 : 2;
}
