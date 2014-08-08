#include "stdafx.h"
#include "component_name_resolver.h"

HMODULE component_name_resolver::g_get_module_from_service(service_ptr p_ptr)
{
	struct raw_service_vtable
	{
		const void * const m_service_release;
		const void * const m_service_add_ref;
		const void * const m_service_query;
	};

	struct raw_service_base
	{
		const raw_service_vtable * const m_vtable;
	};

	bool success = false;

	raw_service_base *ptr = reinterpret_cast<raw_service_base *>(p_ptr.get_ptr());

	HMODULE hModule = NULL;

	BOOL result = ::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, 
		(LPCWSTR) ptr->m_vtable->m_service_query, &hModule);

	if (result == FALSE)
	{
		hModule = NULL;
	}
	else
	{
		success = true;
	}

	return hModule;
}
