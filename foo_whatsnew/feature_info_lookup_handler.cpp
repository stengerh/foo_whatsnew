#include "stdafx.h"
#include "feature.h"

namespace feature_kinds
{
	static const GUID info_lookup_handler = { 0xf774a704, 0xfd03, 0x4805, { 0x9b, 0xa9, 0x4b, 0xb7, 0x1e, 0xf2, 0xe2, 0x5b } };
}


class feature_kind_info_lookup_handler : public feature_kind
{
public:
	virtual GUID get_guid()
	{
		return feature_kinds::info_lookup_handler;
	}

	virtual const char *get_name()
	{
		return "Information Lookup Handler";
	}

	virtual bool get_description(pfc::string_base &p_out)
	{
		p_out = "Information Lookup Handlers retrieve information about a track or an album.";
		return true;
	}
};

static service_factory_single_t<feature_kind_info_lookup_handler> g_info_lookup_handler_factory;

class feature_scanner_info_lookup_handler : public feature_scanner
{
public:
#ifdef EXTRACT_COMPONENT_NAME
	virtual void scan(enum_feature_info_callback &p_callback, component_name_resolver * p_resolver)
#else
	virtual void scan(enum_feature_info_callback &p_callback)
#endif
	{
		service_enum_t<info_lookup_handler> e;
		service_ptr_t<info_lookup_handler> ptr;

		feature_info_impl info;
		info.set_kind_guid(feature_kinds::info_lookup_handler);

		typedef pfc::array_t<t_uint8> t_buffer;
		t_buffer buffer;

		while (e.next(ptr))
		{
			pfc::string8 name;
			ptr->get_name(name);
			info.set_name(name);
			info.set_key((t_uint8 const *)name.get_ptr(), name.get_length());

#ifdef EXTRACT_COMPONENT_NAME
			info.set_component_name_from_service(ptr, p_resolver);
#endif

			if (!p_callback.on_feature_info(info)) break;
		}
	}
};

static service_factory_single_t<feature_scanner_info_lookup_handler> g_feature_scanner_factory;
