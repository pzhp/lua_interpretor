#include "error.h"
#include <iomanip>

namespace lua_in
{

ErrorOutput g_InfoOutput(std::string("lexinfo.txt"));

ErrorOutput::ErrorOutput(void)
{}

ErrorOutput::~ErrorOutput(void)
{
	m_infoStream.close();
}

ErrorOutput::ErrorOutput(std::string info_fileNmae):
 m_infoStream(info_fileNmae.c_str())
{
	m_infoStream << "*********Info Log*******" << std::endl;
	m_isDumpAll = true;
}

void ErrorOutput::Dump(std::string info, bool iserror)
{
	if (m_isDumpAll || iserror)
	{
		m_infoStream << m_strPre <<": " << info << std::endl;
	}
}

void ErrorOutput::SetPrefix(std::string str)
{
	m_strPre = str;
}

void ErrorOutput::SetDumpLevel(bool isAll)
{
	m_isDumpAll = isAll;
}

}