#include "stdafx.h"
#include "feature.h"

namespace feature_kinds
{
	static const GUID advanced_setting = { 0xaf04f1a0, 0xa325, 0x4801, { 0xb7, 0x17, 0xfb, 0x5e, 0x7c, 0x81, 0xb2, 0xe2 } };
}


class feature_kind_advanced_setting : public feature_kind
{
public:
	virtual GUID get_guid() {return feature_kinds::advanced_setting;}
	virtual const char *get_name() {return "Advanced Setting";}
	virtual bool get_description(pfc::string_base &p_out) {return false;}
};

static service_factory_single_t<feature_kind_advanced_setting> g_feature_kind_factory;

class feature_scanner_advanced_setting : public feature_scanner
{
	void get_setting_path(pfc::string_base &p_out, GUID p_guid)
	{
		if (p_guid == pfc::guid_null || p_guid == advconfig_entry::guid_root)
		{
			p_out.reset();
		}
		else
		{
			service_enum_t<advconfig_entry> e;
			service_ptr_t<advconfig_entry> ptr;
			while (e.next(ptr))
			{
				service_ptr_t<advconfig_branch> branch;
				if (ptr->service_query_t(branch) && branch->get_guid() == p_guid)
				{
					get_setting_path(p_out, branch->get_parent());
					pfc::string8 name;
					branch->get_name(name);
					p_out << name << "/";
					break;
				}
			}
		}
	}

public:
	virtual void scan(enum_feature_info_callback &p_callback)
	{
		service_enum_t<advconfig_entry> e;
		service_ptr_t<advconfig_entry> ptr;

		feature_info_impl info;

		info.set_kind_guid(feature_kinds::advanced_setting);

		typedef pfc::array_t<t_uint8> t_buffer;
		t_buffer buffer;

		pfc::string8_fastalloc name, qname;

		while (e.next(ptr))
		{
			service_ptr_t<advconfig_branch> branch;
			if (ptr->service_query_t(branch)) continue;

			name.reset();
			ptr->get_name(name);

			qname.reset();
			get_setting_path(qname, ptr->get_parent());
			qname << name;

			info.set_name(qname);

			GUID guid = ptr->get_guid();
			buffer.set_size(0);
			stream_writer_buffer_append_ref_t<t_buffer> stream(buffer);

			stream.write_lendian_t(guid, abort_callback_impl());

			info.set_key(buffer.get_ptr(), buffer.get_size());

			if (!p_callback.on_feature_info(info)) break;
		}
	}
};

static service_factory_single_t<feature_scanner_advanced_setting> g_feature_scanner_factory;
