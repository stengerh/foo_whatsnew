#pragma once

void save_feature_list_to_json_string(const pfc::list_base_const_t<feature_handle_ptr> &p_list, pfc::string_base & p_json);
void save_feature_list_to_json_file(const pfc::list_base_const_t<feature_handle_ptr> &p_list, const service_ptr_t<file> &p_file, abort_callback &p_abort);

bool load_feature_list_from_json_string(pfc::list_base_t<feature_handle_ptr> &p_list, const char * p_json);
bool load_feature_list_from_json_file(pfc::list_base_t<feature_handle_ptr> &p_list, const service_ptr_t<file> &p_file, abort_callback &p_abort);
