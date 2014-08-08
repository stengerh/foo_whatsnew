#include "stdafx.h"
#include "FeatureLogViewer.h"
#include "feature.h"

DECLARE_COMPONENT_VERSION(
	"Feature Watcher",
	"1.1.1",
	"Watches available features and reports changes.\n"
	"\n"
	"Copyright (C) 2014 Holger Stenger\n"
	"- https://github.com/stengerh/foo_whatsnew\n"
	"\n"
	"RadpidJSON\n"
	"Copyright (C) 2011-2014 Milo Yip\n"
	"- https://github.com/miloyip/rapidjson/\n"
	"\n"
	"Base64 encoder and decoder\n"
	"Copyright (C) 2004-2008 Ren\xc3\xa9 Nyffenegger\n"
	"- http://www.adp-gmbh.ch/cpp/common/base64.html\n"
	"\n"
	"Some icons by Yusuke Kamiyamane. Licensed under a Creative Commons Attribution 3.0 License.\n"
	"- http://p.yusukekamiyamane.com/\n"
	"- http://creativecommons.org/licenses/by/3.0/\n"
);

static void g_show_feature_log();

static CIconHandle CreateNotificationIcon(int cx, int cy)
{
		CIconHandle icon;
#if 1
		CImageList iml;
		iml.Create(cx, cy, ILC_COLOR32 | ILC_MASK, 2, 2);

		CIcon baseIcon = static_api_ptr_t<ui_control>()->load_main_icon(cx, cy);
		iml.AddIcon(baseIcon);

		CIcon overlayIcon;
		overlayIcon.LoadIcon(IDI_Overlay);
		iml.AddIcon(overlayIcon);

		CImageList iml2;
		if (iml2.Merge(iml, 0, iml, 1, 0, 0))
		{
			icon = iml2.GetIcon(0);
		}
#else
		icon.LoadIcon(IDI_Added);
#endif

		return icon;
}

template <class T>
class CTaskbarNotifyIconImpl
{
	UINT WM_CALLBACKNI;
	UINT WM_TASKBARCREATED;
	UINT m_id;
	CIconHandle m_icon;
	bool m_icon_installed;

public:
	CTaskbarNotifyIconImpl(UINT WM_CALLBACKNI)
	{
		this->WM_CALLBACKNI = WM_CALLBACKNI;
		WM_TASKBARCREATED = RegisterWindowMessage(_T("TaskbarCreated"));
		m_id = 0;
		m_icon_installed = false;
	}

	BEGIN_MSG_MAP(CTaskbarNotifyIconImpl<T>)
		MESSAGE_HANDLER(WM_CALLBACKNI, OnCallbackNI)
		MESSAGE_HANDLER(WM_TASKBARCREATED, OnTaskbarCreated)
	END_MSG_MAP()

	LRESULT OnCallbackNI(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		T *pT = static_cast<T *>(this);;
		ATLASSERT(::IsWindow(pT->m_hWnd));

		switch (lParam)
		{
		case WM_LBUTTONDBLCLK:
			pT->OnLButtonDblClkNI(wParam);
			break;
		case WM_CONTEXTMENU:
			pT->OnContextMenuNI(wParam);
			break;
		case NIN_BALLOONUSERCLICK:
			pT->OnBalloonUserClickNI(wParam);
			break;
		}

		return 0;
	}

	LRESULT OnTaskbarCreated(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		return 0;
	}

	void AddNotifyIcon(HICON hIcon, UINT id, const char *tip, const char *info, const char *info_title)
	{
		if (m_icon_installed) RemoveNotifyIcon();

		T *pT = static_cast<T *>(this);;
		ATLASSERT(::IsWindow(pT->m_hWnd));

		m_icon = hIcon;
		m_id = id;

		NOTIFYICONDATA nid;
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = pT->m_hWnd;
		nid.uID = m_id;
		nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;

		nid.hIcon = hIcon;

		nid.uCallbackMessage = WM_CALLBACKNI;

#ifdef _UNICODE
		pfc::stringcvt::convert_utf8_to_wide(nid.szTip, tabsize(nid.szTip), tip, pfc::infinite_size);
#else
		pfc::stringcvt::convert_utf8_to_ansi(nid.szTip, tabsize(nid.szTip), tip, pfc::infinite_size);
#endif

#ifdef _UNICODE
		pfc::stringcvt::convert_utf8_to_wide(nid.szInfo, tabsize(nid.szInfo), info, pfc::infinite_size);
		pfc::stringcvt::convert_utf8_to_wide(nid.szInfoTitle, tabsize(nid.szInfoTitle), info_title, pfc::infinite_size);
#else
		pfc::stringcvt::convert_utf8_to_ansi(nid.szInfo, tabsize(nid.szInfo), info, pfc::infinite_size);
		pfc::stringcvt::convert_utf8_to_ansi(nid.szInfoTitle, tabsize(nid.szInfoTitle), info_title, pfc::infinite_size);
#endif
		nid.uTimeout = 15000;
		nid.dwInfoFlags = NIIF_INFO;

		m_icon_installed = Shell_NotifyIcon(NIM_ADD, &nid) ? true : false;

		nid.uFlags = 0;
		nid.uVersion = NOTIFYICON_VERSION;

		Shell_NotifyIcon(NIM_SETVERSION, &nid);
	}

	void RemoveNotifyIcon()
	{
		if (!m_icon_installed)
			return;

		T *pT = static_cast<T *>(this);;
		ATLASSERT(::IsWindow(pT->m_hWnd));

		NOTIFYICONDATA nid = {sizeof(NOTIFYICONDATA), pT->m_hWnd, m_id, 0, WM_CALLBACKNI};

		Shell_NotifyIcon(NIM_DELETE, &nid);

		m_icon.DestroyIcon();

		m_icon_installed = false;
	}

	// Overridables
	void OnLButtonDblClkNI(UINT id)
	{
	}

	void OnContextMenuNI(UINT id)
	{
	}

	void OnBalloonUserClickNI(UINT id)
	{
	}
};

class CFeatureChangeNotificationWindow :
	public CWindowImpl<CFeatureChangeNotificationWindow>,
	public CTaskbarNotifyIconImpl<CFeatureChangeNotificationWindow>
{
public:
	DECLARE_WND_CLASS(_T("foo_whatsnew notification window"));

	BEGIN_MSG_MAP(CFeatureChangeNotificationWindow)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		CHAIN_MSG_MAP(CTaskbarNotifyIconImpl<CFeatureChangeNotificationWindow>)
	END_MSG_MAP()

	virtual void OnFinalMessage(HWND)
	{
		delete this;
	}

	CFeatureChangeNotificationWindow() : CTaskbarNotifyIconImpl<CFeatureChangeNotificationWindow>(WM_USER)
	{
	}

	enum {
		ID_NOTIFYICON = 1,
	};

	LRESULT OnCreate(LPCREATESTRUCT pcs)
	{
		const int cxSmIcon = ::GetSystemMetrics(SM_CXSMICON);
		const int cySmIcon = ::GetSystemMetrics(SM_CYSMICON);

		pfc::string_formatter title;
		title << "What's new in ";
		static_api_ptr_t<core_version_info> api;
		service_ptr_t<core_version_info_v2> api_v2;
		if (api->service_query_t(api_v2))
			title << api_v2->get_name();
		else
			title << "foobar2000";
		title << "?";

		AddNotifyIcon(
			CreateNotificationIcon(cxSmIcon, cySmIcon),
			ID_NOTIFYICON,
			"Features added or removed",
			"Features were added or removed. Click here to view the details.",
			title);

		return 0;
	}

	void OnDestroy()
	{
		RemoveNotifyIcon();
	}

	void OnLButtonDblClkNI(UINT id)
	{
		if (id == ID_NOTIFYICON)
		{
			g_show_feature_log();
		}
	}

	void OnBalloonUserClickNI(UINT id)
	{
		if (id == ID_NOTIFYICON)
		{
			g_show_feature_log();
		}
	}
};

static CWindow g_log_window, g_notify_window;

static void g_show_feature_log()
{
	if (g_notify_window.IsWindow())
	{
		g_notify_window.DestroyWindow();
	}

	if (!g_log_window.IsWindow())
	{
		CFeatureLogViewer *ptr = new CFeatureLogViewer();
		g_log_window = ptr->Create(core_api::get_main_window());
		ptr->SetIcon(CreateNotificationIcon(::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON)), FALSE);
		ptr->ShowWindow(SW_SHOW);
	}
	::SetForegroundWindow(g_log_window);
}

class whatsnew_commands : public mainmenu_commands
{
public:
	//! Retrieves number of implemented commands. Index parameter of other methods must be in 0....command_count-1 range.
	t_uint32 get_command_count()
	{
		return 1;
	}

	//! Retrieves GUID of specified command.
	GUID get_command(t_uint32 p_index)
	{
		if (p_index == 0)
		{
			// {86C82997-BD8E-4133-B09D-14304949E499}
			static const GUID guid = 
			{ 0x86c82997, 0xbd8e, 0x4133, { 0xb0, 0x9d, 0x14, 0x30, 0x49, 0x49, 0xe4, 0x99 } };
			
			return guid;
		}
		else throw pfc::exception_bug_check("Command index out of range");
	}

	//! Retrieves name of item, for list of commands to assign keyboard shortcuts to etc.
	void get_name(t_uint32 p_index,pfc::string_base & p_out)
	{
		if (p_index == 0)
		{
			p_out = "What's New?";
		}
		else throw pfc::exception_bug_check("Command index out of range");
	}

	//! Retrieves item's description for statusbar etc.
	bool get_description(t_uint32 p_index,pfc::string_base & p_out)
	{
		if (p_index == 0)
		{
			p_out = "Shows list of features that were recently added or removed.";
			return true;
		}
		else throw pfc::exception_bug_check("Command index out of range");
	}

	//! Retrieves GUID of owning menu group.
	GUID get_parent()
	{
		return mainmenu_groups::help;
	}

	//! Retrieves sorting priority of the command; the lower the number, the upper in the menu your commands will appear. Third party components should use sorting_priority_base and up (values below are reserved for internal use). In case of equal priority, order is undefined.
	t_uint32 get_sort_priority() {return sort_priority_dontcare;}

	//! Executes the command. p_callback parameter is reserved for future use and should be ignored / set to null pointer.
	void execute(t_uint32 p_index,service_ptr_t<service_base> p_callback)
	{
		if (p_index == 0)
		{
			// TODO
			g_show_feature_log();
		}
		else throw pfc::exception_bug_check("Command index out of range");
	}
};

static mainmenu_commands_factory_t<whatsnew_commands> g_mainmenu_commands_factory;

class detect_changes_callback : public feature_log::enum_items_callback
{
	t_filetimestamp m_session_timestamp;
	bool m_changes;

public:
	detect_changes_callback()
	{
		m_session_timestamp = static_api_ptr_t<feature_log>()->get_session_timestamp();
		m_changes = false;
	}

	bool on_feature_log_item(const feature_log_item &p_item)
	{
		m_changes = (p_item.get_timestamp() == m_session_timestamp);
		return false;
	}

	bool has_changes() {return m_changes;}
};

static void g_check_for_changes()
{
	detect_changes_callback callback;
	static_api_ptr_t<feature_log>()->enum_items(callback);

	if (callback.has_changes())
	{
		//g_show_feature_log();
		CFeatureChangeNotificationWindow *ptr = new CFeatureChangeNotificationWindow();
		g_notify_window = ptr->Create(NULL, 0, _T("foo_whatsnew NIW"), WS_POPUP);
	}
}

class main_thread_callback_whatsnew : public main_thread_callback
{
	unsigned m_hop_count;

public:
	main_thread_callback_whatsnew(unsigned p_hop_count)
	{
		m_hop_count = p_hop_count;
	}

	virtual void callback_run()
	{
		if (m_hop_count > 0)
		{
			--m_hop_count;
			static_api_ptr_t<main_thread_callback_manager>()->add_callback(this);
		}
		else
		{
			g_check_for_changes();
		}
	}
};

class initquit_whatsnew : public initquit
{
public:
	void on_init()
	{
		// Do not run feature check in quite mode.
		if (core_api::get_main_window() != NULL)
		{
			service_ptr_t<main_thread_callback_whatsnew> callback = new service_impl_t<main_thread_callback_whatsnew>(2);
			callback->callback_run();
		}
	}

	void on_quit()
	{
		if (g_notify_window.IsWindow())
			g_notify_window.DestroyWindow();
	}
};

static initquit_factory_t<initquit_whatsnew> g_initguit_factory;
