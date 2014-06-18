#pragma once

namespace pfc
{
	class format_timestamp
	{
		string_formatter m_buffer;
	public:
		explicit format_timestamp(t_filetimestamp p_timestamp, bool p_local_time);
		operator const char*() {return m_buffer;}
	};
}

