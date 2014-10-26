#ifndef DICTINARY_H_
#define DICTINARY_H_

#include <unordered_set>
#include <map>
#include <tuple>
#include "token.h"

namespace lua_in
{

class Dictinary
{
public:
	Dictinary();
	~Dictinary();
	std::tuple<TokenValue, TokenType, int> lookup(const std::string& name) const;
	bool haveToken(const std::string& name) const;

private:
	void addToken(std::string name, std::tuple<TokenValue, TokenType, int> tokenmeta);

private:
	std::map<std::string, std::tuple<TokenValue, TokenType, int>> m_dic;
};

}

#endif