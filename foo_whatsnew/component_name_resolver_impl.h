#pragma once

#include "component_name_resolver.h"

class component_name_resolver_impl : public component_name_resolver
{
public:
	component_name_resolver_impl();
	virtual ~component_name_resolver_impl();

	virtual bool get_module_name(HMODULE p_module, pfc::string_base & p_module_name);
	virtual bool get_component_name(HMODULE p_module, pfc::string_base & p_component_name);

private:
	void ensure_initialized();

	bool m_initialized;
	pfc::map_t<HMODULE, pfc::string8> m_module_map;
	pfc::map_t<HMODULE, pfc::string8> m_component_map;
};