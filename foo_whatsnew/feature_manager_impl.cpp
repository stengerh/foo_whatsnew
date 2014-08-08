#include "stdafx.h"
#include "feature.h"

static critical_section g_cs;

class comparator_feature_info
{
public:
	static int compare(const feature_info *p_item1, const feature_info *p_item2)
	{
		int rv = pfc::compare_t(p_item1->get_kind_guid(), p_item2->get_kind_guid());
		if (rv == 0) {
			rv = feature_info::compare_raw_data(*p_item1, *p_item2);
		}
		return rv;
	}
};

static pfc::map_t<const feature_info *, feature_handle *, comparator_feature_info> g_handles;

class feature_handle_impl : public feature_handle
{
	feature_info_impl m_info;
public:
	feature_handle_impl(const feature_info &p_info) : m_info(p_info) {}
	~feature_handle_impl()
	{
		c_insync lock(g_cs);
		g_handles.remove(&m_info);
	}

	virtual const feature_info *get_info() {return &m_info;}
};

class feature_manager_impl : public feature_manager
{
public:
	virtual feature_handle_ptr create_handle(const feature_info &p_info)
	{
		feature_handle_ptr rv;

		c_insync lock(g_cs);
		feature_handle *to = 0;
		if (g_handles.query(&p_info, to))
		{
			rv = to;
		}
		else
		{
			rv = new service_impl_t<feature_handle_impl>(p_info);
			g_handles.set(rv->get_info(), rv.get_ptr());
		}

		return rv;
	}
};

static service_factory_single_t<feature_manager_impl> g_feature_manager_factory;
