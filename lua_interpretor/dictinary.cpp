#include "dictinary.h"

namespace lua_in
{

Dictinary::Dictinary()
{
	/* need to modify priority in later*/
	addToken("and",     std::make_tuple(TokenValue::V_And,     TokenType::T_Keywords,    -1));
	addToken("break",   std::make_tuple(TokenValue::V_Break,   TokenType::T_Keywords,    -1));
	addToken("do",      std::make_tuple(TokenValue::V_Do,      TokenType::T_Keywords,    -1));
	addToken("else",    std::make_tuple(TokenValue::V_Else,    TokenType::T_Keywords,    -1));
	addToken("elseif",  std::make_tuple(TokenValue::V_Elseif,  TokenType::T_Keywords,    -1));
	addToken("end",     std::make_tuple(TokenValue::V_End,     TokenType::T_Keywords,    -1));
	addToken("false",   std::make_tuple(TokenValue::V_False,   TokenType::T_Keywords,    -1));
	addToken("true",    std::make_tuple(TokenValue::V_True,    TokenType::T_Keywords,    -1));
	addToken("function",std::make_tuple(TokenValue::V_Function,TokenType::T_Keywords,    -1));
	addToken("goto",    std::make_tuple(TokenValue::V_Goto,    TokenType::T_Keywords,    -1));
	addToken("if",      std::make_tuple(TokenValue::V_If,      TokenType::T_Keywords,    -1));
	addToken("in",      std::make_tuple(TokenValue::V_In,      TokenType::T_Keywords,    -1));
	addToken("then",    std::make_tuple(TokenValue::V_Then,    TokenType::T_Keywords,	 -1));
	addToken("until",   std::make_tuple(TokenValue::V_Until,   TokenType::T_Keywords,	 -1));
	addToken("while",   std::make_tuple(TokenValue::V_While,   TokenType::T_Keywords,	 -1));
	addToken("nil",     std::make_tuple(TokenValue::V_NIL,     TokenType::T_Keywords,    -1));
	addToken("print",   std::make_tuple(TokenValue::V_Print,   TokenType::T_Keywords,    -1));
	addToken("println", std::make_tuple(TokenValue::V_PrintLn, TokenType::T_Keywords,    -1));
	addToken("read",    std::make_tuple(TokenValue::V_Read,    TokenType::T_Keywords,    -1));
	addToken("readln",  std::make_tuple(TokenValue::V_ReadLn,  TokenType::T_Keywords,    -1));

	addToken("+",	    std::make_tuple(TokenValue::V_Plus,    TokenType::T_Operators, -1));
	addToken("-",       std::make_tuple(TokenValue::V_Minus,   TokenType::T_Operators, -1));
	addToken("*",       std::make_tuple(TokenValue::V_Mutiply, TokenType::T_Operators, -1));
	addToken("/",       std::make_tuple(TokenValue::V_Divide,  TokenType::T_Operators, -1));
	addToken("%",       std::make_tuple(TokenValue::V_Mod,     TokenType::T_Operators, -1));
	addToken("^",       std::make_tuple(TokenValue::V_Uparrow, TokenType::T_Operators, -1));
	addToken("#",       std::make_tuple(TokenValue::V_Sharp,   TokenType::T_Operators, -1));
	addToken("==",      std::make_tuple(TokenValue::V_Equal,   TokenType::T_Operators, -1));
	addToken("~=",      std::make_tuple(TokenValue::V_NotEqual,TokenType::T_Operators, -1));
	addToken("<=",      std::make_tuple(TokenValue::V_LessEqual,    TokenType::T_Operators, -1));
	addToken(">=",      std::make_tuple(TokenValue::V_GreaterEqual, TokenType::T_Operators, -1));
	addToken("<",       std::make_tuple(TokenValue::V_Less,         TokenType::T_Operators, -1));
	addToken(">",       std::make_tuple(TokenValue::V_Greater,      TokenType::T_Operators, -1));
	addToken("=",       std::make_tuple(TokenValue::V_Assign,       TokenType::T_Operators, -1));

	addToken("(",       std::make_tuple(TokenValue::V_LeftParen,    TokenType::T_Delimiter, -1));
	addToken(")",       std::make_tuple(TokenValue::V_RightParen,   TokenType::T_Delimiter, -1));
	addToken("[",       std::make_tuple(TokenValue::V_LeftSquare,   TokenType::T_Delimiter, -1));
	addToken("]",       std::make_tuple(TokenValue::V_RightSquare,  TokenType::T_Delimiter, -1));
	addToken("{",       std::make_tuple(TokenValue::V_LeftBrace,    TokenType::T_Delimiter, -1));
	addToken("}",       std::make_tuple(TokenValue::V_RightBrace,   TokenType::T_Delimiter, -1));
	
	addToken("::",      std::make_tuple(TokenValue::V_ColonTwo,     TokenType::T_Delimiter, -1));
	addToken(";",       std::make_tuple(TokenValue::V_SemiColon,    TokenType::T_Delimiter, -1));
	addToken(":",       std::make_tuple(TokenValue::V_Colon,        TokenType::T_Delimiter, -1));
	addToken(",",       std::make_tuple(TokenValue::V_Comma,        TokenType::T_Delimiter, -1));
	addToken(".",       std::make_tuple(TokenValue::V_DotOne,       TokenType::T_Delimiter, -1));
	addToken("..",      std::make_tuple(TokenValue::V_DotTwo,       TokenType::T_Delimiter, -1));
	addToken("...",     std::make_tuple(TokenValue::V_DotThree,     TokenType::T_Delimiter, -1));
}


Dictinary::~Dictinary()
{}

void Dictinary::addToken(std::string name, std::tuple<TokenValue, TokenType, int> tokenmeta)
{
	m_dic.insert(make_pair(name, tokenmeta));
}

std::tuple<TokenValue, TokenType, int> Dictinary::lookup(const std::string& name) const
{
	auto iter = m_dic.find(name);
	if (iter != m_dic.end())
	{
		return iter->second;
	}

	return std::make_tuple(TokenValue::V_UnReserved, TokenType::T_Identify, -1);
}

bool Dictinary::haveToken(const std::string& name) const
{
	auto iter = m_dic.find(name);
	if (iter != m_dic.end())
	{
		return true;
	}
	return false;
}

}