#include <cstdint>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "delaunay.h"
#include "helper.h"
#include "logger.h"
#include "voronoi.h"

using namespace std;

namespace {

struct RuntimeOptions {
	bool valid = true;
	unsigned int seed = 1;
	int site_count = 300;
	double domain_width = 1000.0;
	double min_spacing_fraction = 0.025;
	string output_dir = "artifacts";
	string error_message;
};

RuntimeOptions parse_runtime_options(int argc, char* argv[]){
	RuntimeOptions options;
	for(int i = 1; i < argc; ++i){
		string arg = argv[i];
		if(arg.rfind("--seed=", 0) == 0){
			try {
				options.seed = static_cast<unsigned int>(stoul(arg.substr(7)));
			} catch(const exception&){
				options.valid = false;
				options.error_message = "Invalid value for --seed.";
				return options;
			}
		}else if(arg.rfind("--count=", 0) == 0){
			try {
				options.site_count = stoi(arg.substr(8));
			} catch(const exception&){
				options.valid = false;
				options.error_message = "Invalid value for --count.";
				return options;
			}
		}else if(arg.rfind("--out-dir=", 0) == 0){
			options.output_dir = arg.substr(10);
		}
	}

	if(options.site_count < 3){
		options.valid = false;
		options.error_message = "--count must be greater than or equal to 3.";
	}
	return options;
}

// Poisson-disc style rejection sampling: candidates closer than
// min_spacing_fraction * domain_width to an accepted site are rejected,
// which spreads the sites evenly and avoids sliver triangles.
vector<Point> generate_spaced_random_sites(const RuntimeOptions& options){
	mt19937 gen(options.seed);
	uniform_real_distribution<double> coord_dist(0.0, options.domain_width);

	const double min_spacing = options.min_spacing_fraction * options.domain_width;
	const double min_spacing_squared = min_spacing * min_spacing;
	const long long max_attempts = static_cast<long long>(options.site_count) * 10000LL;

	vector<Point> sites;
	sites.reserve(static_cast<size_t>(options.site_count));

	for(long long attempt = 0; attempt < max_attempts
		&& static_cast<int>(sites.size()) < options.site_count; ++attempt){
		Point candidate(coord_dist(gen), coord_dist(gen));

		bool too_close = false;
		for(const Point& site : sites){
			double dx = candidate.get_x() - site.get_x();
			double dy = candidate.get_y() - site.get_y();
			if(dx * dx + dy * dy < min_spacing_squared){
				too_close = true;
				break;
			}
		}
		if(!too_close){
			sites.push_back(candidate);
		}
	}

	if(static_cast<int>(sites.size()) < options.site_count){
		cout << "Site generation error: only placed " << sites.size() << " of "
		     << options.site_count << " sites with the required spacing.\n";
		return {};
	}
	return sites;
}

} // namespace

int main(int argc, char* argv[]){
	logger::apply_runtime_inputs(argc, argv);

	RuntimeOptions options = parse_runtime_options(argc, argv);
	if(!options.valid){
		cout << options.error_message << "\n";
		return 1;
	}

	vector<Point> sites = generate_spaced_random_sites(options);
	if(sites.empty()){
		return 1;
	}

	DelaunayTriangulation triangulation = bowyer_watson_triangulate(sites);
	if(triangulation.triangles.empty()){
		cout << "Delaunay triangulation failed.\n";
		return 1;
	}

	VoronoiDiagram voronoi = build_voronoi_diagram(triangulation);
	if(voronoi.vertices.empty() && voronoi.edges.empty()){
		cout << "Voronoi diagram generation failed.\n";
		return 1;
	}

	std::filesystem::create_directories(options.output_dir);
	string delaunay_path = options.output_dir + "/portfolio-delaunay-export.json";
	string voronoi_path = options.output_dir + "/portfolio-voronoi-export.json";
	bool delaunay_ok = write_delaunay_schema_file(triangulation, "portfolio_delaunay", delaunay_path);
	bool voronoi_ok = write_voronoi_schema_file(voronoi, "portfolio_voronoi", voronoi_path);
	if(!delaunay_ok || !voronoi_ok){
		cout << "Failed to export one or more Delaunay/Voronoi artifacts.\n";
		return 1;
	}

	cout << "Seed: " << options.seed << "\n";
	cout << "Input sites: " << sites.size() << "\n";
	cout << "Delaunay triangles: " << triangulation.triangles.size() << "\n";
	cout << "Voronoi vertices: " << voronoi.vertices.size() << "\n";
	cout << "Voronoi finite edges: " << voronoi.edges.size() << "\n";
	cout << "Delaunay validation: " << (is_delaunay(triangulation) ? "passed" : "failed") << "\n";
	cout << "Exported Delaunay artifact to "
	     << std::filesystem::absolute(delaunay_path).string() << "\n";
	cout << "Exported Voronoi artifact to "
	     << std::filesystem::absolute(voronoi_path).string() << "\n";
	return 0;
}
