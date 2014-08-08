#include "stdafx.h"
#include "feature.h"

void feature_info::reset()
{
	set_name("");
	set_kind_guid(pfc::guid_null);
	set_key(0, 0);
#ifdef EXTRACT_COMPONENT_NAME
	clear_component_name();
#endif
}

#ifdef EXTRACT_COMPONENT_NAME
void feature_info::set_component_name_from_service(service_ptr p_ptr, component_name_resolver *p_resolver)
{
	HMODULE module = component_name_resolver::g_get_module_from_service(p_ptr);

	set_component_name("");

	if (module != NULL)
	{
		pfc::string8 component_name;

		if (p_resolver->get_component_name(module, component_name))
		{
			set_component_name(component_name);
		}
	}
}
#endif

int feature_info::compare_raw_data(const feature_info &p_item1, const feature_info &p_item2)
{
	int rv = pfc::compare_t(p_item1.get_key_size(), p_item2.get_key_size());
	if (rv != 0) return rv;
	rv = memcmp(p_item1.get_key(), p_item2.get_key(), p_item1.get_key_size());
	return rv;
}
