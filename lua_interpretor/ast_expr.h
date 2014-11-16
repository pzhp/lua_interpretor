#ifndef AST_EXPR_H__
#define AST_EXPR_H__

#include "ast_node.h"

namespace lua_in
{
	class AST_Exp :public AST_Field
	{
	public:
		AST_Exp(size_t beg, size_t end) :
			AST_Field(beg, end)
		{}
	};

	class AST_NIL : public AST_Exp
	{
	public:
		AST_NIL(size_t beg) :
			AST_Exp(beg, beg)
		{}
	};

	class AST_BOOL :public AST_Exp
	{
	public:
		AST_BOOL(bool vl, size_t beg) :
			m_value(vl), AST_Exp(beg, beg)
		{}
	private:
		bool m_value;
		static std::string m_grammar;
	};

	class AST_String :public AST_Exp
	{
	public:
		AST_String(std::string nm, size_t beg) :
			m_liteValue(nm), AST_Exp(beg, beg)
		{}
	private:
		std::string m_liteValue;
	};

	class AST_Number :public AST_Exp
	{
	public:
		AST_Number(double nm, size_t beg) :
			m_liteValue(nm), AST_Exp(beg, beg)
		{}
	private:
		double m_liteValue;
	};

	class AST_3Dot :public AST_Exp
	{
		/* vararg exp in varargs function */
	public:
		AST_3Dot(size_t beg) :
			AST_Exp(beg, beg)
		{}
	private:
		static std::string m_grammar;
	};

	class AST_LambFunDef :public AST_Exp
	{
		/*functiondef ::= function funcbody*/
	public:
		AST_LambFunDef(AST_FunBody* pf, size_t beg, size_t end) :
			m_pFunbody(pf), AST_Exp(beg, end)
		{
			m_pFunbody->SetParent(this);
		}
	private:
		AST_FunBody* m_pFunbody;
	};

	/**** prefixexp  begin ***********/
	class AST_PrefixExp : public AST_Exp
	{
		/* prefixexp ::= var | functioncall | ¡®(¡¯ exp ¡®)' */
	public:
		AST_PrefixExp(size_t beg, size_t end) :
			AST_Exp(beg, end)
		{}
	};

	//var::
	class AST_Var :public AST_PrefixExp
	{
	public:
		AST_Var(size_t beg, size_t end) :
			AST_PrefixExp(beg, end)
		{}
	};

	class AST_Name :public AST_Var
	{
		/* var ::=  Name | prefixexp ¡®[¡¯ exp ¡®]¡¯ | prefixexp ¡®.¡¯ Name */
	public:
		AST_Name(std::string nm, size_t beg) :
			m_name(nm), AST_Var(beg, beg)
		{
		}
	private:
		std::string m_name;
	};

	class AST_Var_Brack :public AST_Var
	{
	public:
		AST_Var_Brack(AST_PrefixExp* pp, AST_Exp* pe, size_t beg, size_t end) :
			m_pPrefix(pp), m_pExp(pe), AST_Var(beg, end)
		{
			m_pPrefix->SetParent(this);
			m_pExp->SetParent(this);
		}
	private:
		AST_PrefixExp* m_pPrefix;
		AST_Exp* m_pExp;
	};

	class AST_Var_Dot :public AST_Var
	{
	public:
		AST_Var_Dot(AST_PrefixExp* pp, AST_Name* pe, size_t beg, size_t end) :
			m_pPrefix(pp), m_pName(pe), AST_Var(beg, end)
		{}
	private:
		AST_PrefixExp* m_pPrefix;
		AST_Name* m_pName;
	};

	/* AST_FunCall in ast_stat.h */

	class AST_Brack_Exp :public AST_PrefixExp
	{
	public:
		AST_Brack_Exp(AST_Exp* pp, size_t beg, size_t end) :
			m_pExp(pp), AST_PrefixExp(beg, end)
		{}
	private:
		AST_Exp* m_pExp;
		static std::string m_grammar;
	};
	/**** prefixexp  begin ***********/


	class AST_TableCons : public AST_Exp
	{
	public:
		AST_TableCons(AST_FieldList* pfi, size_t beg, size_t end) :
			m_pFieldlist(pfi), AST_Exp(beg, end)
		{}
	private:
		AST_FieldList* m_pFieldlist;
	};

	class AST_BiExpr :public AST_Exp
	{
	public:
		AST_BiExpr(AST_Exp* lf, AST_Exp* rg, AST_BiOp* op, size_t beg, size_t end) :
			m_pLeft(lf), m_pright(rg), m_pOP(op), AST_Exp(beg, end)
		{}
	private:
		AST_Exp* m_pLeft;
		AST_Exp* m_pright;
		AST_BiOp* m_pOP;
		static std::string m_grammar;
	};

	class AST_UnExpr :public AST_Exp
	{
	public:
		AST_UnExpr(AST_Exp* lf, AST_UnOp* op, size_t beg, size_t end) :
			m_pExp(lf), m_pOP(op), AST_Exp(beg, end)
		{}
	private:
		AST_Exp* m_pExp;
		AST_UnOp* m_pOP;
	};

}

#endif // !AST_EXPR_H__