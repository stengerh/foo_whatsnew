#include "stdafx.h"
#include "feature.h"

namespace feature_kinds
{
	// {DD0C183B-DB8C-4ad3-974C-C3C8F93A4BE7}
	static const GUID mainmenu_command = { 0xdd0c183b, 0xdb8c, 0x4ad3, { 0x97, 0x4c, 0xc3, 0xc8, 0xf9, 0x3a, 0x4b, 0xe7 } };
}

class feature_kind_mainmenu_command : public feature_kind
{
public:
	virtual GUID get_guid()
	{
		return feature_kinds::mainmenu_command;
	}

	virtual const char *get_name()
	{
		return "Main Menu Command";
	}

	virtual bool get_description(pfc::string_base &p_out)
	{
		p_out.reset();
		return false;
	}
};

static service_factory_single_t<feature_kind_mainmenu_command> g_feature_kind_factory;

class feature_scanner_mainmenu_command : public feature_scanner
{
	void get_command_path(pfc::string_base &p_out, GUID p_group)
	{
		if (p_group == pfc::guid_null)
		{
			p_out.reset();
		}
		else
		{
			service_enum_t<mainmenu_group> e;
			service_ptr_t<mainmenu_group> ptr;
			while (e.next(ptr))
			{
				if (ptr->get_guid() == p_group)
				{
					get_command_path(p_out, ptr->get_parent());

					service_ptr_t<mainmenu_group_popup> popup;
					if (ptr->service_query_t(popup))
					{
						pfc::string8 temp;
						popup->get_display_string(temp);
						p_out << temp << "/";
					}

					break;
				}
			}
		}
	}

public:
	virtual void scan(enum_feature_info_callback &p_callback)
	{
		feature_info_impl info;
		info.set_kind_guid(feature_kinds::mainmenu_command);

		typedef pfc::array_t<t_uint8, pfc::alloc_fast_aggressive> t_buffer;
		t_buffer buffer;

		// path and qualified name
		pfc::string8_fastalloc path, qname;

		service_enum_t<mainmenu_commands> e;
		service_ptr_t<mainmenu_commands> ptr;
		while (e.next(ptr))
		{
			path.reset();
			get_command_path(path, ptr->get_parent());

			const t_uint32 count = ptr->get_command_count();
			for (t_uint32 n = 0; n < count; ++n)
			{
				pfc::string8 name;
				ptr->get_name(n, name);

				qname = path;
				qname << name;

				info.set_name(qname);

				GUID guid = ptr->get_command(n);
				buffer.set_size(0);
				stream_writer_buffer_append_ref_t<t_buffer> stream(buffer);
				stream.write_lendian_t(guid, abort_callback_impl());
				info.set_key(buffer.get_ptr(), buffer.get_size());

				if (!p_callback.on_feature_info(info)) return;
			}
		}
	}
};

static service_factory_single_t<feature_scanner_mainmenu_command> g_feature_scanner_factory;
