#include "stdafx.h"
#include "feature.h"

namespace feature_kinds
{
	static const GUID contextmenu_command = { 0x2666915c, 0xc83a, 0x47aa, { 0xb1, 0x29, 0x9f, 0xc5, 0xb, 0x1a, 0x3f, 0x73 } };
}


class feature_kind_contextmenu_command : public feature_kind
{
public:
	virtual GUID get_guid()
	{
		return feature_kinds::contextmenu_command;
	}

	virtual const char *get_name()
	{
		return "Context Menu Command";
	}

	virtual bool get_description(pfc::string_base &p_out)
	{
		return false;
	}
};

static service_factory_single_t<feature_kind_contextmenu_command> g_contextmenu_command_factory;

class feature_scanner_contextmenu_command : public feature_scanner
{
public:
#ifdef EXTRACT_COMPONENT_NAME
	virtual void scan(enum_feature_info_callback &p_callback, component_name_resolver * p_resolver)
#else
	virtual void scan(enum_feature_info_callback &p_callback)
#endif
	{
		service_enum_t<contextmenu_item> e;
		service_ptr_t<contextmenu_item> ptr;

		feature_info_impl info;
		info.set_kind_guid(feature_kinds::contextmenu_command);

		typedef pfc::array_t<t_uint8> t_buffer;
		t_buffer buffer;

		pfc::string8_fastalloc name, path;

		while (e.next(ptr))
		{
			const unsigned count = ptr->get_num_items();

			for (unsigned n = 0; n < count; ++n)
			{
				name.reset();
				ptr->get_item_name(n, name);

				path.reset();
				ptr->get_item_default_path(n, path);

				if (!path.is_empty())
					path << "/";
				path << name;

				info.set_name(path);

				GUID guid = ptr->get_item_guid(n);
				buffer.set_size(0);
				stream_writer_buffer_append_ref_t<t_buffer> stream(buffer);

				stream.write_lendian_t(guid, abort_callback_impl());

				info.set_key(buffer.get_ptr(), buffer.get_size());

#ifdef EXTRACT_COMPONENT_NAME
				info.set_component_name_from_service(ptr, p_resolver);
#endif

				if (!p_callback.on_feature_info(info)) break;
			}
		}
	}
};

static service_factory_single_t<feature_scanner_contextmenu_command> g_feature_scanner_factory;
