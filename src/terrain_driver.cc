// Command-line front end for the supported base terrain analysis.
//
// Input on stdin is a point count N followed by N lines of "x y z" values.
// The CLI contract uses metres for horizontal coordinates and elevations.
// --format=json emits the versioned machine contract used by integrations.

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "point.h"
#include "terrain.h"

using namespace std;

namespace {

constexpr const char* kSchemaVersion = "planar-terrain-analysis/v1";
constexpr int kExitUsageOrInput = 2;
constexpr int kExitAnalysis = 3;

struct Options {
	bool json = false;
	bool help = false;
	int top_k = 10;
	string error_code;
	string error_message;
};

string json_escape(const string& value){
	ostringstream out;
	for(unsigned char character : value){
		switch(character){
			case '"': out << "\\\""; break;
			case '\\': out << "\\\\"; break;
			case '\b': out << "\\b"; break;
			case '\f': out << "\\f"; break;
			case '\n': out << "\\n"; break;
			case '\r': out << "\\r"; break;
			case '\t': out << "\\t"; break;
			default:
				if(character < 0x20){
					out << "\\u" << hex << setw(4) << setfill('0')
					    << static_cast<int>(character) << dec << setfill(' ');
				}else{
					out << static_cast<char>(character);
				}
		}
	}
	return out.str();
}

void emit_error_json(const string& code, const string& message){
	cout << "{\"schema_version\":\"" << kSchemaVersion
	     << "\",\"status\":\"error\",\"error\":{\"code\":\""
	     << json_escape(code) << "\",\"message\":\""
	     << json_escape(message) << "\"}}\n";
}

int fail(const Options& options, const string& code, const string& message, int exit_code){
	if(options.json){
		emit_error_json(code, message);
	}else{
		cerr << "terrain_driver: " << code << ": " << message << "\n";
	}
	return exit_code;
}

bool parse_non_negative_int(const string& value, int& parsed){
	try {
		size_t consumed = 0;
		long long candidate = stoll(value, &consumed, 10);
		if(consumed != value.size() || candidate < 0 ||
		   candidate > numeric_limits<int>::max()){
			return false;
		}
		parsed = static_cast<int>(candidate);
		return true;
	} catch(const exception&){
		return false;
	}
}

Options parse_options(int argc, char* argv[]){
	Options options;
	for(int i = 1; i < argc; ++i){
		if(string(argv[i]) == "--format=json") options.json = true;
	}

	for(int i = 1; i < argc; ++i){
		string argument = argv[i];
		if(argument == "--format=json" || argument == "--format=text"){
			continue;
		}
		if(argument == "--units=metres"){
			continue;
		}
		if(argument == "--help" || argument == "-h"){
			options.help = true;
			continue;
		}
		if(argument.rfind("--top-k=", 0) == 0){
			if(!parse_non_negative_int(argument.substr(8), options.top_k)){
				options.error_code = "invalid_top_k";
				options.error_message = "--top-k must be a non-negative integer.";
			}
			continue;
		}
		if(argument.rfind("--units=", 0) == 0){
			options.error_code = "unsupported_units";
			options.error_message = "This CLI contract currently supports --units=metres only.";
			continue;
		}
		if(argument.rfind("--level", 0) == 0 || argument.rfind("--stage", 0) == 0){
			options.error_code = "unsupported_option";
			options.error_message = "Reservoir level/stage options are not part of the supported base analysis.";
			continue;
		}
		options.error_code = "unknown_option";
		options.error_message = "Unknown option: " + argument;
	}
	return options;
}

void print_help(){
	cout << "Usage: terrain_driver [--format=text|json] [--units=metres] [--top-k=N]\n"
	     << "\n"
	     << "Read N followed by N x/y/z metre triples from stdin. The base analysis\n"
	     << "reports a Delaunay TIN, balanced-pad cut/fill, slope, and MFD flow.\n"
	     << "JSON uses schema " << kSchemaVersion << ".\n"
	     << "Reservoir --level and --stage options are not supported.\n";
}

bool parse_double_token(const string& token, double& value){
	try {
		size_t consumed = 0;
		value = stod(token, &consumed);
		return consumed == token.size();
	} catch(const exception&){
		return false;
	}
}

struct ReadResult {
	vector<Point> points;
	vector<double> elevations;
	string error_code;
	string error_message;
};

ReadResult read_ground_points(istream& input){
	ReadResult result;
	string count_token;
	if(!(input >> count_token)){
		result.error_code = "missing_point_count";
		result.error_message = "Expected a point count before the XYZ records.";
		return result;
	}

	int count = 0;
	if(!parse_non_negative_int(count_token, count)){
		result.error_code = "invalid_point_count";
		result.error_message = "Point count must be a non-negative integer.";
		return result;
	}
	result.points.reserve(static_cast<size_t>(count));
	result.elevations.reserve(static_cast<size_t>(count));

	for(int i = 0; i < count; ++i){
		string x_token, y_token, z_token;
		if(!(input >> x_token >> y_token >> z_token)){
			result.error_code = "incomplete_point_data";
			result.error_message = "Expected exactly three values for every declared point.";
			return result;
		}
		double x = 0.0, y = 0.0, z = 0.0;
		if(!parse_double_token(x_token, x) ||
		   !parse_double_token(y_token, y) ||
		   !parse_double_token(z_token, z)){
			result.error_code = "invalid_numeric_value";
			result.error_message = "Every point coordinate and elevation must be numeric.";
			return result;
		}
		result.points.push_back(Point(x, y));
		result.elevations.push_back(z);
	}

	string extra;
	if(input >> extra){
		result.error_code = "unexpected_input";
		result.error_message = "Input contains values after the declared point records.";
	}
	return result;
}

string validation_message(TerrainInputStatus status){
	switch(status){
		case TerrainInputStatus::ok: return "Terrain input is valid.";
		case TerrainInputStatus::point_elevation_count_mismatch:
			return "Point and elevation counts must match.";
		case TerrainInputStatus::insufficient_points:
			return "At least three distinct, non-collinear points are required.";
		case TerrainInputStatus::non_finite_coordinate:
			return "Horizontal coordinates must be finite.";
		case TerrainInputStatus::non_finite_elevation:
			return "Elevations must be finite.";
		case TerrainInputStatus::duplicate_xy:
			return "Duplicate XY coordinates require an explicit elevation-resolution policy.";
		case TerrainInputStatus::collinear_xy:
			return "Terrain points must span a non-collinear planar footprint.";
	}
	return "Unknown terrain validation failure.";
}

vector<int> ranked_flow_sites(const FlowResult& flow){
	vector<int> ranked(flow.accumulation.size());
	for(size_t i = 0; i < ranked.size(); ++i){
		ranked[i] = static_cast<int>(i);
	}
	stable_sort(ranked.begin(), ranked.end(), [&flow](int lhs, int rhs){
		return flow.accumulation[lhs] > flow.accumulation[rhs];
	});
	return ranked;
}

double clean_zero(double value){
	return value == 0.0 ? 0.0 : value;
}

bool results_are_finite(const CutFillResult& earthwork,
	                    const SlopeStats& slope,
	                    const FlowResult& flow){
	if(!isfinite(earthwork.balanced_elevation) || !isfinite(earthwork.cut) ||
	   !isfinite(earthwork.fill) || !isfinite(slope.min_slope) ||
	   !isfinite(slope.mean_slope) || !isfinite(slope.max_slope)){
		return false;
	}
	for(double value : flow.accumulation){
		if(!isfinite(value)) return false;
	}
	return true;
}

void emit_json_success(const Tin& tin,
	                   const CutFillResult& earthwork,
	                   const SlopeStats& slope,
	                   const FlowResult& flow,
	                   int top_k){
	vector<int> ranked = ranked_flow_sites(flow);
	int emitted = min(static_cast<int>(ranked.size()), top_k);
	cout << setprecision(15);
	cout << "{\"schema_version\":\"" << kSchemaVersion << "\",\"status\":\"ok\",";
	cout << "\"units\":{\"horizontal\":\"metre\",\"elevation\":\"metre\",";
	cout << "\"area\":\"square_metre\",\"volume\":\"cubic_metre\",";
	cout << "\"slope\":\"rise_over_run\",\"aspect\":\"degree_clockwise_from_north\",";
	cout << "\"flow_accumulation\":\"unit_input_per_site\"},";
	cout << "\"input\":{\"point_count\":" << tin.mesh.sites.size() << "},";
	cout << "\"mesh\":{\"site_count\":" << tin.mesh.sites.size()
	     << ",\"triangle_count\":" << tin.mesh.triangles.size() << "},";
	cout << "\"balanced_pad\":{\"elevation\":" << clean_zero(earthwork.balanced_elevation)
	     << ",\"cut_volume\":" << clean_zero(earthwork.cut)
	     << ",\"fill_volume\":" << clean_zero(earthwork.fill) << "},";
	cout << "\"slope\":{\"minimum\":" << clean_zero(slope.min_slope)
	     << ",\"mean\":" << clean_zero(slope.mean_slope)
	     << ",\"maximum\":" << clean_zero(slope.max_slope) << "},";
	cout << "\"flow\":{\"top_k\":" << emitted << ",\"top\":[";
	for(int rank = 0; rank < emitted; ++rank){
		if(rank > 0) cout << ',';
		int index = ranked[rank];
		cout << "{\"rank\":" << (rank + 1)
		     << ",\"site_index\":" << index
		     << ",\"x\":" << clean_zero(tin.mesh.sites[index].get_x())
		     << ",\"y\":" << clean_zero(tin.mesh.sites[index].get_y())
		     << ",\"z\":" << clean_zero(tin.z[index])
		     << ",\"accumulation\":" << clean_zero(flow.accumulation[index]) << '}';
	}
	cout << "]}}\n";
}

void emit_text_success(const Tin& tin,
	                   const CutFillResult& earthwork,
	                   const SlopeStats& slope,
	                   const FlowResult& flow,
	                   int top_k){
	vector<int> ranked = ranked_flow_sites(flow);
	int emitted = min(static_cast<int>(ranked.size()), top_k);
	cout << fixed << setprecision(6);
	cout << "sites " << tin.mesh.sites.size() << "\n";
	cout << "triangles " << tin.mesh.triangles.size() << "\n";
	cout << "balanced_elevation_m " << earthwork.balanced_elevation << "\n";
	cout << "cut_m3 " << earthwork.cut << "\n";
	cout << "fill_m3 " << earthwork.fill << "\n";
	cout << "slope_min_rise_over_run " << slope.min_slope << "\n";
	cout << "slope_mean_rise_over_run " << slope.mean_slope << "\n";
	cout << "slope_max_rise_over_run " << slope.max_slope << "\n";
	cout << "flow_top " << emitted << "\n";
	cout << "rank index x_m y_m z_m accumulation_unit_input_per_site\n";
	for(int rank = 0; rank < emitted; ++rank){
		int index = ranked[rank];
		cout << (rank + 1) << ' ' << index << ' '
		     << tin.mesh.sites[index].get_x() << ' '
		     << tin.mesh.sites[index].get_y() << ' '
		     << tin.z[index] << ' '
		     << flow.accumulation[index] << "\n";
	}
}

} // namespace

int main(int argc, char* argv[]){
	Options options = parse_options(argc, argv);
	if(!options.error_code.empty()){
		return fail(options, options.error_code, options.error_message, kExitUsageOrInput);
	}
	if(options.help){
		print_help();
		return 0;
	}

	ReadResult input = read_ground_points(cin);
	if(!input.error_code.empty()){
		return fail(options, input.error_code, input.error_message, kExitUsageOrInput);
	}
	TerrainInputStatus validation = validate_terrain_input(input.points, input.elevations);
	if(validation != TerrainInputStatus::ok){
		return fail(options,
		            terrain_input_status_code(validation),
		            validation_message(validation),
		            kExitUsageOrInput);
	}

	Tin tin = build_tin(input.points, input.elevations);
	if(tin.mesh.triangles.empty()){
		return fail(options,
		            "triangulation_failed",
		            "Validated input did not produce a Delaunay terrain mesh.",
		            kExitAnalysis);
	}

	double balanced = balanced_pad_elevation(tin);
	CutFillResult earthwork = cut_fill(tin, balanced);
	SlopeStats slope = slope_stats(tin);
	FlowResult flow = flow_accumulation(tin);
	if(!results_are_finite(earthwork, slope, flow)){
		return fail(options,
		            "non_finite_analysis",
		            "Terrain analysis produced a non-finite result.",
		            kExitAnalysis);
	}

	if(options.json){
		emit_json_success(tin, earthwork, slope, flow, options.top_k);
	}else{
		emit_text_success(tin, earthwork, slope, flow, options.top_k);
	}
	return 0;
}
