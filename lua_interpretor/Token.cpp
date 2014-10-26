#include "token.h"
#include <sstream>
#include <iomanip>

namespace lua_in
{
char* TokenType_Desc[] = {
	                      "Number","String", "Boolean", 
		                  "Operator", "Identifier", "Keywords",
		                  "Delimiter", "EOF", "Unknown" 
                         };

TokenLocation::TokenLocation(const std::string filename, int line, int column):
			m_filename(filename), m_line(line), m_column(column)
{}

TokenLocation::TokenLocation()
{}

std::string TokenLocation::toString() const
{
	std::stringstream strstream;
	strstream << "[Line: " << std::setw(4) << m_line << ", Column: " << std::setw(4)<< m_column << " in " << m_filename << "]";
	return strstream.str();
}


Token::Token(void) : m_type(T_Unknown), m_value(V_UnReserved),
	m_location("", 0, 0), m_name(""), m_symPriority(-1), m_number_value(0), m_str_value("")
{}

Token::~Token(void)
{}

Token::Token(TokenType type, TokenValue val, const TokenLocation& location, std::string name, int symbolPrec):
m_type(type), m_value(val), m_location(location), m_name(name), m_symPriority(symbolPrec), m_number_value(0), m_str_value("")
{}

Token::Token(TokenType type, TokenValue val, const TokenLocation& location, std::string name, double numValue):
m_type(type), m_value(val), m_location(location), m_name(name), m_symPriority(-1), m_number_value(numValue), m_str_value("") 
{}

Token::Token(TokenType type, TokenValue val, const TokenLocation& location, std::string name, std::string strValue):
m_type(type), m_value(val), m_location(location), m_name(name), m_symPriority(-1), m_number_value(0), m_str_value(strValue)
{}

Token::Token(std::tuple<TokenValue, TokenType, int> tuple_val, TokenLocation& location, std::string name)
{
	m_type = std::get<1>(tuple_val);
	m_value = std::get<0>(tuple_val);
	m_symPriority = std::get<2>(tuple_val);

	m_name = name;
	m_location = location;
}


std::string Token::toString() const
{
	std::stringstream strstream;
	strstream << m_location.toString() <<"\t"<< std::setiosflags(std::ios::left) << std::setw(10) << m_name;
	if (!m_errorReason.empty())
	{
		strstream << "\t\t" << ">> Error: " << m_errorReason;
	}

	return strstream.str();
}

bool Token::IsErrorToken() const
{
	if (m_errorReason.empty())
		return false;
	else
		return true;
}


}
