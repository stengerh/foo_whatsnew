#include "stdafx.h"
#include "feature.h"

namespace feature_kinds
{
	static const GUID file_type = { 0xc07a18d2, 0x4800, 0x4720, { 0xab, 0xc8, 0xee, 0x85, 0x8c, 0x57, 0x43, 0xa3 } };
}


class feature_kind_file_type : public feature_kind
{
public:
	virtual GUID get_guid()
	{
		return feature_kinds::file_type;
	}

	virtual const char *get_name()
	{
		return "Audio File Type";
	}

	virtual bool get_description(pfc::string_base &p_out)
	{
		return false;
	}
};

static service_factory_single_t<feature_kind_file_type> g_file_type_factory;

class feature_scanner_file_type : public feature_scanner
{
public:
	virtual void scan(enum_feature_info_callback &p_callback)
	{
		service_enum_t<input_file_type> e;
		service_ptr_t<input_file_type> ptr;

		feature_info_impl info;
		info.set_kind_guid(feature_kinds::file_type);

		typedef pfc::array_t<t_uint8> t_buffer;
		t_buffer buffer;

		pfc::string8_fastalloc name, mask;

		while (e.next(ptr))
		{
			const unsigned count = ptr->get_count();

			for (unsigned n = 0; n < count; ++n)
			{
				name.reset();
				mask.reset();

				if (ptr->get_name(n, name))
				{
					if (ptr->get_mask(n, mask))
					{
						name << " (" << string_lower(mask) << ")";
					}
				}
				else if (ptr->get_mask(n, mask))
				{
					name << "Audio Files (" << string_lower(mask) << ")";
				}

				if (name.is_empty()) continue;

				info.set_name(name);

				string_lower key(name);
				info.set_key((const t_uint8*)key.get_ptr(), key.length());

				if (!p_callback.on_feature_info(info)) break;
			}
		}
	}
};

static service_factory_single_t<feature_scanner_file_type> g_feature_scanner_factory;
