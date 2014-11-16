#include "ast_stat.h"
#include "ast_expr.h"


namespace lua_in
{

	AST_Label::AST_Label(AST_Name* sn, size_t beg, size_t end) :
		AST_Stat(beg, end), m_sname(sn)
	{
		m_sname->SetParent(this);
	}

}