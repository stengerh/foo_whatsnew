#include "stdafx.h"
#include "feature.h"
#include "feature_tools.h"
#include "tools.h"

class enum_feature_info_callback_impl : public enum_feature_info_callback
{
	pfc::list_base_t<feature_handle_ptr> &m_out;
	static_api_ptr_t<feature_manager> fm;
public:
	enum_feature_info_callback_impl(pfc::list_base_t<feature_handle_ptr> &p_out) : m_out(p_out) {}

	bool on_feature_info(const feature_info &p_info)
	{
		m_out.add_item(fm->create_handle(p_info));
		return true;
	}
};

class cfg_feature_log : public cfg_var, public feature_log_item_list
{
private:
	void get_data_raw(stream_writer * p_stream,abort_callback & p_abort)
	{
		t_uint32 count = pfc::downcast_guarded<t_uint32>(get_count());
		p_stream->write_lendian_t(count, p_abort);
		for (t_uint32 index = 0; index < count; ++index)
		{
			const feature_log_item &item = get_item_ref(index);
			p_stream->write_lendian_t(item.get_timestamp(), p_abort);
			p_stream->write_lendian_t((t_uint32)item.get_change_kind(), p_abort);
			p_stream->write_string(item.get_change_description(), p_abort);

			feature_handle_ptr handle = item.get_feature();
			const feature_info *info = handle->get_info();
			p_stream->write_string(info->get_name(), p_abort);
			p_stream->write_lendian_t(info->get_kind_guid(), p_abort);
			const t_size key_size = info->get_key_size();
			p_stream->write_lendian_t(key_size, p_abort);
			p_stream->write_object(info->get_key(), key_size, p_abort);
		}
	}

	void set_data_raw(stream_reader * p_stream,t_size p_sizehint,abort_callback & p_abort)
	{
	}
};

namespace feature_handle_tools
{
	void read_from_stream(feature_info &p_info, stream_reader *p_stream, abort_callback &p_abort)
	{
		pfc::string8_fastalloc name;
		p_stream->read_string(name, p_abort);

		GUID kind_guid;
		p_stream->read_lendian_t(kind_guid, p_abort);

		t_uint32 key_size;
		p_stream->read_lendian_t(key_size, p_abort);

		pfc::array_t<t_uint8> key;
		key.set_size(key_size);
		p_stream->read_object(key.get_ptr(), key.get_size(), p_abort);

		p_info.set_name(name);
		p_info.set_kind_guid(kind_guid);
		p_info.set_key(key.get_ptr(), key.get_size());
	}

	void read_from_stream(pfc::list_base_t<feature_handle_ptr> &p_data, stream_reader *p_stream, abort_callback &p_abort)
	{
		p_data.remove_all();

		t_uint32 count;
		p_stream->read_lendian_t(count, p_abort);

		static_api_ptr_t<feature_manager> mgr;

		for (t_uint32 index = 0; index < count; ++index)
		{
			feature_info_impl info;

			read_from_stream(info, p_stream, p_abort);

			p_data.add_item(mgr->create_handle(info));
		}
	}

	void write_to_stream(const feature_info &p_info, stream_writer *p_stream, abort_callback &p_abort)
	{
		p_stream->write_string(p_info.get_name(), p_abort);

		p_stream->write_lendian_t(p_info.get_kind_guid(), p_abort);

		p_stream->write_lendian_t(pfc::downcast_guarded<t_uint32>(p_info.get_key_size()), p_abort);

		p_stream->write_object(p_info.get_key(), p_info.get_key_size(), p_abort);
	}

	void write_to_stream(const pfc::list_base_const_t<feature_handle_ptr> &p_data, stream_writer *p_stream, abort_callback &p_abort)
	{
		t_uint32 count = pfc::downcast_guarded<t_uint32>(p_data.get_count());
		p_stream->write_lendian_t(count, p_abort);

		for (t_uint32 index = 0; index < count; ++index)
		{
			write_to_stream(*p_data[index]->get_info(), p_stream, p_abort);
		}
	}
}

class feature_log_impl : public feature_log
{
	pfc::list_t<feature_log_item_impl> m_changes;
	t_filetimestamp m_session_timestamp;

	void get_present_features(pfc::list_base_t<feature_handle_ptr> &p_out)
	{
		service_enum_t<feature_scanner> e;
		service_ptr_t<feature_scanner> ptr;
		while (e.next(ptr))
		{
			ptr->scan(enum_feature_info_callback_impl(p_out));
		}
	}

	void get_feature_list_path(pfc::string_base &p_out)
	{
		p_out << core_api::get_profile_path() << "\\features.dat";
	}

	bool load_feature_list(pfc::list_base_t<feature_handle_ptr> &p_data)
	{
		pfc::string8 path;
		get_feature_list_path(path);

		try
		{
			abort_callback_impl abort;
			service_ptr_t<file> stream;
			filesystem::g_open_read(stream, path, abort);

			feature_handle_tools::read_from_stream(p_data, stream.get_ptr(), abort);

			return true;
		}
		catch (const exception_io &exc)
		{
			console::formatter() << "Loading feature list from " << path << " failed: " << exc;

			return false;
		}
	}

	void save_feature_list(const pfc::list_base_const_t<feature_handle_ptr> &p_data)
	{
		pfc::string8 path;
		get_feature_list_path(path);

		try
		{
			abort_callback_impl abort;
			service_ptr_t<file> stream;
			filesystem::g_open_write_new(stream, path, abort);

			feature_handle_tools::write_to_stream(p_data, stream.get_ptr(), abort);
		}
		catch (const exception_io &exc)
		{
			console::formatter() << "Saving feature list to " << path << " failed: " << exc;
		}
	}

	static void append_feature_log_entry(pfc::string_base &p_out, const feature_log_item &p_item)
	{
		switch (p_item.get_change_kind())
		{
		case feature_log_item::change_added:
			p_out << "+";
			break;
		case feature_log_item::change_removed:
			p_out << "-";
			break;
		case feature_log_item::change_modified:
			p_out << "*";
			break;
		}

		feature_handle_ptr handle = p_item.get_feature();
		const feature_info *info = handle->get_info();

		{
			service_instance_array_t<feature_kind> kinds;
			for (t_size n = 0; n < kinds.get_size(); ++n)
			{
				if (info->get_kind_guid() == kinds[n]->get_guid())
				{
					p_out << kinds[n]->get_name();
				}
			}
		}

		p_out << ": " << info->get_name() << "\r\n";
	}

	class update_feature_log_callback : public enum_items_callback
	{
		t_filetimestamp m_session_timestamp;
		pfc::list_base_t<feature_log_item_impl> &m_list;

	public:
		update_feature_log_callback(t_filetimestamp p_session_timestamp, pfc::list_base_t<feature_log_item_impl> &p_list)
			: m_session_timestamp(p_session_timestamp), m_list(p_list)
		{
		}

		bool on_feature_log_item(const feature_log_item &p_item)
		{
			if (p_item.get_timestamp() != m_session_timestamp)
				return false;

			m_list.add_item(p_item);

			return true;
		}
	};

	void update_feature_log()
	{
		pfc::string_formatter path;
		path << core_api::get_profile_path() << "\\features.log";

		try
		{
			abort_callback_impl abort;
			service_ptr_t<file> stream;
			try
			{
				filesystem::g_open(stream, path, filesystem::open_mode_write_existing, abort);
				stream->seek_ex(0, file::seek_from_eof, abort);
			}
			catch (exception_io_not_found)
			{
				filesystem::g_open(stream, path, filesystem::open_mode_write_new, abort);
			}

			pfc::string_formatter header;
			header << "[" << pfc::format_timestamp(get_session_timestamp(), true) << " " << core_version_info::g_get_version_string() << "]\r\n";

			feature_log_item_list list;

			enum_items(update_feature_log_callback(get_session_timestamp(), list));

			list.sort_canonical();

			pfc::string_formatter log;

			for (t_size n = 0; n < list.get_count(); ++n)
			{
				append_feature_log_entry(log, list[n]);
			}

			if (stream->get_size(abort) == 0)
				stream->write_string_raw("\r\n", abort);
			stream->write_string_raw(header, abort);
			stream->write_string_raw(log, abort);
		}
		catch (const exception_io &exc)
		{
			console::formatter() << "Updating feature log at " << path << " failed: " << exc;
		}
	}

	void scan()
	{
		m_session_timestamp = filetimestamp_from_system_timer();

		service_list_t<feature_handle> previous;
		bool previous_valid = load_feature_list(previous);

		service_list_t<feature_handle> current;
		get_present_features(current);

		bool has_changes = false;

#if 0
		if (!previous_valid)
		{
			has_changes = true;
		}
		else
#endif
		{
			feature_log_item_impl item;
			item.set_timestamp(m_session_timestamp);

			item.set_change_kind(feature_log_item::change_added);

			for (t_size n = 0; n < current.get_count(); ++n)
			{
				if (!previous.have_item(current[n]))
				{
					item.set_feature(current[n]);
					m_changes.add_item(item);
					has_changes = true;
				}
			}

			item.set_change_kind(feature_log_item::change_removed);

			for (t_size n = 0; n < previous.get_count(); ++n)
			{
				if (!current.have_item(previous[n]))
				{
					item.set_feature(previous[n]);
					m_changes.add_item(item);
					has_changes = true;
				}
			}
		}

		if (has_changes)
		{
			save_feature_list(current);
			update_feature_log();
		}
	}

	//! Get items from current session.
	//! \returns true if calling the method triggered a scan, false if results were cached.
	void ensure_scanned()
	{
		if (m_session_timestamp == filetimestamp_invalid)
		{
			scan();
		}
	}

public:
	feature_log_impl()
	{
		m_session_timestamp = filetimestamp_invalid;
	}

	virtual t_filetimestamp get_session_timestamp()
	{
		ensure_scanned();

		return m_session_timestamp;
	}

	//! Get all items.
	//! Items are ordered by timestamp; newest items are first.
	virtual void enum_items(enum_items_callback &p_callback)
	{
		ensure_scanned();

		for (t_size n = 0; n < m_changes.get_count(); ++n)
		{
			if (!p_callback.on_feature_log_item(m_changes[n])) break;
		}
	}
};

static service_factory_single_t<feature_log_impl> g_feature_log_factory;
