
#include "ast_node.h"
#include "ast_stat.h"
#include "ast_expr.h"

namespace lua_in
{

	AST_Node::AST_Node(void)
	{
	}

	AST_Node::~AST_Node(void)
	{
	}


	int AST_Operator::ComparePriority(AST_Operator* op)
	{
		if (m_priority > op->GetPriority())
		{
			return 1;
		}

		if (m_priority == op->GetPriority())
		{
			return 0;
		}
		else
		{
			return -1;
		}			
	}

	bool AST_Operator::IsSameName(AST_Operator* op)
	{
		return m_name == op->GetName();
	}


	void AST_Block::AddState(AST_Stat* ps)
	{
		m_vecStat.push_back(ps);
		ps->SetParent(this);
	}


	AST_ReStat::AST_ReStat(AST_ExpList* pl, size_t beg, size_t end) :
		m_pExplist(pl), AST_Node(beg, end)
	{
		m_pExplist->SetParent(this);
	}





	AST_FunCName::AST_FunCName(AST_Name* pf, size_t beg) :
		m_pFirst(pf), AST_Node(beg, beg)
	{
		m_pFirst->SetParent(this);
	}

	void AST_FunCName::AddDotName(AST_Name* pd)
	{
		pd->SetParent(this);
		m_vecDotNamelist.push_back(pd);
	}

	void AST_FunCName::SetColonName(AST_Name* pc)
	{
		pc->SetParent(this);
		m_pColonName = pc;
	}


	AST_VarList::AST_VarList(AST_Var* pv, size_t beg) :AST_Node(beg, beg)
	{
		pv->SetParent(this);
		m_vecVar.push_back(pv);
	}

	void AST_VarList::AddVar(AST_Var* pv)
	{
		pv->SetParent(this);
		m_vecVar.push_back(pv);
	}


	AST_NameList::AST_NameList(AST_Name* pv, size_t beg) :AST_Node(beg, beg)
	{
		pv->SetParent(this);
		m_vecName.push_back(pv);
	}

	void AST_NameList::AddVar(AST_Name* pv)
	{
		pv->SetParent(this);
		m_vecName.push_back(pv);
	}


	AST_ExpList::AST_ExpList(AST_Exp* pv, size_t beg) :AST_Node(beg, beg)
	{
		pv->SetParent(this);
		m_vecExp.push_back(pv);
	}

	void AST_ExpList::AddVar(AST_Exp* pv)
	{
		pv->SetParent(this);
		m_vecExp.push_back(pv);
	}


	AST_Args_Brack::AST_Args_Brack(AST_ExpList* pe, size_t beg, size_t end) :
		m_pExplist(pe), AST_Args(beg, end)
	{
		m_pExplist->SetParent(this);
	}


	AST_Args_Table::AST_Args_Table(AST_TableCons* pt, size_t beg, size_t end) :
		m_pTableCon(pt), AST_Args(beg, end)
	{

	}


	AST_Args_String::AST_Args_String(AST_String* ps, size_t beg, size_t end) :
		m_pString(ps), AST_Args(beg, end)
	{
		m_pString->SetParent(this);
	}


	AST_ParaList_Name::AST_ParaList_Name(AST_NameList* pn, size_t beg, size_t end) :
		AST_ParaList(beg, end), m_pNamelist(pn)
	{
		m_pNamelist->SetParent(this);
	}


	AST_FunBody::AST_FunBody(AST_ParaList* pl, AST_Block* pb, size_t beg, size_t end) :
		m_pParaList(pl), m_pBlock(pb), AST_Node(beg, end)
	{
		m_pParaList->SetParent(this);
		m_pBlock->SetParent(this);
	}


	AST_ParaList_Name_Dot::AST_ParaList_Name_Dot(AST_NameList* pn, size_t beg, size_t end) :
		AST_ParaList(beg, end), m_pNamelist(pn)
	{
		m_pNamelist->SetParent(this);
	}


	AST_Field_Brack::AST_Field_Brack(AST_Exp* p1, AST_Exp* p2, size_t beg, size_t end) :
		m_pExp1(p1), m_pExp2(p2), AST_Field(beg, end)
	{
		m_pExp1->SetParent(this);
		m_pExp2->SetParent(this);
	}


	AST_Field_Name::AST_Field_Name(AST_Name* nm, AST_Exp* pe, size_t beg, size_t end) :
		m_pName(nm), m_pExp(pe), AST_Field(beg, end)
	{
		m_pName->SetParent(this);
		m_pExp->SetParent(this);
	}


	AST_FieldList::AST_FieldList(AST_Field* pf, size_t beg, size_t end) :
		m_pFiled1(pf), AST_Node(beg, end)
	{
		m_pFiled1->SetParent(this);
	}

	void AST_FieldList::AddFFPair(AST_FieldStep* pfs, AST_Field* pfd)
	{
		pfs->SetParent(this);
		pfd->SetParent(this);
		m_vecFFPair.push_back(std::make_pair(pfs, pfd));
	}

	void AST_FieldList::SetLastFieldStep(AST_FieldStep* pfs)
	{
		pfs->SetParent(this);
		m_pFStep = pfs;
	}


	AST_Elseif_Node::AST_Elseif_Node(AST_Exp* pe, AST_Block* pb, size_t beg, size_t end) :
		m_pExp(pe), m_pBlock(pb), AST_Node(beg, end)
	{
		m_pExp->SetParent(this);
		m_pBlock->SetParent(this);
	}


	AST_Else_Node::AST_Else_Node(AST_Block* pb, size_t beg, size_t end) :
		m_pBlock(pb), AST_Node(beg, end)
	{
		m_pBlock->SetParent(this);
	}

}
