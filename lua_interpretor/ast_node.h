#ifndef AST_NODE_H__
#define AST_NODE_H__

#include <string>
#include <vector>
#include "Token.h"

namespace lua_in
{
	class AST_Stat;
	class AST_ReStat;
	class AST_Exp;
	class AST_Var;
	class AST_ExpList;
	class AST_Name;

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
	
	protected:
		size_t        m_begLoc;
		size_t        m_endLoc;
		AST_Node*     m_parent;
	};

	/***************************basic elements*********************************************/
	class AST_Operator : public AST_Node
	{
	public:
		AST_Operator(std::string nm, int pri, size_t beg) : m_name(nm), m_priority(pri), AST_Node(beg, beg)
		{}

		inline int GetPriority() const
		{
			return m_priority;
		}

		inline std::string GetName() const
		{
			return m_name;
		}

		int ComparePriority(AST_Operator* op);
		bool IsSameName(AST_Operator* op);

	protected:
		std::string m_name;
		int m_priority;
	};

	class AST_BiOp : public AST_Operator
	{
	public:
		AST_BiOp(std::string op, int pri, size_t beg) :
			AST_Operator(op, pri, beg)
		{}
	private:
		static std::string m_grammar;
	};

	class AST_UnOp : public AST_Operator
	{
	public:
		AST_UnOp(std::string op, int pri, size_t beg) :
			AST_Operator(op, pri, beg)
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
		static std::string m_grammar;
	};

	/***********************************************/
	class AST_Chunk :public AST_Node
	{
	public:
		AST_Chunk(size_t beg, size_t end) :
			AST_Node(beg, end)
		{
		}
	private:
		static std::string m_grammar;
	};

	class AST_Block :public AST_Chunk
	{
	public:
		AST_Block(size_t beg, size_t end) :AST_Chunk(beg, end)
		{
		}
		void AddState(AST_Stat* ps);
		void SetReturnStat(AST_ReStat* pr)
		{
			m_pReStat = pr;
		}
	private:
		std::vector<AST_Stat*> m_vecStat;
		AST_ReStat* m_pReStat;
		static std::string m_grammar;
	};

	// AST_Stat in ast_stat.h

	class AST_ReStat : public AST_Node
	{
	public:
		AST_ReStat(AST_ExpList* pl, size_t beg, size_t end);

	private:
		AST_ExpList* m_pExplist;
		static std::string m_grammar;
	};



	class AST_FunCName :public AST_Node
	{
		/* funcname :: = Name{ ¡®.¡¯ Name }[¡®:¡¯ Name] */
	public:
		AST_FunCName(AST_Name* pf, size_t beg);

		void AddDotName(AST_Name* pd);

		void SetColonName(AST_Name* pc);

	private:
		AST_Name* m_pFirst;
		std::vector<AST_Name*> m_vecDotNamelist;
		AST_Name* m_pColonName;
	};

	class AST_VarList :public AST_Node
	{
	public:
		AST_VarList(AST_Var* pv, size_t beg);
		void AddVar(AST_Var* pv);
	private:
		std::vector<AST_Var*> m_vecVar;
	};

	class AST_NameList : public AST_Node
	{
	public:
		AST_NameList(AST_Name* pv, size_t beg);
		void AddVar(AST_Name* pv);
	private:
		std::vector<AST_Name*> m_vecName;
	};

	class AST_ExpList : public AST_Node
	{
	public:
		AST_ExpList(AST_Exp* pv, size_t beg);
		void AddVar(AST_Exp* pv);
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
	private:
		static std::string m_grammar;
	};

	class AST_Args_Brack :public AST_Args
	{
	public:
		AST_Args_Brack(AST_ExpList* pe, size_t beg, size_t end);
	private:
		AST_ExpList* m_pExplist;
		static std::string m_grammar;
	};

	class AST_TableCons;
	class AST_Args_Table :public AST_Args
	{
	public:
		AST_Args_Table(AST_TableCons* pt, size_t beg, size_t end);
	private:
		AST_TableCons* m_pTableCon;
		static std::string m_grammar;
	};

	class AST_String;
	class AST_Args_String :public AST_Args
	{
	public:
		AST_Args_String(AST_String* ps, size_t beg, size_t end);
	private:
		AST_String* m_pString;
		static std::string m_grammar;
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

	class AST_FunBody :public AST_Node
	{
	public:
		AST_FunBody(AST_ParaList* pl, AST_Block* pb, size_t beg, size_t end);
	private:
		AST_ParaList* m_pParaList;
		AST_Block* m_pBlock;
	};

	class AST_ParaList_Name :public AST_ParaList
	{
	public:
		AST_ParaList_Name(AST_NameList* pn, size_t beg, size_t end);
	private:
		AST_NameList* m_pNamelist;
	};

	class AST_ParaList_Name_Dot :public AST_ParaList
	{
	public:
		AST_ParaList_Name_Dot(AST_NameList* pn, size_t beg, size_t end);
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
		AST_Field_Brack(AST_Exp* p1, AST_Exp* p2, size_t beg, size_t end);

	private:
		AST_Exp* m_pExp1;
		AST_Exp* m_pExp2;
	};

	class AST_Field_Name : public AST_Field
	{
		/* field ::=  Name ¡®=¡¯ exp */
	public:
		AST_Field_Name(AST_Name* nm, AST_Exp* pe, size_t beg, size_t end);

	private:
		AST_Name* m_pName;
		AST_Exp* m_pExp;
	};

	class AST_FieldStep : public AST_Node
	{
		/* fieldsep ::= ¡®,¡¯ | ¡®;¡¯ */
	public:
		AST_FieldStep(std::string de, size_t beg) :
			m_delimiter(de), AST_Node(beg, beg)
		{}
	private:
		std::string m_delimiter;
	};

	class AST_FieldList : public AST_Node
	{
		/* fieldlist :: = field{ fieldsep field }[fieldsep] */
	public:
		AST_FieldList(AST_Field* pf, size_t beg, size_t end);

		void AddFFPair(AST_FieldStep* pfs, AST_Field* pfd);
		void SetLastFieldStep(AST_FieldStep* pfs);

	private:
		AST_Field* m_pFiled1;
		std::vector<std::pair<AST_FieldStep*, AST_Field*>> m_vecFFPair;
		AST_FieldStep* m_pFStep;
	};



	/*****************for statement use************************************/
	class AST_Elseif_Node : public AST_Node
	{
	public:
		AST_Elseif_Node(AST_Exp* pe, AST_Block* pb, size_t beg, size_t end);
	private:
		AST_Exp* m_pExp;
		AST_Block* m_pBlock;
	};

	class AST_Else_Node : public AST_Node
	{
	public:
		AST_Else_Node(AST_Block* pb, size_t beg, size_t end);
	private:
		AST_Block* m_pBlock;
	};

}
#endif