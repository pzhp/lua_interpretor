#ifndef SNODE_H__
#define SNODE_h__

#include <string>
#include <vector>
#include "Token.h"

namespace lua_in
{

	class AST_Stat;
	class AST_ReStat;
	class AST_Exp;

	class AST_Node
	{
	public:
		AST_Node(void);
		AST_Node(size_t beg, size_t end) : m_begLoc(beg), m_endLoc(end), m_parent(NULL){}
		virtual ~AST_Node(void);
		inline void SetParent(AST_Node* p){ m_parent = p; }
		inline void SetEndPosit(size_t end)
		{
			m_endLoc = end;
		}
	private:
		size_t m_begLoc;
		size_t m_endLoc;
		AST_Node* m_parent;
		static std::string m_descr;
	};

	/***************************basic elements*********************************************/
	class AST_Operator : public AST_Node
	{
	public:
		AST_Operator(std::string nm, size_t beg, size_t end) : m_name(nm), AST_Node(beg, end)
		{}
	private:
		std::string m_name;
	};

	class AST_BiOp : public AST_Operator
	{
	public:
		AST_BiOp(std::string op, size_t beg, size_t end) :
			AST_Operator(op, beg, end)
		{}
	};

	class AST_UnOp : public AST_Operator
	{
	public:
		AST_UnOp(std::string op, size_t beg, size_t end) :
			AST_Operator(op, beg, end)
		{}
	};

	class AST_Delimiter : public AST_Node
	{
	public:
		AST_Delimiter(std::string nm, size_t beg, size_t end) :
			m_name(nm), AST_Node(beg, end)
		{}
	private:
		std::string m_name;
	};

	class AST_Name : public AST_Var
	{
	public:
		AST_Name(std::string nm, size_t beg) :m_name(nm), AST_Var(beg, beg)
		{}
	private:
		std::string m_name;
	};

	/***********************************************/
	class AST_Chunk :public AST_Node
	{
	public:
		AST_Chunk(size_t beg, size_t end) :
			AST_Node(beg, end)
		{}
	};

	class AST_Block :public AST_Chunk
	{
	public:
		AST_Block(size_t beg) :AST_Chunk(beg, beg)
		{}
		void AddState(AST_Stat* ps)
		{
			m_vecStat.push_back(ps);
		}
		void SetReturnStat(AST_ReStat* pr)
		{
			m_pReStat = pr;
		}
	private:
		std::vector<AST_Stat*> m_vecStat;
		AST_ReStat* m_pReStat;
	};

	// AST_Stat in ast_stat.h

	class AST_ReStat : public AST_Node
	{
	public:
		AST_ReStat(AST_ExpList* pl, size_t beg, size_t end) :
			m_pExplist(pl), AST_Node(beg, end)
		{}

	private:
		AST_ExpList* m_pExplist;
	};

	class AST_Label : public AST_Node
	{
	public:
		AST_Label(AST_Name* sn, size_t beg, size_t end) :
			AST_Node(beg, end), m_sname(sn)
		{}

	private:
		AST_Name* m_sname;
	};

	class AST_FunCName :public AST_Node
	{
		/* funcname :: = Name{ ¡®.¡¯ Name }[¡®:¡¯ Name] */
	public:
		AST_FunCName(AST_Name* pf, size_t beg) :
			m_pFirst(pf), AST_Node(beg, beg)
		{
			m_pFirst->SetParent(this);
		}

		void AddDotName(AST_Name* pd)
		{
			pd->SetParent(this);
			m_vecDotNamelist.push_back(pd);
		}

		void SetColonName(AST_Name* pc)
		{
			pc->SetParent(this);
			m_pColonName = pc;
		}

	private:
		AST_Name* m_pFirst;
		std::vector<AST_Name*> m_vecDotNamelist;
		AST_Name* m_pColonName;
	};

	class AST_VarList :public AST_Node
	{
	public:
		AST_VarList(AST_Var* pv, size_t beg) :AST_Node(beg, beg)
		{
			pv->SetParent(this);
			m_vecVar.push_back(pv);
		}
		void AddVar(AST_Var* pv)
		{
			pv->SetParent(this);
			m_vecVar.push_back(pv);
		}
	private:
		std::vector<AST_Var*> m_vecVar;
	};

	class AST_NameList : public AST_Node
	{
	public:
		AST_NameList(AST_Name* pv, size_t beg) :AST_Node(beg, beg)
		{
			pv->SetParent(this);
			m_vecName.push_back(pv);
		}
		void AddVar(AST_Name* pv)
		{
			pv->SetParent(this);
			m_vecName.push_back(pv);
		}
	private:
		std::vector<AST_Name*> m_vecName;
	};

	class AST_ExpList : public AST_Node
	{
	public:
		AST_ExpList(AST_Exp* pv, size_t beg) :AST_Node(beg, beg)
		{
			pv->SetParent(this);
			m_vecExp.push_back(pv);
		}
		void AddVar(AST_Exp* pv)
		{
			pv->SetParent(this);
			m_vecExp.push_back(pv);
		}
	private:
		std::vector<AST_Exp*> m_vecExp;
	};

	// AST_Exp in ast_expr.h
	//AST_FunCall in ast_stat.h

	class AST_Args : public AST_Node
	{
	public:
		AST_Args(size_t beg, size_t end) :
			AST_Node(beg, end)
		{}
	};

	class AST_Args_Brack :public AST_Args
	{
	public:
		AST_Args_Brack(AST_ExpList* pe, size_t beg, size_t end) :
			m_pExplist(pe), AST_Args(beg, end)
		{
			m_pExplist->SetParent(this);
		}
	private:
		AST_ExpList* m_pExplist;
	};

	class AST_TableCons;
	class AST_Args_Table :public AST_Args
	{
	public:
		AST_Args_Table(AST_TableCons* pt, size_t beg, size_t end) :
			m_pTableCon(pt), AST_Args(beg, end)
		{}
	private:
		AST_TableCons* m_pTableCon;
	};

	class AST_String;
	class AST_Args_String :public AST_Args
	{
	public:
		AST_Args_String(AST_String* ps, size_t beg, size_t end) :
			m_pString(ps), AST_Args(beg, end)
		{
			m_pString->SetParent(this);
		}
	private:
		AST_String* m_pString;
	};

	class AST_FunBody :public AST_Node
	{
	public:
		AST_FunBody(AST_ParaList* pl, AST_Block* pb, size_t beg, size_t end) :
			m_pParaList(pl), m_pBlock(pb), AST_Node(beg, end){}
	private:
		AST_ParaList* m_pParaList;
		AST_Block* m_pBlock;
	};

	class AST_ParaList : public AST_Node
	{
		/* parlist ::= namelist [¡®,¡¯ ¡®...¡¯] | ¡®...¡¯ */
	public:
		AST_ParaList(size_t beg, size_t end) :
			AST_Node(beg, end)
		{}

	private:
	};

	class AST_ParaList_Name :public AST_ParaList
	{
	public:
		AST_ParaList_Name(AST_NameList* pn, size_t beg, size_t end) :
			AST_ParaList(beg, end), m_pNamelist(pn)
		{}
	private:
		AST_NameList* m_pNamelist;
	};

	class AST_ParaList_3Dot :public AST_ParaList
	{
	public:
		AST_ParaList_3Dot(size_t beg, size_t end) :
			AST_ParaList(beg, end)
		{}
	};

	///
	class AST_Field : public AST_Node
	{
		/* field ::= ¡®[¡¯ exp ¡®]¡¯ ¡®=¡¯ exp | Name ¡®=¡¯ exp | exp */
	public:
		AST_Field(size_t beg, size_t end) :
			AST_Node(beg, end)
		{}
	};

	class AST_Field_Brack : public AST_Field
	{
		/* field ::= ¡®[¡¯ exp1 ¡®]¡¯ ¡®=¡¯ exp2 */
	public:
		AST_Field_Brack(AST_Exp* p1, AST_Exp* p2, size_t beg, size_t end) :
			m_pExp1(p1), m_pExp2(p2), AST_Field(beg, end)
		{}

	private:
		AST_Exp* m_pExp1;
		AST_Exp* m_pExp2;
	};

	class AST_Field_Name : public AST_Field
	{
		/* field ::=  Name ¡®=¡¯ exp */
	public:
		AST_Field_Name(AST_Name* nm, AST_Exp* pe, size_t beg, size_t end) :
			m_pName(nm), m_pExp(pe), AST_Field(beg, end)
		{}

	private:
		AST_Name* m_pName;
		AST_Exp* m_pExp;
	};

	class AST_FieldStep : public AST_Node
	{
		/* fieldsep ::= ¡®,¡¯ | ¡®;¡¯ */
	public:
		AST_FieldStep(AST_Delimiter* de, size_t beg, size_t end) :
			m_pDel(de), AST_Node(beg, end)
		{}
	private:
		AST_Delimiter* m_pDel;
	};

	class AST_FieldList : public AST_Node
	{
		/* fieldlist :: = field{ fieldsep field }[fieldsep] */
	public:
		AST_FieldList(AST_Field* pf, size_t beg, size_t end):
			m_pFiled1(pf), AST_Node(beg,end)
		{}
		
		void AddFFPair(AST_FieldStep* pfs, AST_Field* pfd)
		{
			pfs->SetParent(this);
			pfd->SetParent(this);
			m_vecFFPair.push_back(std::make_pair(pfs, pfd));
		}

		void SetLastFieldStep(AST_FieldStep* pfs)
		{
			pfs->SetParent(this);
			m_pFStep = pfs;
		}
	private:
		AST_Field* m_pFiled1;
		std::vector<std::pair<AST_FieldStep*, AST_Field*>> m_vecFFPair;
		AST_FieldStep* m_pFStep;
	};



	/*****************for statement use************************************/
	class AST_Elseif_Node : public AST_Node
	{
	public:
		AST_Elseif_Node(AST_Exp* pe, AST_Block* pb, size_t beg, size_t end) :
			m_pExp(pe), m_pBlock(pb), AST_Node(beg, end)
		{

		}
	private:
		AST_Exp* m_pExp;
		AST_Block* m_pBlock;
	};
	std::string AST_Elseif_Node::m_descr = "elseif exp then block";

	class AST_Else_Node : public AST_Node
	{
	public:
		AST_Else_Node(AST_Exp* pe, AST_Block* pb, size_t beg, size_t end) :
			m_pBlock(pb), AST_Node(beg, end)
		{

		}
	private:
		AST_Block* m_pBlock;
	};
	std::string AST_Else_Node::m_descr = "else block";

}
#endif