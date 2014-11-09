#ifndef PARSER_H__
#define  PARSER_H__

#include <vector>
#include <string>
#include <map>
#include <stack>
#include <sstream>

#include "Token.h"
#include "ast_node.h"
#include "ast_stat.h"
#include "ast_expr.h"

namespace lua_in
{
	class Parser
	{
	public:
		Parser(void);
		~Parser(void);

		//chunk & block
		AST_Block*        ParserBlock();

		//Statement//
		AST_Stat*         ParserStat();
		AST_AssignStat*      ParserAssignStat();
		AST_Goto*		     ParserGoto();
		AST_DoEnd*           ParserDoEnd();
		AST_While*           ParserWhile();
		AST_Repeat*          ParserRepeat();
		AST_Condition*       ParserCondition();
		AST_Elseif_Node*     ParserElseIf();
		AST_Else_Node*       ParserElse();
		AST_Stat*            ParserFor();
		AST_FunDef*          ParserFunDef();
		AST_Local_FunDef*    ParserLocalFunDef();
		AST_Local_NameList*  ParserLocalNameList();

		//restate
		AST_ReStat*        ParserReturnStat();
		AST_Label*		   ParserLabel();
		AST_FunCName*      ParserFunCName();
		AST_VarList*       ParserVarList();
		AST_Var*           ParserVar(); 
		AST_NameList*      ParserNameList();
		AST_ExpList*       ParserExpList();
		AST_Exp*           ParserExp();
		AST_PrefixExp*     ParserPrefixExp();//
		AST_FunCall*       ParserFunCall();//
		AST_Args*          ParserArgs();
		AST_LambFunDef*    ParserLambFunDef(); // 
		AST_FunBody*       ParserFunBody();
		AST_ParaList*      ParserParaList();//
		AST_TableCons*     ParserTableCons();//
		AST_FieldList*     ParserFieldList();//
		AST_Field*         ParserField();//
		AST_FieldStep*     ParserFieldStep();//
		AST_BiOp*          ParserBiOp();//
		AST_UnOp*          ParserUnOp();//

		//
		AST_Name*          ParserName();
		AST_String*        ParserString();//
		AST_Number*        ParserNumber();//
		AST_BiExpr*        ParserBiExpr();
		AST_UnExpr*        ParserUnExpr();

	private:
		inline bool IsEqualTo(size_t index, const std::string& str) const
		{
			return m_tokenStream[index].getTokenName() == str;
		}

		inline bool IsEqualTo(const std::string& str) const
		{
			return m_tokenStream[m_curPos].getTokenName() == str;
		}

		bool IsIdentifier()
		{
			return m_tokenStream[m_curPos].getTokenType() == T_Identify;
		}

		void PushErrInfo(std::string elem, std::string pattern)
		{
			m_ErrStack.push(GenErrorInfo(elem, pattern));
		}

		std::string GetTokenParserErr(const std::string& rsn) const
		{
			std::ostringstream osm;
			osm << "Error: " << m_tokenStream[m_curPos].getTokenLocation().toString();
			osm << rsn;
			return osm.str();
		}

		std::string AddLocToErrInfo(const std::string& rsn) const
		{
			std::string str = rsn;
			str += m_tokenStream[m_curPos].getTokenLocation().toString();
			return str;
		}

		std::string GenErrorInfo(const std::string& elem, const std::string& pattern) const
		{
			std::ostringstream osm;
			osm << "Error: '" << elem << "' in '" << pattern << "'";
			return osm.str();
		}


	private:
		std::vector<Token> m_tokenStream;
		size_t m_curPos;
		enum Parser_State{
			PS_Beg,
			PS_End,
			PS_Error, // 回溯用
			PS_Accept,
			PS_Fatal  // 终止parser工作
		};
		Parser_State m_parStat;
		std::stack<std::string> m_ErrStack;
	};

}
#endif
