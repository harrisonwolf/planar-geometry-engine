#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
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
	string distribution = "uniform";
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

string json_escape(const string& value){
	ostringstream output;
	for(char character : value){
		switch(character){
			case '\\': output << "\\\\"; break;
			case '"': output << "\\\""; break;
			case '\b': output << "\\b"; break;
			case '\f': output << "\\f"; break;
			case '\n': output << "\\n"; break;
			case '\r': output << "\\r"; break;
			case '\t': output << "\\t"; break;
			default:
				if(static_cast<unsigned char>(character) < 0x20U){
					output << "\\u" << hex << setw(4) << setfill('0')
					       << static_cast<int>(static_cast<unsigned char>(character));
				}else{
					output << character;
				}
		}
	}
	return output.str();
}

bool supported_distribution(const string& distribution){
	return distribution == "uniform"
		|| distribution == "clustered"
		|| distribution == "jittered-grid"
		|| distribution == "near-collinear";
}

const char* generator_name(const string& distribution){
	if(distribution == "clustered") return "splitmix64_four_cluster_integer_v1";
	if(distribution == "jittered-grid") return "splitmix64_jittered_grid_integer_v1";
	if(distribution == "near-collinear") return "splitmix64_near_collinear_integer_v1";
	return "splitmix64_uniform_unique_integer_v1";
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
			}else if(argument.rfind("--distribution=", 0) == 0){
				options.distribution = argument.substr(15);
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
	}else if(!supported_distribution(options.distribution)){
		options.valid = false;
		options.error = "--distribution must be uniform, clustered, jittered-grid, or near-collinear";
	}else{
		const uint64_t axis = static_cast<uint64_t>(options.coord_max) * 2ULL + 1ULL;
		if(axis * axis < static_cast<uint64_t>(options.count)){
			options.valid = false;
			options.error = "coordinate domain cannot contain the requested unique points";
		}else if(options.distribution == "clustered"){
			const uint64_t radius = max<uint64_t>(1, static_cast<uint64_t>(options.coord_max) / 8ULL);
			const uint64_t cluster_capacity = 4ULL * (2ULL * radius + 1ULL) * (2ULL * radius + 1ULL);
			if(options.coord_max < 8 || cluster_capacity < static_cast<uint64_t>(options.count)){
				options.valid = false;
				options.error = "clustered distribution needs four disjoint clusters large enough for --count";
			}
		}else if(options.distribution == "jittered-grid"){
			const uint64_t side = static_cast<uint64_t>(ceil(sqrt(static_cast<double>(options.count))));
			const uint64_t spacing = (2ULL * static_cast<uint64_t>(options.coord_max)) / (side - 1ULL);
			if(spacing < 5ULL){
				options.valid = false;
				options.error = "jittered-grid needs coordinate spacing of at least 5 integer units";
			}
		}else if(options.distribution == "near-collinear"
		         && static_cast<uint64_t>(options.count) > axis){
			options.valid = false;
			options.error = "near-collinear distribution needs one distinct x coordinate per point";
		}
	}
	return options;
}

void append_point(GeneratedInput& result, int64_t x, int64_t y){
	result.exact_points.push_back({x, y});
	result.points.push_back(Point(static_cast<double>(x), static_cast<double>(y)));
	fnv_mix_u64(result.hash, static_cast<uint64_t>(x));
	fnv_mix_u64(result.hash, static_cast<uint64_t>(y));
}

void generate_uniform(const Options& options, SplitMix64& generator, GeneratedInput& result){
	const uint64_t axis = static_cast<uint64_t>(options.coord_max) * 2ULL + 1ULL;
	set<pair<int64_t, int64_t>> used;
	while(static_cast<int>(result.points.size()) < options.count){
		const int64_t x = static_cast<int64_t>(generator.bounded(axis)) - options.coord_max;
		const int64_t y = static_cast<int64_t>(generator.bounded(axis)) - options.coord_max;
		if(used.insert({x, y}).second) append_point(result, x, y);
	}
}

void generate_clustered(const Options& options, SplitMix64& generator, GeneratedInput& result){
	const int64_t radius = max<int64_t>(1, static_cast<int64_t>(options.coord_max) / 8LL);
	const int64_t center = static_cast<int64_t>(options.coord_max) / 2LL;
	const uint64_t diameter = static_cast<uint64_t>(2LL * radius + 1LL);
	const array<pair<int64_t, int64_t>, 4> centers{{
		{-center, -center}, {-center, center}, {center, -center}, {center, center}
	}};
	set<pair<int64_t, int64_t>> used;
	while(static_cast<int>(result.points.size()) < options.count){
		const auto cluster = centers.at(result.points.size() % centers.size());
		const int64_t x = cluster.first + static_cast<int64_t>(generator.bounded(diameter)) - radius;
		const int64_t y = cluster.second + static_cast<int64_t>(generator.bounded(diameter)) - radius;
		if(used.insert({x, y}).second) append_point(result, x, y);
	}
}

void generate_jittered_grid(const Options& options, SplitMix64& generator, GeneratedInput& result){
	const int64_t extent = options.coord_max;
	const size_t side = static_cast<size_t>(ceil(sqrt(static_cast<double>(options.count))));
	const int64_t spacing = (2LL * extent) / static_cast<int64_t>(side - 1U);
	const int64_t jitter = spacing / 5LL;
	vector<pair<int64_t, int64_t>> candidates;
	candidates.reserve(side * side);
	for(size_t row = 0; row < side; ++row){
		for(size_t column = 0; column < side; ++column){
			const int64_t base_x = -extent + (2LL * extent * static_cast<int64_t>(column))
				/ static_cast<int64_t>(side - 1U);
			const int64_t base_y = -extent + (2LL * extent * static_cast<int64_t>(row))
				/ static_cast<int64_t>(side - 1U);
			const int64_t offset_x = static_cast<int64_t>(
				generator.bounded(static_cast<uint64_t>(2LL * jitter + 1LL))) - jitter;
			const int64_t offset_y = static_cast<int64_t>(
				generator.bounded(static_cast<uint64_t>(2LL * jitter + 1LL))) - jitter;
			const int64_t x = min<int64_t>(extent, max<int64_t>(-extent, base_x + offset_x));
			const int64_t y = min<int64_t>(extent, max<int64_t>(-extent, base_y + offset_y));
			candidates.push_back({x, y});
		}
	}
	for(size_t index = candidates.size(); index > 1U; --index){
		const size_t swap_index = static_cast<size_t>(generator.bounded(index));
		swap(candidates[index - 1U], candidates[swap_index]);
	}
	for(int index = 0; index < options.count; ++index){
		append_point(result, candidates.at(static_cast<size_t>(index)).first,
		             candidates.at(static_cast<size_t>(index)).second);
	}
}

void generate_near_collinear(const Options& options, SplitMix64& generator, GeneratedInput& result){
	const int64_t extent = options.coord_max;
	const int64_t band = max<int64_t>(1, extent / 10000LL);
	const uint64_t band_width = static_cast<uint64_t>(2LL * band + 1LL);
	for(int index = 0; index < options.count; ++index){
		const int64_t x = -extent + (2LL * extent * static_cast<int64_t>(index))
			/ static_cast<int64_t>(options.count - 1);
		int64_t y = static_cast<int64_t>(generator.bounded(band_width)) - band;
		if(index == 0 || index == options.count - 1) y = 0;
		if(index == options.count / 2) y = band;
		append_point(result, x, y);
	}
}

GeneratedInput generate_input(const Options& options){
	GeneratedInput result;
	result.points.reserve(static_cast<size_t>(options.count));
	result.exact_points.reserve(static_cast<size_t>(options.count));
	SplitMix64 generator(options.seed);
	if(options.distribution == "clustered"){
		generate_clustered(options, generator, result);
	}else if(options.distribution == "jittered-grid"){
		generate_jittered_grid(options, generator, result);
	}else if(options.distribution == "near-collinear"){
		generate_near_collinear(options, generator, result);
	}else{
		generate_uniform(options, generator, result);
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
	cout << "  \"build\": {\"commit\": \"" << json_escape(build_info::commit_full())
	     << "\", \"branch\": \"" << json_escape(build_info::branch())
	     << "\", \"profile\": \"" << json_escape(build_info::profile())
	     << "\", \"dirty\": " << (build_info::dirty() ? "true" : "false")
	     << ", \"compiler_command\": \"" << json_escape(build_info::compiler_command())
	     << "\", \"compiler_version\": \"" << json_escape(build_info::compiler_version())
	     << "\", \"compiler_flags\": \"" << json_escape(build_info::compiler_flags())
	     << "\"},\n";
	cout << "  \"input\": {\"generator\": \"" << generator_name(options.distribution)
	     << "\", \"distribution\": \"" << options.distribution << "\", \"seed\": "
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
