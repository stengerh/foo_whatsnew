#include "ui_extension.h"

namespace ui_helpers
{
const window_position_t ui_helpers::window_position_null(0,0,0,0);


bool container_window::class_release()
{
	if (!get_class_data().refcount && get_class_data().class_registered)
	{
		BOOL ret = UnregisterClass(get_class_data().class_name, core_api::get_my_instance());
		get_class_data().class_registered = !ret;
	}
	return get_class_data().class_registered == false;
}

container_window::container_window() : wnd_host(0){};

initquit_factory_t<container_window_autorelease_t::initquit_t> g_container_window_autorelease_initquit;
container_window_autorelease_t::container_window_autorelease_t() 
{
	g_container_window_autorelease_initquit.get_static_instance().register_window(*this);
};
container_window_autorelease_t::~container_window_autorelease_t() 
{
	g_container_window_autorelease_initquit.get_static_instance().deregister_window(*this);
};
container_window::container_window(const container_window & p_source)
{
	assert(0);
}

HWND container_window::create(HWND wnd_parent, LPVOID create_param, const ui_helpers::window_position_t & p_window_position)
{
	
	ensure_class_registered();
	LPVOID createparams[2] = {this, create_param};
	wnd_host = CreateWindowEx(get_class_data().ex_styles, get_class_data().class_name, get_class_data().window_title,
		get_class_data().styles, p_window_position.x, p_window_position.y, p_window_position.cx, p_window_position.cy,
		wnd_parent, 0, core_api::get_my_instance(), &createparams);
	
	return wnd_host;
}

HWND container_window::create_in_dialog_units(HWND wnd_dialog, const ui_helpers::window_position_t & p_window_position, LPVOID create_param)
{
	RECT rc;
	p_window_position.convert_to_rect(rc);
	MapDialogRect(wnd_dialog, &rc);
	return create(wnd_dialog, create_param, ui_helpers::window_position_t(rc));
}
bool container_window::ensure_class_registered()
{
	if (!get_class_data().class_registered)
	{
		WNDCLASS  wc;
		memset(&wc,0,sizeof(WNDCLASS));
		wc.lpfnWndProc    = (WNDPROC)window_proc;
		wc.hInstance      = core_api::get_my_instance();
		wc.hCursor        = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
		wc.hbrBackground  = get_class_data().want_transparent_background ? NULL : (HBRUSH)(COLOR_BTNFACE+1);
		wc.lpszClassName  = get_class_data().class_name;
		wc.style = get_class_data().class_styles;
		wc.cbWndExtra  = get_class_data().extra_wnd_bytes;
		
		get_class_data().class_registered = (RegisterClass(&wc) != 0);
	}
	return get_class_data().class_registered;
}

void container_window::destroy()
{
	DestroyWindow(wnd_host);
	class_release();
}

LRESULT WINAPI container_window::window_proc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	container_window * p_this;
	
	if(msg == WM_NCCREATE)
	{
		LPVOID * create_params = reinterpret_cast<LPVOID *>(((CREATESTRUCT *)(lp))->lpCreateParams);
		p_this = reinterpret_cast<container_window *>(create_params[0]); //retrieve pointer to class
		SetWindowLongPtr(wnd, GWL_USERDATA, (LPARAM)p_this);//store it for future use
		if (p_this)
		{
			p_this->get_class_data().refcount++;
			p_this->wnd_host = wnd;
		}
		
	}
	else
		p_this = reinterpret_cast<container_window*>(GetWindowLongPtr(wnd,GWL_USERDATA));//if isnt wm_nccreate, retrieve pointer to class

	if (msg == WM_NCDESTROY)
	{
		if (p_this)
		{
			p_this->get_class_data().refcount--;
			p_this->wnd_host = NULL;
		}
	}
	
	if (p_this && p_this->get_class_data().want_transparent_background)
	{
		if (msg == WM_ERASEBKGND)
		{			
			HDC dc = (HDC)wp;

			HWND wnd_parent = GetParent(wnd);
			POINT pt = {0, 0}, pt_old = {0,0};
			MapWindowPoints(wnd, wnd_parent, &pt, 1);
			OffsetWindowOrgEx(dc, pt.x, pt.y, &pt_old);
			SendMessage(wnd_parent, WM_ERASEBKGND,wp, 0);
			SetWindowOrgEx(dc, pt_old.x, pt_old.y, 0);

			return TRUE;
		}
		else if (msg==WM_MOVE || msg==WM_SIZE)
		{
			RedrawWindow(wnd, 0, 0, RDW_ERASE|RDW_INVALIDATE|RDW_ALLCHILDREN);
		}
	}

	if ( msg == WM_SETTINGCHANGE && p_this && p_this->get_class_data().forward_system_settings_change)
	{
		win32_helpers::send_message_to_direct_children(wnd, msg, wp, lp);
	}
	else if ( msg == WM_SYSCOLORCHANGE && p_this && p_this->get_class_data().forward_system_colours_change)
	{
		win32_helpers::send_message_to_direct_children(wnd, msg, wp, lp);
	}
	
	return p_this ? p_this->on_message(wnd, msg, wp, lp) : uDefWindowProc(wnd, msg, wp, lp);
}

HWND container_window::get_wnd()const{return wnd_host;}

}