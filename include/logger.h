// logger.h
// ----------- DEBUG TAGS --------------
// poly.gen
//
#pragma once

// ANSI Escape Code Defs
namespace ANSI {
const std::string RED = "\e[0;31m";
const std::string BLACK = "\e[0;30m";
const std::string GREEN = "\e[0;32m";
const std::string YELLOW = "\e[0;33m";
const std::string BLUE = "\e[0;34m";
const std::string PURPLE = "\e[0;35m";
const std::string CYAN = "\e[0;36m";
const std::string WHITE = "\e[0;37m";
const std::string RESET = "\e[0m";
const std::string BOLD_GREEN = "\e[1;32m";
}

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace logger {

struct RuntimeConfig {
	bool debug_enabled;
	std::vector<std::string> enabled_tags;
	bool match_prefix;
};

inline RuntimeConfig& runtime_config() {
#ifdef DEBUG
	static RuntimeConfig config{true, {"legacy"}, true};
#else
	static RuntimeConfig config{false, {}, true};
#endif
	return config;
}

inline std::vector<std::string> split_tags(std::string_view value) {
	std::vector<std::string> tags;
	std::size_t start = 0;
	while(start <= value.size()) {
		std::size_t end = value.find(',', start);
		if(end == std::string_view::npos) {
			end = value.size();
		}
		std::string tag(value.substr(start, end - start));
		tag.erase(std::remove_if(tag.begin(), tag.end(), [](unsigned char c) {
			return std::isspace(c);
		}), tag.end());
		if(!tag.empty()) {
			tags.push_back(tag);
		}
		if(end == value.size()) {
			break;
		}
		start = end + 1;
	}
	return tags;
}

inline bool is_tag_enabled(std::string_view tag) {
	const RuntimeConfig& config = runtime_config();
	if(!config.debug_enabled) {
		return false;
	}
	for(const std::string& configured_tag : config.enabled_tags) {
		if(configured_tag == "*") {
			return true;
		}
		if(tag == configured_tag) {
			return true;
		}
		if(config.match_prefix && tag.size() > configured_tag.size() &&
		   tag.substr(0, configured_tag.size()) == configured_tag &&
		   tag.at(configured_tag.size()) == '.') {
			return true;
		}
	}
	return false;
}

inline void set_debug_enabled(bool debug_enabled) {
	runtime_config().debug_enabled = debug_enabled;
}

inline void set_enabled_tags(std::vector<std::string> tags) {
	runtime_config().enabled_tags = std::move(tags);
}

inline void set_match_prefix(bool match_prefix) {
	runtime_config().match_prefix = match_prefix;
}

inline bool parse_bool_value(std::string_view value, bool fallback) {
	if(value == "1" || value == "true" || value == "TRUE" || value == "on") {
		return true;
	}
	if(value == "0" || value == "false" || value == "FALSE" || value == "off") {
		return false;
	}
	return fallback;
}

inline void apply_runtime_inputs(int argc, char* argv[]) {
	if(const char* env_tags = std::getenv("GEOM_DEBUG_TAGS")) {
		std::vector<std::string> tags = split_tags(env_tags);
		if(!tags.empty()) {
			set_enabled_tags(std::move(tags));
			set_debug_enabled(true);
		}
	}
	if(const char* env_prefix = std::getenv("GEOM_DEBUG_MATCH_PREFIX")) {
		set_match_prefix(parse_bool_value(env_prefix, runtime_config().match_prefix));
	}
	if(const char* env_debug = std::getenv("GEOM_DEBUG")) {
		set_debug_enabled(parse_bool_value(env_debug, runtime_config().debug_enabled));
	}

	for(int i = 1; i < argc; ++i) {
		std::string_view arg(argv[i]);
		if(arg == "--debug") {
			set_debug_enabled(true);
		}else if(arg == "--no-debug") {
			set_debug_enabled(false);
		}else if(arg.rfind("--debug-tags=", 0) == 0) {
			std::vector<std::string> tags = split_tags(arg.substr(13));
			set_enabled_tags(std::move(tags));
			set_debug_enabled(true);
		}else if(arg.rfind("--debug-match-prefix=", 0) == 0) {
			set_match_prefix(parse_bool_value(arg.substr(21), runtime_config().match_prefix));
		}
	}
}

template<typename... Args>
inline void log_debug(std::string_view tag, Args&&... fmt) {
	if(!is_tag_enabled(tag)) {
		return;
	}
	std::cerr << std::fixed << std::setprecision(20) << "[DEBUG][" << tag << "] ";
	(std::cerr << ... << std::forward<Args>(fmt));
}

} // namespace logger

#define DBG(msg) do { \
	if(::logger::is_tag_enabled("legacy")) { \
		std::cerr << std::fixed << std::setprecision(20) << "[DEBUG][legacy] " << msg; \
	} \
} while(0)

#define DBG_TAG(tag, msg) do { \
	if(::logger::is_tag_enabled(tag)) { \
		std::cerr << std::fixed << std::setprecision(20) << ANSI::YELLOW << "[DEBUG][" << tag << "] " << ANSI::RESET << msg; \
	} \
} while(0)
