#ifndef AST_STAT_H__
#define AST_STAT_H__

#include <vector>

#include "ast_node.h"
#include "ast_expr.h"

namespace lua_in
{

	class AST_Stat : public AST_Node
	{
	public:
		AST_Stat(size_t beg, size_t end) : AST_Node(beg, end)
		{}
	};

	class AST_StatDelim :public AST_Stat
	{
	public:
		AST_StatDelim(size_t beg) : AST_Stat(beg, beg){}
	};

	class AST_AssignStat :public AST_Stat
	{
	public:
		AST_AssignStat(AST_VarList* pvar, AST_ExpList* pexp, size_t beg, size_t end) :
			m_varlist(pvar), m_explist(pexp), AST_Stat(beg, end)
		{}

	private:
		AST_VarList* m_varlist;
		AST_ExpList* m_explist;
		static std::string m_grammar;
	};

	class AST_PrefixExp;
	class AST_FunCall : public AST_Stat, public AST_PrefixExp
	{
		/* functioncall ::=  prefixexp args | prefixexp ¡®:¡¯ Name args */
	public:
		AST_FunCall(AST_PrefixExp* sp, AST_Args* sa, size_t beg, size_t end) :
			m_preExp(sp), m_Args(sa), AST_Stat(beg, end), AST_PrefixExp(beg,end)
		{}

	private:
		AST_PrefixExp* m_preExp;
		AST_Args* m_Args;
	};

	class AST_FunCallNoName : public AST_FunCall
	{
	public:
		AST_FunCallNoName(AST_PrefixExp* sp, AST_Args* sa, size_t beg, size_t end) :
			AST_FunCall(sp, sa, beg, end)
		{}
	};

	class AST_FunCallName :public AST_FunCall
	{
	public:
		AST_FunCallName(AST_PrefixExp* sp, AST_Name* sn, AST_Args* sa, size_t beg, size_t end) :
			AST_FunCall(sp, sa, beg, end), m_sname(sn)
		{}
	private:
		AST_Name* m_sname;
	};

	class AST_Label : public AST_Stat
	{
	public:
		AST_Label(AST_Name* sn, size_t beg, size_t end);

	private:
		AST_Name* m_sname;
	};

	class AST_Break : public AST_Stat
	{
	public:
		AST_Break(size_t beg) : AST_Stat(beg, beg)
		{}

	private:
		static std::string m_grammar;
	};

	class AST_Goto :public AST_Stat
	{
	public:
		AST_Goto(AST_Name* pn, size_t beg, size_t end) :
			m_pName(pn), AST_Stat(beg, end)
		{}

	private:
		AST_Name* m_pName;
	};

	class AST_DoEnd :public AST_Stat
	{
	public:
		AST_DoEnd(AST_Block* pb, size_t beg, size_t end) :
			m_pBlock(pb), AST_Stat(beg, end)
		{
			m_pBlock->SetParent(this);
		}
	private:
		AST_Block* m_pBlock;
		static std::string m_grammar;
	};

	class AST_While : public AST_Stat
	{
	public:
		AST_While(AST_Exp* ep, AST_DoEnd* pb, size_t beg, size_t end) :
			m_pExp(ep), m_pDoEnd(pb), AST_Stat(beg, end)
		{
			m_pExp->SetParent(this);
			m_pDoEnd->SetParent(this);
		}

	private:
		AST_Exp* m_pExp;
		AST_DoEnd* m_pDoEnd;
	};

	class AST_Repeat : public AST_Stat
	{
	public:
		AST_Repeat(AST_Block* pb, AST_Exp* pe, size_t beg, size_t end) :
			m_pBlock(pb), m_pExp(pe), AST_Stat(beg, end)
		{ 
			m_pBlock->SetParent(this);
			m_pExp->SetParent(this);
		}
	private:
		AST_Block* m_pBlock;
		AST_Exp* m_pExp;
	};


	class AST_Condition :public AST_Stat
	{

	public:
		AST_Condition(AST_Exp* ifexp, AST_Block* ifb, size_t beg, size_t end) :
			m_pExp(ifexp), m_pBlock(ifb), AST_Stat(beg, end)
		{  }

		void AddElseIf(AST_Elseif_Node* eip)
		{
			m_vecElseif.push_back(eip);
		}

		void SetElse(AST_Else_Node* ep)
		{
			m_pElse = ep;
		}

	private:
		AST_Exp* m_pExp;
		AST_Block* m_pBlock;
		std::vector<AST_Elseif_Node*> m_vecElseif;
		AST_Else_Node* m_pElse;
		static std::string m_grammar;
	};

	class AST_For3 :public AST_Stat
	{
	public:
		AST_For3(AST_Name* nm, AST_Exp* p1, AST_Exp* p2, AST_Exp* step, AST_DoEnd* pdoend, size_t beg, size_t end) :
			m_name(nm), m_p1(p1), m_p2(p2), m_pStep(step), m_pDoEnd(pdoend), AST_Stat(beg, end)
		{
			m_name->SetParent(this);
			m_p1->SetParent(this);
			m_p2->SetParent(this);
			if (step) m_pStep->SetParent(this);
			m_pDoEnd->SetParent(this);
		}
	private:
		AST_Name* m_name;
		AST_Exp* m_p1;
		AST_Exp* m_p2;
		AST_Exp* m_pStep;
		AST_DoEnd* m_pDoEnd;
	};


	class AST_ForIn :public AST_Stat
	{
	public:
		AST_ForIn(AST_NameList* pnl, AST_ExpList* pel, AST_DoEnd* pdoend, size_t beg, size_t end) :
			m_pNamelist(pnl), m_pExplist(pel), m_pDoEnd(pdoend), AST_Stat(beg, end)
		{
			m_pNamelist->SetParent(this);
			m_pExplist->SetParent(this);
			m_pDoEnd->SetParent(this);
		}

	private:
		AST_NameList*  m_pNamelist;
		AST_ExpList*   m_pExplist;
		AST_DoEnd* m_pDoEnd;
	};

	class AST_FunDef :public AST_Stat
	{
	public:
		AST_FunDef(AST_Name* nm, AST_FunBody* pf, size_t beg, size_t end) :
			m_pName(nm), m_pFunbody(pf), AST_Stat(beg, end)
		{
			m_pName->SetParent(this);
			m_pFunbody->SetParent(this);
		}

	private:
		AST_Name* m_pName;
		AST_FunBody* m_pFunbody;
	};

	class AST_Local_FunDef : public AST_Stat
	{
	public:
		AST_Local_FunDef(AST_FunDef* pf, size_t beg, size_t end) :
			m_pFundef(pf), AST_Stat(beg, end)
		{
			m_pFundef->SetParent(this);
		}
	private:
		AST_FunDef* m_pFundef;
	};

	class AST_Local_NameList : public AST_Stat
	{
	public:
		AST_Local_NameList(AST_NameList* pl, AST_ExpList* pe, size_t beg, size_t end) :
			m_pNamelist(pl), m_pExplist(pe), AST_Stat(beg, end)
		{
			m_pNamelist->SetParent(this);
			m_pExplist->SetParent(this);
		}
	private:
		AST_NameList* m_pNamelist;
		AST_ExpList* m_pExplist;
	};

}
#endif