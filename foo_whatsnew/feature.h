#pragma once

class feature_info;
class feature_handle;
class feature_kind;
class feature_scanner;
class feature_manager;
class feature_log_item;
class feature_log;

typedef service_ptr_t<feature_handle> feature_handle_ptr;
typedef service_ptr_t<feature_kind> feature_kind_ptr;

class feature_info
{
public:
	virtual const char *get_name() const = 0;
	virtual void set_name(const char *p_name, t_size p_name_length = pfc::infinite_size) = 0;

	virtual GUID get_kind_guid() const = 0;
	virtual void set_kind_guid(const GUID &p_guid) = 0;

	virtual const t_uint8 *get_key() const = 0;
	virtual t_size get_key_size() const = 0;
	virtual void set_key(const t_uint8* p_data, t_size p_data_size) = 0;

	virtual void copy(const feature_info &p_source) {set_name(p_source.get_name()); set_kind_guid(p_source.get_kind_guid()); set_key(p_source.get_key(), p_source.get_key_size());}

	void reset() {set_name(""); set_kind_guid(pfc::guid_null); set_key(0, 0);}

	static int compare_raw_data(const feature_info &p_item1, const feature_info &p_item2);

protected:
	feature_info() {}
	~feature_info() {}
};

class enum_feature_info_callback
{
public:
	virtual bool on_feature_info(const feature_info &p_info) = 0;

protected:
	enum_feature_info_callback() {}
	~enum_feature_info_callback() {}
};

class feature_handle : public service_base
{
public:
	virtual const feature_info *get_info() = 0;

	FB2K_MAKE_SERVICE_INTERFACE(feature_handle, service_base);
};

class feature_kind : public service_base
{
public:
	virtual GUID get_guid() = 0;
	virtual const char *get_name() = 0;
	virtual bool get_description(pfc::string_base &p_out) = 0;

	inline void get_name(pfc::string_base &p_out) {p_out = get_name();}

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(feature_kind);
};

class feature_scanner : public service_base
{
public:
	virtual void scan(enum_feature_info_callback &p_callback) = 0;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(feature_scanner);
};

class feature_manager : public service_base
{
public:
	virtual feature_handle_ptr create_handle(const feature_info &p_info) = 0;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(feature_manager);
};

class feature_log_item
{
public:
	enum t_change_kind
	{
		change_invalid,
		change_added,
		change_removed,
		change_modified,
	};

	//! Get time of the scan.
	virtual t_filetimestamp get_timestamp() const = 0;
	virtual void set_timestamp(t_filetimestamp p_timestamp) = 0;

	virtual t_change_kind get_change_kind() const = 0;
	virtual void set_change_kind(t_change_kind p_kind) = 0;

	virtual const char * get_change_description() const = 0;
	virtual void set_change_description(const char *p_text, t_size p_text_length = pfc::infinite_size) = 0;

	virtual feature_handle_ptr get_feature() const = 0;
	virtual void set_feature(feature_handle_ptr p_feature) = 0;

	virtual void copy(const feature_log_item &p_source)
	{
		set_timestamp(p_source.get_timestamp());
		set_change_kind(p_source.get_change_kind());
		set_change_description(p_source.get_change_description());
		set_feature(p_source.get_feature());
	}

	void reset()
	{
		set_timestamp(filetimestamp_invalid);
		set_change_kind(change_invalid);
		set_change_description("");
		set_feature(0);
	}
protected:
	feature_log_item() {}
	~feature_log_item() {}
};

class feature_log : public service_base
{
public:
	class enum_items_callback
	{
	public:
		// Return false to abort enumeration.
		virtual bool on_feature_log_item(const feature_log_item &p_entry) = 0;

	protected:
		enum_items_callback() {}
		~enum_items_callback() {}
	};

	virtual t_filetimestamp get_session_timestamp() = 0;

	//! Get all items.
	//! Items are ordered by timestamp; newest items are first.
	virtual void enum_items(enum_items_callback &p_callback) = 0;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(feature_log);
};

// {19728E66-6523-439f-8D1F-2B1C0809F067}
FOOGUIDDECL const GUID feature_kind::class_guid = { 0x19728e66, 0x6523, 0x439f, { 0x8d, 0x1f, 0x2b, 0x1c, 0x8, 0x9, 0xf0, 0x67 } };

// {C385C7CA-E984-4fb1-951B-8C68DDA2BD60}
FOOGUIDDECL const GUID feature_scanner::class_guid = { 0xc385c7ca, 0xe984, 0x4fb1, { 0x95, 0x1b, 0x8c, 0x68, 0xdd, 0xa2, 0xbd, 0x60 } };

// {F90B3B13-3773-43e3-A5ED-30B4A358DD7A}
FOOGUIDDECL const GUID feature_log::class_guid = { 0xf90b3b13, 0x3773, 0x43e3, { 0xa5, 0xed, 0x30, 0xb4, 0xa3, 0x58, 0xdd, 0x7a } };

// {5BCFE49E-8475-47cb-A854-BCA520FCB781}
FOOGUIDDECL const GUID feature_manager::class_guid = { 0x5bcfe49e, 0x8475, 0x47cb, { 0xa8, 0x54, 0xbc, 0xa5, 0x20, 0xfc, 0xb7, 0x81 } };

// {75B34254-733F-4f00-8938-03A0F7B1297E}
FOOGUIDDECL const GUID feature_handle::class_guid = { 0x75b34254, 0x733f, 0x4f00, { 0x89, 0x38, 0x3, 0xa0, 0xf7, 0xb1, 0x29, 0x7e } };

#include "feature_impl.h"
