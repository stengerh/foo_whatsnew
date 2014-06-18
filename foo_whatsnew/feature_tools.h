#pragma once

namespace feature_tools
{
	template <typename t_container, typename t_permutation>
	void sort_by_name_get_permutation_t(t_container p_container, t_size p_count, t_permutation p_permutation)
	{
		pfc::array_t<HANDLE> sort_strings;
		sort_strings.set_size(p_count);

		for (t_size n = 0; n < p_count; ++n)
		{
			sort_strings[n] = uSortStringCreate(p_container[n]->get_name());
		}

		pfc::sort_get_permutation_t(sort_strings, uSortStringCompare, p_count, p_permutation);

		for (t_size n = 0; n < p_count; ++n)
		{
			uSortStringFree(sort_strings[n]);
		}
	}
}

class feature_log_item_list : public pfc::list_t<feature_log_item_impl>
{
public:
	void sort_canonical();
};
