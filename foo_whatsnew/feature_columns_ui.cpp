#include "stdafx.h"
#include "feature.h"
#include "../columns_ui-sdk/ui_extension.h"

namespace feature_kinds
{
	static const GUID columnsui_extension = { 0xffea8a57, 0x183d, 0x43c5, { 0xb2, 0xed, 0x89, 0x8d, 0xb3, 0x86, 0x1d, 0x3e } };
}


class feature_kind_columnsui_extension : public feature_kind
{
public:
	virtual GUID get_guid()
	{
		return feature_kinds::columnsui_extension;
	}

	virtual const char *get_name()
	{
		return "Columns UI Extension";
	}

	virtual bool get_description(pfc::string_base &p_out)
	{
		p_out = "Built with Columns UI SDK " UI_EXTENSION_VERSION ".";
		return true;
	}
};

static service_factory_single_t<feature_kind_columnsui_extension> g_columnsui_extension_factory;

class feature_scanner_columnsui_extension : public feature_scanner
{
public:
	virtual void scan(enum_feature_info_callback &p_callback)
	{
		service_enum_t<ui_extension::window> e;
		service_ptr_t<ui_extension::window> ptr;

		feature_info_impl info;
		info.set_kind_guid(feature_kinds::columnsui_extension);

		typedef pfc::array_t<t_uint8> t_buffer;
		t_buffer buffer;

		pfc::string8_fastalloc name, qname;

		while (e.next(ptr))
		{
			// Some components return NULL if they fail to initilize, e.g. foo_uie_sql_tree or foo_uie_tagger_mod.
			// http://www.hydrogenaudio.org/forums/index.php?s=&showtopic=52125&view=findpost&p=786386
			if (ptr.is_valid())
			{
				name.reset();
				ptr->get_name(name);

				qname.reset();
				ptr->get_category(qname);

				if (!qname.is_empty())
					qname << "/";
				qname << name;

				info.set_name(qname);

				GUID guid = ptr->get_extension_guid();
				buffer.set_size(0);
				stream_writer_buffer_append_ref_t<t_buffer> stream(buffer);

				stream.write_lendian_t(guid, abort_callback_impl());

				info.set_key(buffer.get_ptr(), buffer.get_size());

				if (!p_callback.on_feature_info(info)) break;
			}
		}
	}
};

static service_factory_single_t<feature_scanner_columnsui_extension> g_feature_scanner_factory;
