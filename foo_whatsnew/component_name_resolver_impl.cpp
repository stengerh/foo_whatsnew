#include "stdafx.h"
#include "component_name_resolver_impl.h"

component_name_resolver_impl::component_name_resolver_impl() : m_initialized(false)
{
}

static void g_free_library(HMODULE hModule, const pfc::string8 & unused)
{
	::FreeLibrary(hModule);
}

component_name_resolver_impl::~component_name_resolver_impl()
{
	m_module_map.enumerate(g_free_library);
	m_module_map.remove_all();
	m_component_map.remove_all();
}

bool component_name_resolver_impl::get_module_name(HMODULE p_module, pfc::string_base & p_module_name)
{
	ensure_initialized();

	return m_module_map.query(p_module, p_module_name);
}

bool component_name_resolver_impl::get_component_name(HMODULE p_module, pfc::string_base & p_component_name)
{
	ensure_initialized();

	return m_component_map.query(p_module, p_component_name);
}

void component_name_resolver_impl::ensure_initialized()
{
	if (!m_initialized)
	{
		m_initialized = true;

		service_enum_t<componentversion> e;
		service_ptr_t<componentversion> ptr;

		e.reset();

		while (e.next(ptr))
		{
			HMODULE hModule = g_get_module_from_service(ptr);

			if (hModule != NULL)
			{
				if (!m_module_map.have_item(hModule))
				{
					pfc::string8 module_name;

					DWORD length = ::uGetModuleFileName(hModule, module_name);
					if (length > 0)
					{
						pfc::string8 component_name;
						ptr->get_component_name(component_name);

						m_module_map.set(hModule, module_name);
						m_component_map.set(hModule, component_name);

#ifdef _DEBUG
						uDebugLog() << "Found " << module_name << " - " << component_name;
#endif
					}
				}
				else
				{
#ifdef _DEBUG
					uDebugLog() << "Duplicate componentversion instance for " << m_module_map[hModule];
#endif
				}
			}
		}

		HMODULE hModuleCore = ::GetModuleHandle(NULL);

		if (!m_module_map.have_item(hModuleCore))
		{
			m_module_map.set(hModuleCore, "Core");
			m_component_map.set(hModuleCore, "foobar2000 core");
		}
	}
}
