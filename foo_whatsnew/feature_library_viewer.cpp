#include "stdafx.h"
#include "feature.h"

namespace feature_kinds
{
	static const GUID library_viewer = { 0xfaa31081, 0xae11, 0x4852, { 0x8f, 0xaa, 0x4c, 0x6, 0xf3, 0x32, 0xef, 0xf3 } };
}


class feature_kind_library_viewer : public feature_kind
{
public:
	virtual GUID get_guid()
	{
		return feature_kinds::library_viewer;
	}

	virtual const char *get_name()
	{
		return "Library Viewer";
	}

	virtual bool get_description(pfc::string_base &p_out)
	{
		return false;
	}
};

static service_factory_single_t<feature_kind_library_viewer> g_library_viewer_factory;

class feature_scanner_library_viewer : public feature_scanner
{
public:
#ifdef EXTRACT_COMPONENT_NAME
	virtual void scan(enum_feature_info_callback &p_callback, component_name_resolver * p_resolver)
#else
	virtual void scan(enum_feature_info_callback &p_callback)
#endif
	{
		service_enum_t<library_viewer> e;
		service_ptr_t<library_viewer> ptr;

		feature_info_impl info;
		info.set_kind_guid(feature_kinds::library_viewer);

		typedef pfc::array_t<t_uint8> t_buffer;
		t_buffer buffer;

		while (e.next(ptr))
		{
			info.set_name(ptr->get_name());

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

static service_factory_single_t<feature_scanner_library_viewer> g_feature_scanner_factory;
