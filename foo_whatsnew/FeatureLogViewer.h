#pragma once

#include "feature.h"
#include "feature_tools.h"
#include "resource.h"

class CFeatureLogViewer :
	public CDialogImpl<CFeatureLogViewer>,
	private message_filter
{
public:
	typedef CFeatureLogViewer thisClass;

	enum {IDD = IDD_FeatureLogViewer};

	enum column_id_t
	{
		column_feature_name,
		column_feature_kind,
		column_change_kind,
		column_timestamp,
#ifdef EXTRACT_COMPONENT_NAME
		column_component_name,
#endif
		column_count
	};

	BEGIN_MSG_MAP(thisClass)
	{
		lResult = 0;
		if (m_resize_helper.process_message(hWnd, uMsg, wParam, lParam))
			return TRUE;
	}
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_CLOSE(OnClose)
		COMMAND_HANDLER_EX(IDCANCEL, BN_CLICKED, OnCancel)
	END_MSG_MAP()

	CFeatureLogViewer();

	virtual void OnFinalMessage(HWND) {delete this;}

	virtual bool pretranslate_message(MSG * p_msg);

	BOOL OnInitDialog(HWND hWndFocus, LPARAM lInitParam);
	void OnDestroy();
	void OnClose();
	void OnCancel(UINT code, int id, HWND wnd);

	void CreateGroups();
	void AddLogItem(const feature_log_item &p_item);

	pfc::map_t<GUID, int> m_feature_kind_groupids;

	dialog_resize_helper m_resize_helper;

	CListViewCtrl m_log;

	feature_log_item_list m_items;
};
