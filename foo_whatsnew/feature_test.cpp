#include "stdafx.h"
#include "feature.h"

#if defined(_DEBUG)

namespace feature_kinds
{
	static const GUID test = { 0x4af3defa, 0x474d, 0x4d3e, { 0x89, 0xe1, 0xe5, 0x10, 0x11, 0xb4, 0x61, 0xc8 } };
}

class feature_kind_test : public feature_kind
{
public:
	virtual GUID get_guid()
	{
		return feature_kinds::test;
	}

	virtual const char *get_name()
	{
		return "Test Feature Type";
	}

	virtual bool get_description(pfc::string_base &p_out)
	{
		return false;
	}
};

static service_factory_single_t<feature_kind_test> g_feature_kind_factory;

class feature_scanner_test : public feature_scanner
{
public:
	virtual void scan(enum_feature_info_callback &p_callback)
	{
		feature_info_impl info;
		info.set_kind_guid(feature_kinds::test);
		info.set_name("Test Feature");

		p_callback.on_feature_info(info);
	}
};

static service_factory_single_t<feature_scanner_test> g_feature_scanner_factory;
#endif
