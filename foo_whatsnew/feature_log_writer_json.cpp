#include "stdafx.h"

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "feature.h"
#include "feature_log_json.h"

#include "base64.h"

using namespace rapidjson;

void save_feature_list_to_json_string(const pfc::list_base_const_t<feature_handle_ptr> &p_list, pfc::string_base & p_json)
{
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);

	writer.StartArray();

	for (t_size index = 0; index < p_list.get_count(); ++index)
	{
		const feature_info * info = p_list[index]->get_info();

		writer.StartObject();

		writer.String("name");
		writer.String(info->get_name());

		writer.String("kind");
		writer.String(pfc::print_guid(info->get_kind_guid()));

		writer.String("key");
		std::string key_base64 = base64_encode(info->get_key(), info->get_key_size());
		writer.String(key_base64.c_str());

		if (info->has_component_name())
		{
			writer.String("component");
			writer.String(info->get_component_name());
		}

		writer.EndObject();
	}

	writer.EndArray(p_list.get_count());

	p_json.set_string(buffer.GetString());
}

void save_feature_list_to_json_file(const pfc::list_base_const_t<feature_handle_ptr> &p_list, const service_ptr_t<file> &p_file, abort_callback &p_abort)
{
	pfc::string8 json;

	save_feature_list_to_json_string(p_list, json);

	p_file->seek(0, p_abort);
	p_file->write(json.get_ptr(), json.get_length(), p_abort);
}
