#ifndef BUILD_INFO_H
#define BUILD_INFO_H

#include <string>

#ifndef GEOM_BUILD_COMMIT
#define GEOM_BUILD_COMMIT "unknown"
#endif

#ifndef GEOM_BUILD_BRANCH
#define GEOM_BUILD_BRANCH "unknown"
#endif

#ifndef GEOM_BUILD_TIME_UTC
#define GEOM_BUILD_TIME_UTC "unknown"
#endif

#ifndef GEOM_BUILD_PROFILE
#define GEOM_BUILD_PROFILE "unknown"
#endif

#ifndef GEOM_BUILD_DIRTY
#define GEOM_BUILD_DIRTY 0
#endif

namespace build_info {

inline std::string commit_full() {
	return GEOM_BUILD_COMMIT;
}

inline std::string commit_short() {
	const std::string full = commit_full();
	if(full.size() <= 7) {
		return full;
	}
	return full.substr(0, 7);
}

inline std::string branch() {
	return GEOM_BUILD_BRANCH;
}

inline std::string build_time_utc() {
	return GEOM_BUILD_TIME_UTC;
}

inline std::string profile() {
	return GEOM_BUILD_PROFILE;
}

inline bool dirty() {
	return GEOM_BUILD_DIRTY != 0;
}

inline std::string provenance_summary() {
	std::string summary = branch() + " @ " + commit_short();
	if(dirty()) {
		summary += " (dirty)";
	}
	summary += " built " + build_time_utc();
	return summary;
}

} // namespace build_info

#endif
