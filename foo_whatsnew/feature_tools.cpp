#include "stdafx.h"
#include "feature.h"
#include "feature_tools.h"

struct t_compare_data
{
	// descending
	t_filetimestamp m_timestamp;
	// ascending
	unsigned m_change;
	// ascending
	t_size m_feature_kind_priority;
	// ascending
	HANDLE m_name;

	static int compare(const t_compare_data &p_item1, const t_compare_data &p_item2)
	{
		int rv = -pfc::compare_t(p_item1.m_timestamp, p_item2.m_timestamp);
		if (rv == 0) rv = pfc::compare_t(p_item1.m_change, p_item2.m_change);
		if (rv == 0) rv = pfc::compare_t(p_item1.m_feature_kind_priority, p_item2.m_feature_kind_priority);
		if (rv == 0) rv = uSortStringCompare(p_item1.m_name, p_item2.m_name);
		return rv;
	}
};

static void g_get_feature_kind_priorities(pfc::map_t<GUID, t_size> &p_out)
{
	service_instance_array_t<feature_kind> kinds;

	const t_size count = kinds.get_size();

	pfc::array_t<t_size> order;
	order.set_size(count);
	order_helper::g_fill(order);

	feature_tools::sort_by_name_get_permutation_t(kinds, count, order.get_ptr());

	for (t_size n = 0; n < count; ++n)
	{
		p_out.set(kinds[n]->get_guid(), order_helper::g_find_reverse(order.get_ptr(), n));
	}
}

void feature_log_item_list::sort_canonical()
{
	pfc::map_t<GUID, t_size> feature_kind_priorities;
	g_get_feature_kind_priorities(feature_kind_priorities);

	pfc::array_t<t_compare_data> data;
	pfc::array_t<t_size> order;

	const t_size count = get_count();
	data.set_count(count);
	order.set_count(count);
	order_helper::g_fill(order);

	for (t_size n = 0; n < count; ++n)
	{
		const feature_log_item &item = get_item_ref(n);
		data[n].m_timestamp = item.get_timestamp();
		data[n].m_change = item.get_change_kind();
		feature_kind_priorities.query(item.get_feature()->get_info()->get_kind_guid(), data[n].m_feature_kind_priority);
		data[n].m_name = uSortStringCreate(item.get_feature()->get_info()->get_name());
	}

	pfc::sort_get_permutation_t(data, t_compare_data::compare, data.get_count(), order.get_ptr());
	reorder(order.get_ptr());

	for (t_size n = 0; n < count; ++n)
	{
		uSortStringFree(data[n].m_name);
	}
}
