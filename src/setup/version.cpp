/*
 * Copyright (C) 2011-2013 Daniel Scharrer
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the author(s) be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "setup/version.hpp"

#include <cstring>
#include <algorithm>
#include <istream>
#include <ostream>

#include <boost/version.hpp>
#include <boost/static_assert.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/size.hpp>

#include "util/load.hpp"
#include "util/log.hpp"

namespace setup {

namespace {

typedef char stored_legacy_version[12];

struct known_legacy_version {
	
	char name[13]; // terminating 0 byte is ignored
	
	version_constant version;
	
	unsigned char bits;
	
	operator version_constant() const { return version; }
	
};

static const known_legacy_version legacy_versions[] = {
	{ "i1.2.10--16\x1a", INNO_VERSION(1, 2, 10), 16 },
	{ "i1.2.10--32\x1a", INNO_VERSION(1, 2, 10), 32 },
};

typedef char stored_version[64];

struct known_version {
	
	stored_version name;
	
	version_constant version;
	bool unicode;
	
	operator version_constant() const { return version; }
	
};

static const known_version versions[] = {
	{ "Inno Setup Setup Data (1.3.21)",     INNO_VERSION_EXT(1, 3, 21, 0), false },
	{ "Inno Setup Setup Data (1.3.25)",     INNO_VERSION_EXT(1, 3, 25, 0), false },
	{ "Inno Setup Setup Data (2.0.0)",      INNO_VERSION_EXT(2, 0,  0, 0), false },
	{ "Inno Setup Setup Data (2.0.1)",      INNO_VERSION_EXT(2, 0,  1, 0), false },
	{ "Inno Setup Setup Data (2.0.2)",      INNO_VERSION_EXT(2, 0,  2, 0), false }, // !
	{ "Inno Setup Setup Data (2.0.5)",      INNO_VERSION_EXT(2, 0,  5, 0), false },
	{ "Inno Setup Setup Data (2.0.6a)",     INNO_VERSION_EXT(2, 0,  6, 0), false },
	{ "Inno Setup Setup Data (2.0.7)",      INNO_VERSION_EXT(2, 0,  7, 0), false },
	{ "Inno Setup Setup Data (2.0.8)",      INNO_VERSION_EXT(2, 0,  8, 0), false },
	{ "Inno Setup Setup Data (2.0.11)",     INNO_VERSION_EXT(2, 0, 11, 0), false },
	{ "Inno Setup Setup Data (2.0.17)",     INNO_VERSION_EXT(2, 0, 17, 0), false },
	{ "Inno Setup Setup Data (2.0.18)",     INNO_VERSION_EXT(2, 0, 18, 0), false },
	{ "Inno Setup Setup Data (3.0.0a)",     INNO_VERSION_EXT(3, 0,  0, 0), false },
	{ "Inno Setup Setup Data (3.0.1)",      INNO_VERSION_EXT(3, 0,  1, 0), false },
	{ "Inno Setup Setup Data (3.0.3)",      INNO_VERSION_EXT(3, 0,  3, 0), false },
	{ "Inno Setup Setup Data (3.0.4)",      INNO_VERSION_EXT(3, 0,  4, 0), false }, // !
	{ "Inno Setup Setup Data (3.0.5)",      INNO_VERSION_EXT(3, 0,  5, 0), false },
	{ "My Inno Setup Extensions Setup Data (3.0.6.1)",
	                                        INNO_VERSION_EXT(3, 0,  6, 1), false },
	{ "Inno Setup Setup Data (4.0.0a)",     INNO_VERSION_EXT(4, 0,  0, 0), false },
	{ "Inno Setup Setup Data (4.0.1)",      INNO_VERSION_EXT(4, 0,  1, 0), false },
	{ "Inno Setup Setup Data (4.0.3)",      INNO_VERSION_EXT(4, 0,  3, 0), false },
	{ "Inno Setup Setup Data (4.0.5)",      INNO_VERSION_EXT(4, 0,  5, 0), false },
	{ "Inno Setup Setup Data (4.0.9)",      INNO_VERSION_EXT(4, 0,  9, 0), false },
	{ "Inno Setup Setup Data (4.0.10)",     INNO_VERSION_EXT(4, 0, 10, 0), false },
	{ "Inno Setup Setup Data (4.0.11)",     INNO_VERSION_EXT(4, 0, 11, 0), false },
	{ "Inno Setup Setup Data (4.1.0)",      INNO_VERSION_EXT(4, 1,  0, 0), false },
	{ "Inno Setup Setup Data (4.1.2)",      INNO_VERSION_EXT(4, 1,  2, 0), false },
	{ "Inno Setup Setup Data (4.1.3)",      INNO_VERSION_EXT(4, 1,  3, 0), false },
	{ "Inno Setup Setup Data (4.1.4)",      INNO_VERSION_EXT(4, 1,  4, 0), false },
	{ "Inno Setup Setup Data (4.1.5)",      INNO_VERSION_EXT(4, 1,  5, 0), false },
	{ "Inno Setup Setup Data (4.1.6)",      INNO_VERSION_EXT(4, 1,  6, 0), false },
	{ "Inno Setup Setup Data (4.1.8)",      INNO_VERSION_EXT(4, 1,  8, 0), false },
	{ "Inno Setup Setup Data (4.2.0)",      INNO_VERSION_EXT(4, 2,  0, 0), false },
	{ "Inno Setup Setup Data (4.2.1)",      INNO_VERSION_EXT(4, 2,  1, 0), false },
	{ "Inno Setup Setup Data (4.2.2)",      INNO_VERSION_EXT(4, 2,  2, 0), false },
	{ "Inno Setup Setup Data (4.2.3)",      INNO_VERSION_EXT(4, 2,  3, 0), false },
	{ "Inno Setup Setup Data (4.2.4)",      INNO_VERSION_EXT(4, 2,  4, 0), false }, // !
	{ "Inno Setup Setup Data (4.2.5)",      INNO_VERSION_EXT(4, 2,  5, 0), false },
	{ "Inno Setup Setup Data (4.2.6)",      INNO_VERSION_EXT(4, 2,  6, 0), false },
	{ "Inno Setup Setup Data (5.0.0)",      INNO_VERSION_EXT(5, 0,  0, 0), false },
	{ "Inno Setup Setup Data (5.0.1)",      INNO_VERSION_EXT(5, 0,  1, 0), false },
	{ "Inno Setup Setup Data (5.0.3)",      INNO_VERSION_EXT(5, 0,  3, 0), false },
	{ "Inno Setup Setup Data (5.0.4)",      INNO_VERSION_EXT(5, 0,  4, 0), false },
	{ "Inno Setup Setup Data (5.1.0)",      INNO_VERSION_EXT(5, 1,  0, 0), false },
	{ "Inno Setup Setup Data (5.1.2)",      INNO_VERSION_EXT(5, 1,  2, 0), false },
	{ "Inno Setup Setup Data (5.1.7)",      INNO_VERSION_EXT(5, 1,  7, 0), false },
	{ "Inno Setup Setup Data (5.1.10)",     INNO_VERSION_EXT(5, 1, 10, 0), false },
	{ "Inno Setup Setup Data (5.1.13)",     INNO_VERSION_EXT(5, 1, 13, 0), false },
	{ "Inno Setup Setup Data (5.2.0)",      INNO_VERSION_EXT(5, 2,  0, 0), false },
	{ "Inno Setup Setup Data (5.2.1)",      INNO_VERSION_EXT(5, 2,  1, 0), false },
	{ "Inno Setup Setup Data (5.2.3)",      INNO_VERSION_EXT(5, 2,  3, 0), false },
	{ "Inno Setup Setup Data (5.2.5)",      INNO_VERSION_EXT(5, 2,  5, 0), false },
	{ "Inno Setup Setup Data (5.2.5) (u)",  INNO_VERSION_EXT(5, 2,  5, 0), true  },
	{ "Inno Setup Setup Data (5.3.0)",      INNO_VERSION_EXT(5, 3,  0, 0), false },
	{ "Inno Setup Setup Data (5.3.0) (u)",  INNO_VERSION_EXT(5, 3,  0, 0), true  },
	{ "Inno Setup Setup Data (5.3.3)",      INNO_VERSION_EXT(5, 3,  3, 0), false },
	{ "Inno Setup Setup Data (5.3.3) (u)",  INNO_VERSION_EXT(5, 3,  3, 0), true  },
	{ "Inno Setup Setup Data (5.3.5)",      INNO_VERSION_EXT(5, 3,  5, 0), false },
	{ "Inno Setup Setup Data (5.3.5) (u)",  INNO_VERSION_EXT(5, 3,  5, 0), true  },
	{ "Inno Setup Setup Data (5.3.6)",      INNO_VERSION_EXT(5, 3,  6, 0), false },
	{ "Inno Setup Setup Data (5.3.6) (u)",  INNO_VERSION_EXT(5, 3,  6, 0), true  },
	{ "Inno Setup Setup Data (5.3.7)",      INNO_VERSION_EXT(5, 3,  7, 0), false },
	{ "Inno Setup Setup Data (5.3.7) (u)",  INNO_VERSION_EXT(5, 3,  7, 0), true  },
	{ "Inno Setup Setup Data (5.3.8)",      INNO_VERSION_EXT(5, 3,  8, 0), false },
	{ "Inno Setup Setup Data (5.3.8) (u)",  INNO_VERSION_EXT(5, 3,  8, 0), true  },
	{ "Inno Setup Setup Data (5.3.9)",      INNO_VERSION_EXT(5, 3,  9, 0), false },
	{ "Inno Setup Setup Data (5.3.9) (u)",  INNO_VERSION_EXT(5, 3,  9, 0), true  },
	{ "Inno Setup Setup Data (5.3.10)",     INNO_VERSION_EXT(5, 3, 10, 0), false },
	{ "Inno Setup Setup Data (5.3.10) (u)", INNO_VERSION_EXT(5, 3, 10, 0), true  },
	{ "Inno Setup Setup Data (5.4.2)",      INNO_VERSION_EXT(5, 4,  2, 0), false },
	{ "Inno Setup Setup Data (5.4.2) (u)",  INNO_VERSION_EXT(5, 4,  2, 0), true  },
	{ "Inno Setup Setup Data (5.5.0)",      INNO_VERSION_EXT(5, 5,  0, 0), false },
	{ "Inno Setup Setup Data (5.5.0) (u)",  INNO_VERSION_EXT(5, 5,  0, 0), true  },
	{ "Inno Setup Setup Data (5.5.6)",      INNO_VERSION_EXT(5, 5,  6, 0), false },
	{ "Inno Setup Setup Data (5.5.6) (u)",  INNO_VERSION_EXT(5, 5,  6, 0), true  },	
	{ "!!! BlackBox v2?, marked as 5.5.0",  INNO_VERSION_EXT(5, 5,  0, 1), true  },
	
};

} // anonymous namespace

std::ostream & operator<<(std::ostream & os, const version & v) {
	
	os << v.a() << '.' << v.b() << '.' << v.c();
	if(v.d()) {
		os << '.' << v.d();
	}
	
	if(v.unicode) {
		os << " (unicode)";
	}
	
	if(v.bits != 32) {
		os << " (" << int(v.bits) << "-bit)";
	}
	
	return os;
}

void version::load(std::istream & is) {
	
	static const char digits[] = "0123456789";
	
	BOOST_STATIC_ASSERT(sizeof(stored_legacy_version) <= sizeof(stored_version));
	
	stored_legacy_version legacy_version;
	is.read(legacy_version, std::streamsize(sizeof(legacy_version)));
	
	if(legacy_version[0] == 'i' && legacy_version[sizeof(legacy_version) - 1] == '\x1a') {
		
		for(size_t i = 0; i < size_t(boost::size(legacy_versions)); i++) {
			if(!memcmp(legacy_version, legacy_versions[i].name, sizeof(legacy_version))) {
				value = legacy_versions[i].version;
				bits = legacy_versions[i].bits;
				unicode = false;
				known = true;
				debug("known legacy version: \"" << versions[i].name << '"');
				return;
			}
		}
		
		debug("unknown legacy version: \""
		      << std::string(legacy_version, sizeof(legacy_version)) << '"');
		
		if(legacy_version[0] != 'i' || legacy_version[2] != '.' || legacy_version[4] != '.'
		   || legacy_version[7] != '-' || legacy_version[8] != '-') {
			throw version_error();
		}
		
		if(legacy_version[9] == '1' && legacy_version[10] == '6') {
			bits = 16;
		} else if(legacy_version[9] == '3' && legacy_version[10] == '2') {
			bits = 32;
		} else {
			throw version_error();
		}
		
		std::string version_str(legacy_version, sizeof(legacy_version));
		
		try {
			unsigned a = util::to_unsigned(version_str.data() + 1, 1);
			unsigned b = util::to_unsigned(version_str.data() + 3, 1);
			unsigned c = util::to_unsigned(version_str.data() + 5, 2);
			value = INNO_VERSION(a, b, c);
		} catch(boost::bad_lexical_cast) {
			throw version_error();
		}
		
		unicode = false;
		known = false;
		
		return;
	}
	
	stored_version version;
	BOOST_STATIC_ASSERT(sizeof(legacy_version) <= sizeof(version));
	memcpy(version, legacy_version, sizeof(legacy_version));
	is.read(version + sizeof(legacy_version),
	        std::streamsize(sizeof(version) - sizeof(legacy_version)));
	
	
	for(size_t i = 0; i < size_t(boost::size(versions)); i++) {
		if(!memcmp(version, versions[i].name, sizeof(version))) {
			value = versions[i].version;
			bits = 32;
			unicode = versions[i].unicode;
			known = true;
			debug("known version: \"" << versions[i].name << '"');
			return;
		}
	}
	
	char * end = std::find(version, version + boost::size(version), '\0');
	std::string version_str(version, end);
	debug("unknown version: \"" << version_str << '"');
	if(version_str.find("Inno Setup") == std::string::npos) {
		throw version_error();
	}
	
	size_t bracket = version_str.find('(');
	for(; bracket != std::string::npos; bracket = version_str.find('(', bracket + 1)) {
		
		if(version_str.length() - bracket < 6) {
			continue;
		}
		
		try {
			
			size_t a_start = bracket + 1;
			size_t a_end = version_str.find_first_not_of(digits, a_start);
			if(a_end == std::string::npos || version_str[a_end] != '.') {
				continue;
			}
			unsigned a = util::to_unsigned(version_str.data() + a_start, a_end - a_start);
			
			size_t b_start = a_end + 1;
			size_t b_end = version_str.find_first_not_of(digits, b_start);
			if(b_end == std::string::npos || version_str[b_end] != '.') {
				continue;
			}
			unsigned b = util::to_unsigned(version_str.data() + b_start, b_end - b_start);
			
			size_t c_start = b_end + 1;
			size_t c_end = version_str.find_first_not_of(digits, c_start);
			if(c_end == std::string::npos) {
				continue;
			}
			unsigned c = util::to_unsigned(version_str.data() + c_start, c_end - c_start);
			
			size_t d_start = c_end;
			if(version_str[d_start] == 'a') {
				if(d_start + 1 >= version_str.length()) {
					continue;
				}
				d_start++;
			}
			
			unsigned d = 0;
			if(version_str[d_start] == '.') {
				d_start++;
				size_t d_end = version_str.find_first_not_of(digits, d_start);
				if(d_end != std::string::npos && d_end != d_start) {
					d = util::to_unsigned(version_str.data() + d_start, d_end - d_start);
				}
			}
			
			value = INNO_VERSION_EXT(a, b, c, d);
			break;
			
		} catch(boost::bad_lexical_cast) {
			continue;
		}
	}
	if(bracket == std::string::npos) {
		throw version_error();
	}
	
	bits = 32;
	unicode = (version_str.find("(u)") != std::string::npos);
	known = false;
}

bool version::is_ambiguous() const {
	
	if(value == INNO_VERSION(2, 0, 1)) {
		// might be either 2.0.1 or 2.0.2
		return true;
	}
	
	if(value == INNO_VERSION(3, 0, 3)) {
		// might be either 3.0.3 or 3.0.4
		return true;
	}
	
	if(value == INNO_VERSION(4, 2, 3)) {
		// might be either 4.2.3 or 4.2.4
		return true;
	}
	
	if(value == INNO_VERSION(5, 5, 0)) {
		// might be either 5.5.0 or 5.5.0.1
		return true;
	}
	
	return false;
}

version_constant version::next() {
	
	const known_legacy_version * legacy_end = boost::end(legacy_versions);
	const known_legacy_version * legacy_version;
	legacy_version = std::upper_bound(boost::begin(legacy_versions), legacy_end, value);
	if(legacy_version != legacy_end) {
		return legacy_version->version;
	}
	
	const known_version * end = boost::end(versions);
	const known_version * version = std::upper_bound(boost::begin(versions), end, value);
	if(version != end) {
		return version->version;
	}
	
	return 0;
}

} // namespace setup
