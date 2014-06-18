#include "stdafx.h"
#include "tools.h"

namespace pfc
{
	format_timestamp::format_timestamp(t_filetimestamp p_timestamp, bool p_local_time)
	{
		FILETIME ft;
		if (p_local_time)
		{
			FileTimeToLocalFileTime((const FILETIME*)&p_timestamp, &ft);
		}
		else
		{
			ft = *(const FILETIME*)&p_timestamp;
		}

		SYSTEMTIME st;
		FileTimeToSystemTime(&ft, &st);

		m_buffer << format_uint(st.wYear, 4) << "-" << format_uint(st.wMonth, 2) << "-" << format_uint(st.wDay, 2) << " "
			<< format_uint(st.wHour, 2) << ":" << format_uint(st.wMinute, 2) << ":" << format_uint(st.wSecond, 2);
	}
}
