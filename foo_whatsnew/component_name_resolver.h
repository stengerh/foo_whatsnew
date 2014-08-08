#pragma once

class component_name_resolver
{
protected:
	component_name_resolver() {}
	virtual ~component_name_resolver() {}

public:
	virtual bool get_module_name(HMODULE p_module, pfc::string_base & p_module_name) = 0;
	virtual bool get_component_name(HMODULE p_module, pfc::string_base & p_component_name) = 0;

	static HMODULE g_get_module_from_service(service_ptr p_ptr);
};
