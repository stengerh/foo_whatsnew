#include "stdafx.h"
#include "FeatureLogViewer.h"
#include "tools.h"

using pfc::stringcvt::string_os_from_utf8;

// {D3B5BDF8-96EE-4853-988B-A4959EF271FC}
static const GUID guid_cfg_window_size = { 0xd3b5bdf8, 0x96ee, 0x4853, { 0x98, 0x8b, 0xa4, 0x95, 0x9e, 0xf2, 0x71, 0xfc } };
static cfg_window_size cfg_window_size_log_viewer(guid_cfg_window_size);

struct t_column_data
{
	int width[CFeatureLogViewer::column_count];
	int order[CFeatureLogViewer::column_count];
};

static const t_column_data g_default_column_data =
{
	{
		400,
		0,
		80,
		0,
#ifdef EXTRACT_COMPONENT_NAME
		0,
#endif
	},
	{
		CFeatureLogViewer::column_feature_name,
		CFeatureLogViewer::column_feature_kind,
		CFeatureLogViewer::column_change_kind,
		CFeatureLogViewer::column_timestamp,
#ifdef EXTRACT_COMPONENT_NAME
		CFeatureLogViewer::column_component_name,
#endif
	},
};

// {A5FBE375-1A9E-497F-9B93-F6C351FB9CE0}
static const GUID guid_cfg_column_data = { 0xa5fbe375, 0x1a9e, 0x497f, { 0x9b, 0x93, 0xf6, 0xc3, 0x51, 0xfb, 0x9c, 0xe0 } };
static cfg_struct_t<t_column_data> cfg_column_data(guid_cfg_column_data, g_default_column_data);

static bool g_find_by_guid(const GUID &p_guid, feature_kind_ptr &p_out)
{
	service_enum_t<feature_kind> e;
	service_ptr_t<feature_kind> ptr;
	while (e.next(ptr))
	{
		if (ptr->get_guid() == p_guid)
		{
			p_out = ptr;
			return true;
		}
	}
	return false;
}

static CImageList CreateStateImageList(int cx, int cy)
{
	CImageList iml;
	iml.Create(16, 16, ILC_COLOR8 | ILC_MASK, 0, 4);

	CBitmap bmp;
	bmp.Attach((HBITMAP) ::LoadImage(core_api::get_my_instance(), MAKEINTRESOURCE(IDB_Icons8), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR));

	iml.Add(bmp, 0xff00ff);

	return iml;
}

class enum_session_items_callback : public feature_log::enum_items_callback
{
	t_filetimestamp m_session_timestamp;
	feature_log_item_list &m_list;

public:
	enum_session_items_callback(feature_log_item_list &p_list) : m_list(p_list)
	{
		m_session_timestamp = static_api_ptr_t<feature_log>()->get_session_timestamp();
	}

	bool on_feature_log_item(const feature_log_item &p_item)
	{
		if (p_item.get_timestamp() != m_session_timestamp)
			return false;

		m_list.add_item(p_item);

		return true;
	}
};

void CFeatureLogViewer::AddLogItem(const feature_log_item &p_item)
{
	int row = -1;

	const feature_info *info = p_item.get_feature()->get_info();

	// Use loop+switch to insert columns in numerical order.
	for (int column = 0; column < column_count; ++column)
	{
		pfc::string8 text;

		switch (column)
		{
		case column_feature_name:
			{
				text = info->get_name();
			}
			break;
		case column_feature_kind:
			{
				feature_kind_ptr kind;
				if (g_find_by_guid(info->get_kind_guid(), kind))
				{
					kind->get_name(text);
				}
			}
			break;
		case column_change_kind:
			{
				switch (p_item.get_change_kind())
				{
				case feature_log_item::change_added:
					text = "Added";
					break;
				case feature_log_item::change_removed:
					text = "Removed";
					break;
				case feature_log_item::change_modified:
					text = "Modified";
					break;
				}
			}
			break;
		case column_timestamp:
			text = pfc::format_timestamp(p_item.get_timestamp(), true);
			break;
#ifdef EXTRACT_COMPONENT_NAME
		case column_component_name:
			if (info->has_component_name())
			{
				text = info->get_component_name();
			}
			break;
#endif
		}

		if (column == 0)
		{
			string_os_from_utf8 ostemp(text);

			LVITEM item;
			item.mask = LVIF_TEXT | LVIF_STATE | LVIF_GROUPID;
			item.iItem = m_log.GetItemCount();
			item.iSubItem = 0;
			item.stateMask = LVIS_STATEIMAGEMASK;
			item.state = INDEXTOSTATEIMAGEMASK(p_item.get_change_kind());
			item.pszText = const_cast<TCHAR*>(ostemp.get_ptr());

			m_feature_kind_groupids.query(info->get_kind_guid(), item.iGroupId);

			row = m_log.InsertItem(&item);
		}
		else
		{
			m_log.SetItemText(row, column, string_os_from_utf8(text));
		}
	}
}

static const dialog_resize_helper::param g_resize_helper_table[] =
{
	{IDC_Log, dialog_resize_helper::XY_SIZE},
	{IDCANCEL, dialog_resize_helper::XY_MOVE},
};

CFeatureLogViewer::CFeatureLogViewer() : m_resize_helper(g_resize_helper_table, tabsize(g_resize_helper_table), 0, 0, 0, 0)
{
}

BOOL CFeatureLogViewer::OnInitDialog(HWND hWndFocus, LPARAM lInitParam)
{
	ModifyStyleEx(0, WS_EX_DLGMODALFRAME);

	m_resize_helper.add_sizegrip();

	SetWindowText(string_os_from_utf8(pfc::string_formatter() << "What's New? - " << core_version_info::g_get_version_string()));

	cfg_window_size_log_viewer.on_window_creation(*this);

	m_log.Attach(GetDlgItem(IDC_Log));
	m_log.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP);

	m_log.SetImageList(CreateStateImageList(16, 16), LVSIL_STATE);

	for (int column = 0; column < column_count; ++column)
	{
		LPCTSTR heading = TEXT("TODO");
		int format = LVCFMT_LEFT;
		int width = cfg_column_data.get_value().width[column];

		switch (column)
		{
		case column_feature_name:
			heading = TEXT("Name");
			break;
		case column_feature_kind:
			heading = TEXT("Type");
			break;
		case column_change_kind:
			heading = TEXT("Change");
			break;
		case column_timestamp:
			heading = TEXT("Date");
			break;
#ifdef EXTRACT_COMPONENT_NAME
		case column_component_name:
			heading = TEXT("Component");
			break;
#endif
		}

		m_log.InsertColumn(column, heading, format, width);
	}

	BOOL result = m_log.SetColumnOrderArray(column_count, cfg_column_data.get_value().order);

	m_log.EnableGroupView(TRUE);

	CreateGroups();

	static_api_ptr_t<feature_log>()->enum_items(enum_session_items_callback(m_items));

	m_items.sort_canonical();

	for (t_size n = 0; n < m_items.get_count(); ++n)
	{
		AddLogItem(m_items[n]);
	}

	m_log.SetSelectedColumn(0);

	static_api_ptr_t<message_loop>()->add_message_filter(this);

	return FALSE;
}

void CFeatureLogViewer::OnDestroy()
{
	static_api_ptr_t<message_loop>()->remove_message_filter(this);

	cfg_window_size_log_viewer.on_window_destruction(*this);

	for (int n = 0; n < column_count; ++n)
	{
		cfg_column_data.get_value().width[n] = m_log.GetColumnWidth(n);
	}

	BOOL result = m_log.GetColumnOrderArray(column_count, cfg_column_data.get_value().order);
}

bool CFeatureLogViewer::pretranslate_message(MSG *msg)
{
	if (IsDialogMessage(msg))
		return true;
	return false;
}

void CFeatureLogViewer::OnClose()
{
	DestroyWindow();
}

void CFeatureLogViewer::OnCancel(UINT code, int id, HWND wnd)
{
	DestroyWindow();
}

void CFeatureLogViewer::CreateGroups()
{
	LVGROUP group;
	group.cbSize = sizeof(LVGROUP);
	group.mask = LVGF_GROUPID | LVGF_HEADER;
	group.iGroupId = 0;

	service_instance_array_t<feature_kind> feature_kinds;
	pfc::array_t<t_size> order;
	order.set_size(feature_kinds.get_size());
	order_helper::g_fill(order);

	feature_tools::sort_by_name_get_permutation_t(feature_kinds, order.get_size(), order.get_ptr());

	for (t_size n = 0; n < feature_kinds.get_size(); ++n)
	{
		group.iGroupId += 1;
		string_os_from_utf8 osname(feature_kinds[order[n]]->get_name());
		group.pszHeader = const_cast<LPTSTR>(osname.get_ptr());

		m_log.InsertGroup(-1, &group);

		m_feature_kind_groupids.set(feature_kinds[order[n]]->get_guid(), group.iGroupId);
	}

#if 0
	group.iGroupId = feature_log_item::change_added;
	group.pszHeader = _T("Added");
	m_log.InsertGroup(0, &group);

	group.iGroupId = feature_log_item::change_removed;
	group.pszHeader = _T("Removed");
	m_log.InsertGroup(1, &group);

	group.iGroupId = feature_log_item::change_modified;
	group.pszHeader = _T("Modified");
	m_log.InsertGroup(2, &group);
#endif
}
