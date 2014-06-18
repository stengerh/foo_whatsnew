#include "stdafx.h"
#include "feature.h"

namespace feature_kinds
{
	static const GUID feature_kind = { 0x63394afd, 0xa265, 0x48bf, { 0xa9, 0x2b, 0x3c, 0x24, 0xa9, 0x85, 0x1e, 0x1b } };
}


class feature_kind_feature_kind : public feature_kind
{
public:
	virtual GUID get_guid()
	{
		return feature_kinds::feature_kind;
	}

	virtual const char *get_name()
	{
		return "Feature Type";
	}

	virtual bool get_description(pfc::string_base &p_out)
	{
		return false;
	}
};

static service_factory_single_t<feature_kind_feature_kind> g_feature_kind_factory;

class feature_scanner_feature_kind : public feature_scanner
{
public:
	virtual void scan(enum_feature_info_callback &p_callback)
	{
		service_enum_t<feature_kind> e;
		service_ptr_t<feature_kind> ptr;

		pfc::string8_fastalloc name;

		feature_info_impl info;
		info.set_kind_guid(feature_kinds::feature_kind);

		typedef pfc::array_t<t_uint8> t_buffer;
		t_buffer buffer;

		while (e.next(ptr))
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
};

static service_factory_single_t<feature_scanner_feature_kind> g_feature_scanner_factory;
