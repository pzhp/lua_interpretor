#ifndef ERROR_H_
#define ERROR_H_

#include <string>
#include <fstream>

namespace lua_in
{

class ErrorOutput
{
public:
	ErrorOutput(void);
	ErrorOutput(std::string info_fileNmae);
	~ErrorOutput(void);

	void Dump(std::string  info, bool iserror=0);
	void SetPrefix(std::string str);
	void SetDumpLevel(bool isAll);

private:
	std::ofstream m_infoStream;
	std::string m_strPre;
	bool m_isDumpAll;	
};

}
#endif