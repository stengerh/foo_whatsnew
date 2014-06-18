#include "stdafx.h"
#include "feature.h"

int feature_info::compare_raw_data(const feature_info &p_item1, const feature_info &p_item2)
{
	int rv = pfc::compare_t(p_item1.get_key_size(), p_item2.get_key_size());
	if (rv != 0) return rv;
	rv = memcmp(p_item1.get_key(), p_item2.get_key(), p_item1.get_key_size());
	return rv;
}
