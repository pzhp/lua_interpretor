#include <cctype>
#include <cstdlib>
#include <string>
#include <sstream>

#include "scanner.h"

namespace lua_in
{

extern ErrorOutput g_InfoOutput;

Scanner::Scanner(void)
{
}

Scanner::Scanner(std::string in_fileName) : m_line(1), m_column(0), 
		m_infileName(in_fileName), m_isErrRecover(false)
{
	g_InfoOutput.SetPrefix("Lex");
	m_infileStream.open(in_fileName.c_str());
	if (m_infileStream.fail())
	{
		g_InfoOutput.Dump("cannot open input file\n",true);
		m_currState = S_Fatal;
	}
}


Scanner::~Scanner(void)
{
	m_infileStream.close();
}

char Scanner::GetNextChar()
{
	char ch = m_infileStream.get();	
	if (m_infileStream.eof())
	{
		m_currState = S_EOF;
		return 0;
	}

	if (ch == '\n')
	{
		m_line++;
		m_column = 0;
	}
	else
	{
		m_column++;
	}

	return ch;
}

char Scanner::PeekNextChar()
{
	char ch = m_infileStream.peek();
	if (m_infileStream.eof())
	{
		m_currState = S_EOF;
		return 0;
	}
	return ch;
}

Token Scanner::ProcessNumberState( char ch )
{	
	/* only support int(12),float(12.0), both are saved in "double", not support 'e' form */
	bool isfloat = false;
	while (true)
	{
		if (isdigit(ch))
		{
			m_buffer += ch;
		}
		else if (ch == '.' && !isfloat) //first match 1.2 except 1.2.3
		{
			m_buffer += ch;
			isfloat = true;
		}
		else if (isspace(ch)|| m_currState==S_EOF)
		{
			m_currState = S_Accept;
			return Token(T_ConstNumber, V_ConstNumber, m_curLoc, m_buffer, std::atof(m_buffer.c_str()));
		}
		else
		{
			m_currState = S_Error;
			m_buffer += ch;
			m_strErrReason = "cannot recognise number";
			return Token();
		}
		ch = GetNextChar();
	}
}

Token Scanner::ProcessIndentifyState(char ch)
{
	while (true)
	{
		if (m_currState == S_EOF || m_currState == S_Accept)
		{
			m_currState = S_Accept;
			if (m_dict.haveToken(m_buffer))// is reserved
			{
				return Token(m_dict.lookup(m_buffer), m_curLoc, m_buffer);
			}
			else // is identifier
			{
				return Token(T_Identify, V_UnReserved, m_curLoc, m_buffer);
			}
		}

		if (isdigit(ch) || isalpha(ch) || ch == '_')
		{
			m_buffer += ch;
			ch = GetNextChar();
		}
		else // form "print(a)"
		{
			m_currState = S_Accept;
			PutBackChar(ch);
		}
	}	
}

Token Scanner::GetNextToken()
{
	while (true)
	{		
		Token tk;
		// init a single token recognise state
		m_isErrRecover = false;
		m_currState = S_Begin;
		m_strErrReason.clear();
		m_buffer.clear();

		char ch = GetNextChar();
		//deal with space char
		while (isspace(ch) && m_currState != S_EOF)
		{
			ch = GetNextChar();
		}
		if (m_currState == S_EOF)
		{
			tk = Token(T_EOF, V_UnReserved, TokenLocation(m_infileName,m_line,m_column), std::string("EOF"));
			return tk;
		}
		
		//record location at a useful char
		StampLocation();
		
		if (isdigit(ch))
		{
			tk = ProcessNumberState(ch);
		}
		else if (std::isalpha(ch) || ch == '_')
		{
			tk = ProcessIndentifyState(ch);
		}
		else if (ch == '\'' || ch == '"')
		{
			tk = ProcessStringState(ch);
		}
		else if (ch == '-' && PeekNextChar() == '-')
		{
			HandleComemnt(ch);
			m_currState = S_Begin;
			continue;
		}
		else if (m_dict.haveToken(std::string(1,ch)))
		{
			tk = ProcessOperatorAndDelim(ch);
		}
		else
		{
			m_currState = S_Fatal;
			m_strErrReason = "cannot find dispatch method";
		}

		//result
		if (m_currState == S_Error)
		{
			if (!m_isErrRecover)
			{
				RecoverFromError();
				tk = Token(T_Unknown, V_UnReserved, m_curLoc, m_buffer);
				tk.SetTokenErrorReason(m_strErrReason);
			}
			m_currState = S_Begin;
		}
		
		if (m_currState == S_Fatal)
		{
			tk = Token(T_Unknown, V_UnReserved, m_curLoc, m_buffer);
			tk.SetTokenErrorReason(m_strErrReason);
		}

		return tk;
	}
}

void Scanner::StampLocation()
{
	m_curLoc = TokenLocation(m_infileName, m_line, m_column);
}

void Scanner::RunScanner()
{
	Token tk;
	while (m_currState != S_EOF && m_currState != S_Fatal)
	{
		tk = GetNextToken();
		m_vecToken.push_back(tk);
		g_InfoOutput.Dump(tk.toString(),tk.IsErrorToken());
	}
}

void Scanner::RecoverFromError()
{
	char ch = PeekNextChar();
	while (!isspace(ch) && m_currState != S_EOF)
	{
		ch = GetNextChar();
		m_buffer += ch;
		ch = PeekNextChar();
	}
}

lua_in::Token Scanner::ProcessStringState(char ch)
{
	/*support: form(''), form("")*/
	
	char flag = ch;
	ch = GetNextChar();
	Token tk;

	while (m_currState != S_EOF)
	{
		if ((ch == flag) && (m_buffer.empty()||m_buffer.back()!= '\\'))
		{
			m_currState = S_Accept;
			std::string desc(1,flag);
			desc += m_buffer;
			desc += flag;
			tk = Token(T_ConstString, V_ConstString, m_curLoc, desc, m_buffer);
			break;
		}

		m_buffer += ch;
		ch = GetNextChar();
	}
	
	//error deal
	if (m_currState == S_EOF)
	{
		m_currState = S_Error;
		std::string desc(1, flag);
		desc += m_buffer;
		tk = Token(T_Unknown, V_UnReserved, m_curLoc, desc, m_buffer);
		tk.SetTokenErrorReason(std::string("cannot recognise literal string"));
		m_isErrRecover = true;
	}
	return tk;
}

void Scanner::HandleComemnt(char ch)
{
	/* support a single row comment */
	ch = GetNextChar();//2th -
	while (ch!='\n' && m_currState != S_EOF)
	{
		ch = GetNextChar();
	}
	return;
}

lua_in::Token Scanner::ProcessOperatorAndDelim(char ch)
{
	std::string str(1, ch);
	m_buffer += ch;
	str += PeekNextChar();
	while (m_dict.haveToken(str) && m_currState != S_EOF)
	{
		ch = GetNextChar();
		m_buffer += ch;
		str += PeekNextChar();
	}
	return Token(m_dict.lookup(m_buffer), m_curLoc, m_buffer);
}

void Scanner::PutBackChar(char ch)
{
	if (!m_infileStream.putback(ch))
	{
		g_InfoOutput.Dump("cannot putback char into filestream", true);
		m_currState = S_Fatal;
	}
}
}
