#include "stdafx.h"

#include <rapidjson/reader.h>

#include "feature.h"
#include "feature_impl.h"
#include "feature_log_json.h"
#include "base64.h"

using namespace rapidjson;

class FeatureListReaderHandler : public BaseReaderHandler<>
{
public:
	FeatureListReaderHandler(pfc::list_base_t<feature_handle_ptr> &p_list)
		: m_state(state_root), m_items(0), m_list(p_list)
	{
	}

	bool StartObject()
	{
		switch (m_state)
		{
		case state_list:
			m_state = state_item;
			m_items = item_flag_name | item_flag_kind | item_flag_key | item_flag_component;
			m_info.reset();
			return true;
		default:
			return false;
		}
	}

	bool EndObject(SizeType)
	{
		switch (m_state)
		{
		case state_item:
			if ((m_items & REQUIRED_ITEMS) == REQUIRED_ITEMS)
			{
				m_list.add_item(m_mgr->create_handle(m_info));
				m_state = state_list;
				return true;
			}
			else
			{
				return false;
			}
		default:
			return false;
		}
	}

	bool StartArray()
	{
		switch (m_state)
		{
		case state_root:
			m_state = state_list;
			return true;
		default:
			return false;
		}
	}

	bool EndArray(SizeType)
	{
		switch (m_state)
		{
		case state_list:
			m_state = state_done;
			return true;
		default:
			return false;
		}
	}

	bool String(const char* str, SizeType length, bool)
	{
		switch (m_state)
		{
		case state_item:
			if (0 == pfc::strcmp_ex(str, length, "name", pfc::infinite_size))
			{
				m_state = state_value_name;
				return true;
			}
			else if (0 == pfc::strcmp_ex(str, length, "kind", pfc::infinite_size))
			{
				m_state = state_value_kind;
				return true;
			}
			else if (0 == pfc::strcmp_ex(str, length, "key", pfc::infinite_size))
			{
				m_state = state_value_key;
				return true;
			}
			else if (0 == pfc::strcmp_ex(str, length, "component", pfc::infinite_size))
			{
				m_state = state_value_component;
				return true;
			}
			else
			{
				return false;
			}
		case state_value_name:
			m_info.set_name(str, length);
			m_items |= item_flag_name;
			m_state = state_item;
			return true;
		case state_value_kind:
			m_info.set_kind_guid(pfc::GUID_from_text(pfc::string8(str, length)));
			m_items |= item_flag_kind;
			m_state = state_item;
			return true;
		case state_value_key:
			{
				std::string key_base64;
				key_base64.append(&str[0], &str[length]);
				std::string key = base64_decode(key_base64);
				m_info.set_key((const t_uint8 *) key.c_str(), key.length());
			}
			m_items |= item_flag_key;
			m_state = state_item;
			return true;
		case state_value_component:
			m_info.set_component_name(str, length);
			m_items |= item_flag_component;
			m_state = state_item;
			return true;
		default:
			return false;
		}
	}

	bool Default()
	{
		return false;
	}

private:
	enum state_t {
		state_root,
		state_list,
		state_item,
		state_value_name,
		state_value_kind,
		state_value_key,
		state_value_component,
		state_done,
	};

	enum item_flag_t {
		item_flag_none = 0,
		item_flag_name = 1 << 0,
		item_flag_kind = 1 << 1,
		item_flag_key = 1 << 2,
		item_flag_component = 1 << 3,
	};

	static const int REQUIRED_ITEMS = item_flag_name | item_flag_kind | item_flag_key;

	state_t m_state;
	int m_items;
	pfc::list_base_t<feature_handle_ptr> &m_list;
	feature_info_impl m_info;
	static_api_ptr_t<feature_manager> m_mgr;
};

bool load_feature_list_from_json_string(pfc::list_base_t<feature_handle_ptr> &p_list, const char * p_json)
{
	FeatureListReaderHandler handler(p_list);
	Reader reader;
	StringStream stream(p_json);

	ParseResult result = reader.Parse(stream, handler);
	return !result.IsError();
}

bool load_feature_list_from_json_file(pfc::list_base_t<feature_handle_ptr> &p_list, const service_ptr_t<file> &p_file, abort_callback &p_abort)
{
	t_filesize size = p_file->get_size(p_abort);

	pfc::string8 json;
	{
		pfc::string_buffer buffer(json, size);
		p_file->read_object(buffer, size, p_abort);
	}

	return load_feature_list_from_json_string(p_list, json);
}
