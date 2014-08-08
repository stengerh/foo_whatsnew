#include "stdafx.h"
#include "feature.h"

namespace feature_kinds
{
	static const GUID preferences_page = { 0x899a625b, 0x6d70, 0x4c5d, { 0x91, 0x53, 0xd2, 0xe0, 0xca, 0x29, 0x56, 0x8c } };
}


class feature_kind_preferences_page : public feature_kind
{
public:
	virtual GUID get_guid()
	{
		return feature_kinds::preferences_page;
	}

	virtual const char *get_name()
	{
		return "Preferences Page";
	}

	virtual bool get_description(pfc::string_base &p_out)
	{
		return false;
	}
};

static service_factory_single_t<feature_kind_preferences_page> g_preferences_page_factory;

class feature_scanner_preferences_page : public feature_scanner
{
	void get_page_path(pfc::string_base &p_out, GUID p_guid)
	{
		if (p_guid == pfc::guid_null)
		{
			p_out.reset();
		}
		else
		{
			bool found = false;

			if (!found)
			{
				service_enum_t<preferences_page> e;
				service_ptr_t<preferences_page> ptr;
				while (e.next(ptr))
				{
					if (ptr->get_guid() == p_guid)
					{
						found = true;
						get_page_path(p_out, ptr->get_parent_guid());
						p_out << ptr->get_name() << "/";
						break;
					}
				}
			}

			if (!found)
			{
				service_enum_t<preferences_branch> e;
				service_ptr_t<preferences_branch> ptr;
				while (e.next(ptr))
				{
					if (ptr->get_guid() == p_guid)
					{
						found = true;
						get_page_path(p_out, ptr->get_parent_guid());
						p_out << ptr->get_name() << "/";
						break;
					}
				}
			}
		}
	}

public:
#ifdef EXTRACT_COMPONENT_NAME
	virtual void scan(enum_feature_info_callback &p_callback, component_name_resolver * p_resolver)
#else
	virtual void scan(enum_feature_info_callback &p_callback)
#endif
	{
		service_enum_t<preferences_page> e;
		service_ptr_t<preferences_page> ptr;

		feature_info_impl info;
		info.set_kind_guid(feature_kinds::preferences_page);

		pfc::string8_fastalloc path;

		typedef pfc::array_t<t_uint8> t_buffer;
		t_buffer buffer;

		while (e.next(ptr))
		{
			path.reset();
			get_page_path(path, ptr->get_parent_guid());
			path << ptr->get_name();

			info.set_name(path);

			GUID guid = ptr->get_guid();
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
};

static service_factory_single_t<feature_scanner_preferences_page> g_feature_scanner_factory;
