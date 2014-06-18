#include "stdafx.h"
#include "../foo_whatsnew/feature.h"

namespace feature_kinds
{
	static const GUID ui_element = { 0x63f2f3c7, 0x2c6d, 0x411c, { 0x93, 0x74, 0xab, 0x10, 0x11, 0xae, 0xeb, 0xd0 } };
}

class feature_kind_ui_element : public feature_kind
{
public:
	GUID get_guid()
	{
		return feature_kinds::ui_element;
	}

	const char *get_name()
	{
		return "UI Element";
	}

	bool get_description(pfc::string_base &p_out)
	{
		return false;
	}
};

static service_factory_single_t<feature_kind_ui_element> g_feature_kind_factory;

class feature_scanner_ui_element : public feature_scanner
{
public:
	void scan(enum_feature_info_callback &p_callback)
	{
		service_enum_t<ui_element> e;
		service_ptr_t<ui_element> ptr;
		pfc::string8_fastalloc name;
		feature_info_impl info;
		info.set_kind_guid(feature_kinds::ui_element);
		typedef pfc::array_t<t_uint8> t_buffer;
		t_buffer buffer;
		while (e.next(ptr))
		{
			if (ptr->is_user_addable())
			{
				name.reset();
				ptr->get_name(name);
				info.set_name(name);

				GUID guid = ptr->get_guid();
				buffer.set_size(0);
				stream_writer_buffer_append_ref_t<t_buffer> stream(buffer);

				stream.write_lendian_t(guid, abort_callback_impl());

				info.set_key(buffer.get_ptr(), buffer.get_size());

				if (!p_callback.on_feature_info(info)) break;
			}
		}
	}
};

static service_factory_t<feature_scanner_ui_element> g_feature_scanner_factory;
