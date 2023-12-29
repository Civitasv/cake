#include "log.h"

std::ostream &operator<<(std::ostream &os, const std::vector<std::string> &args)
{
	for (size_t i = 0; i < args.size(); i++) {
		os << args[i];
		if (i != args.size() - 1) {
			os << " ";
		}
	}

	return os;
}