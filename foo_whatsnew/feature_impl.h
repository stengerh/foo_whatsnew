#pragma once

class feature_info_impl : public feature_info
{
	pfc::string_simple m_name;
	bool m_component_name_valid;
	pfc::string_simple m_component_name;
	GUID m_kind_guid;
	pfc::array_t<t_uint8> m_data;

public:
	feature_info_impl() {reset();}
	feature_info_impl(const feature_info &p_source) {copy(p_source);}
	feature_info_impl(const feature_info_impl &p_source) {copy(p_source);}

	virtual const char *get_name() const {return m_name;}
	virtual void set_name(const char *p_name, t_size p_name_length = pfc::infinite_size) {m_name.set_string(p_name, p_name_length);}

#ifdef EXTRACT_COMPONENT_NAME
	virtual bool has_component_name() const {return m_component_name_valid;}
	virtual const char *get_component_name() const {return m_component_name;}
	virtual void clear_component_name() {m_component_name_valid = false; m_component_name.set_string("", 0);}
	virtual void set_component_name(const char *p_name, t_size p_name_length = pfc::infinite_size) {m_component_name_valid = true; m_component_name.set_string(p_name, p_name_length);}
#endif

	virtual GUID get_kind_guid() const {return m_kind_guid;}
	virtual void set_kind_guid(const GUID &p_guid) {m_kind_guid = p_guid;}

	virtual const t_uint8 *get_key() const {return m_data.get_ptr();}
	virtual t_size get_key_size() const {return m_data.get_size();}
	virtual void set_key(const t_uint8* p_data, t_size p_data_size) {m_data.set_data_fromptr(p_data, p_data_size);}
};

class feature_log_item_impl : public feature_log_item
{
	t_filetimestamp m_timestamp;
	t_change_kind m_change_kind;
	pfc::string8 m_change_description;
	feature_handle_ptr m_feature;

public:
	feature_log_item_impl() {reset();}
	feature_log_item_impl(const feature_log_item &p_source) {copy(p_source);}
	feature_log_item_impl(const feature_log_item_impl &p_source) {copy(p_source);}

	virtual t_filetimestamp get_timestamp() const {return m_timestamp;}
	virtual void set_timestamp(t_filetimestamp p_timestamp) {m_timestamp = p_timestamp;}

	virtual t_change_kind get_change_kind() const {return m_change_kind;}
	virtual void set_change_kind(t_change_kind p_kind) {m_change_kind = p_kind;}

	virtual const char * get_change_description() const {return m_change_description;}
	virtual void set_change_description(const char *p_text, t_size p_text_length = pfc::infinite_size) {m_change_description.set_string(p_text, p_text_length);}

	virtual feature_handle_ptr get_feature() const {return m_feature;}
	virtual void set_feature(feature_handle_ptr p_feature) {m_feature = p_feature;}
};
