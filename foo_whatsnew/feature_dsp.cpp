#include "stdafx.h"
#include "feature.h"

namespace feature_kinds
{
	const GUID dsp = { 0x1de67160, 0x5aa1, 0x4559, { 0x9b, 0x7e, 0x25, 0x40, 0x6a, 0xb4, 0xd5, 0x18 } };
}


class feature_kind_dsp : public feature_kind
{
public:
	virtual GUID get_guid()
	{
		return feature_kinds::dsp;
	}

	virtual const char *get_name()
	{
		return "DSP";
	}

	virtual bool get_description(pfc::string_base &p_out)
	{
		return false;
	}
};

static service_factory_single_t<feature_kind_dsp> g_dsp_factory;

class feature_scanner_dsp : public feature_scanner
{
public:
#ifdef EXTRACT_COMPONENT_NAME
	virtual void scan(enum_feature_info_callback &p_callback, component_name_resolver * p_resolver)
#else
	virtual void scan(enum_feature_info_callback &p_callback)
#endif
	{
		service_enum_t<dsp_entry> e;
		service_ptr_t<dsp_entry> ptr;

		feature_info_impl info;
		info.set_kind_guid(feature_kinds::dsp);

		pfc::string_formatter name;

		typedef pfc::array_t<t_uint8> t_buffer;
		t_buffer buffer;

		while (e.next(ptr))
		{
			name.reset();

			ptr->get_name(name);

			GUID guid = ptr->get_guid();

			info.set_name(name);

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

static service_factory_single_t<feature_scanner_dsp> g_feature_scanner_factory;
