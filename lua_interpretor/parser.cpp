
#include <sstream>
#include "parser.h"


namespace lua_in
{
	/*　运算符优先级，从低到高顺序如下：
	 or
	 and
	 < >  <= >= ~=  ==
	 ..
	 +  -
	 *  /  %
	 not # -
	 ^
	 */

	Parser::Parser(void) :m_curPos(0)
	{
		m_DicBiOperator.insert(std::make_pair("or",  0));
		m_DicBiOperator.insert(std::make_pair("and", 1));
		m_DicBiOperator.insert(std::make_pair("<",   2));
		m_DicBiOperator.insert(std::make_pair(">",   2));
		m_DicBiOperator.insert(std::make_pair("<=",  2));
		m_DicBiOperator.insert(std::make_pair(">=",  2));
		m_DicBiOperator.insert(std::make_pair("~=",  2));
		m_DicBiOperator.insert(std::make_pair("==",  2));
		m_DicBiOperator.insert(std::make_pair("..",  3));
		m_DicBiOperator.insert(std::make_pair("+",   4));
		m_DicBiOperator.insert(std::make_pair("-",   4));
		m_DicBiOperator.insert(std::make_pair("*",   5));
		m_DicBiOperator.insert(std::make_pair("/",   5));
		m_DicBiOperator.insert(std::make_pair("%",   5));
		m_DicBiOperator.insert(std::make_pair("^",   7));
	
		m_DicUnOperator.insert(std::make_pair("not", 6));
		m_DicUnOperator.insert(std::make_pair("#",   6));
		m_DicUnOperator.insert(std::make_pair("-",   6));
	}

	Parser::~Parser(void)
	{

	}

	AST_Stat* Parser::ParserStat()
	{
		if (IsEqualTo(";"))
		{
			return new AST_StatDelim(m_curPos++);
		}
		else if (IsEqualTo("::"))
		{
			return ParserLabel();
		}
		else if (IsEqualTo("break"))
		{
			return new AST_Break(m_curPos++);
		}
		else if (IsEqualTo("goto"))
		{
			return ParserGoto();
		}
		else if (IsEqualTo("do"))
		{
			return ParserDoEnd();
		}
		else if (IsEqualTo("while"))
		{
			return ParserWhile();
		}
		else if (IsEqualTo("repeat"))
		{
			return ParserRepeat();
		}
		else if (IsEqualTo("if"))
		{
			return ParserCondition();
		}
		else if (IsEqualTo("for"))
		{
			return ParserFor();
		}
		else if (IsEqualTo("function"))
		{
			return ParserFunDef();
		}
		else if (IsEqualTo("local") && IsEqualTo(m_curPos + 1, "function"))
		{
			return ParserLocalFunDef();
		}
		else if (IsEqualTo("local"))
		{
			return ParserLocalNameList();
		}
		else
		{
			/*varlist ‘=’ explist | functioncall*/
			size_t flag = m_curPos;
			AST_AssignStat* pAssign = ParserAssignStat();
			if (pAssign)
			{
				return pAssign;
			}
			else // backtrace
			{
				m_curPos = flag;
				m_ErrStack.swap(std::stack<std::string>());
				AST_FunCall* pfuncall = ParserFunCall();
				if (pfuncall)
				{
					return pfuncall;
				}
				else
				{
					PushErrInfo("no match", "'stat'");
					return NULL;
				}
			}
		}
	}

	AST_AssignStat* Parser::ParserAssignStat()
	{
		//try varlist
		size_t bindex = m_curPos;
		AST_VarList* pvarlist = ParserVarList();
		if (!pvarlist)
		{
			PushErrInfo("", "''");
			return NULL;
		}

		if (m_tokenStream[m_curPos].getTokenName() == std::string("="))
		{

			AST_Operator* pop = new AST_Operator("=", m_curPos, m_curPos);
			AST_ExpList* pexplist = ParserExpList();
			if (!pexplist)
			{
				m_ErrStack.push("Error: parser explist in \"varlist = explist\"");
				return NULL;
			}

			AST_AssignStat* ptmp = new AST_AssignStat(pvarlist, pexplist, bindex, m_curPos - 1);

			pvarlist->SetParent(ptmp);
			pop->SetParent(ptmp);
			pexplist->SetParent(ptmp);
			return ptmp;
		}
		else
		{
			m_ErrStack.push("Error: parser = in \"varlist = explist\"");
			return NULL;
		}
	}

	AST_Label* Parser::ParserLabel()
	{
		size_t flag = m_curPos;

		AST_Operator* so1 = new AST_Operator("::", m_curPos, m_curPos);
		m_curPos++;

		/*match name in label pattern*/
		if (m_tokenStream[m_curPos].getTokenType() != T_Identify)
		{
			std::ostringstream ssm;
			ssm << m_tokenStream[m_curPos].toString() << "is not an idenfier";
			m_ErrStack.push(ssm.str());
			return NULL;
		}
		AST_Name* pname = new AST_Name(m_tokenStream[m_curPos].getTokenName(), m_curPos);
		m_curPos++;

		if (!IsEqualTo(m_curPos, "::"))
		{
			m_ErrStack.push("cannot match '::' in label pattern");
			return NULL;
		}
		AST_Operator* so2 = new AST_Operator("::", m_curPos, m_curPos);
		m_curPos++;

		AST_Label* sl = new AST_Label(pname, flag, m_curPos - 1);
		so1->SetParent(sl);
		so2->SetParent(sl);
		pname->SetParent(sl);
		return sl;
	}

	AST_Goto* Parser::ParserGoto()
	{
		size_t flag = m_curPos++;
		if (m_tokenStream[m_curPos].getTokenType() != T_Identify)
		{
			m_ErrStack.push(GetTokenParserErr("parser 'Goto'"));
			return NULL;
		}

		AST_Name* psn = new AST_Name(m_tokenStream[m_curPos].getTokenName(), m_curPos);
		AST_Goto* sg = new AST_Goto(psn, flag, m_curPos++);
		return sg;
	}

	AST_DoEnd* Parser::ParserDoEnd()
	{
		size_t flag = m_curPos++;
		AST_Block* pb = ParserBlock();
		if (!pb)
		{
			m_ErrStack.push("Error: parser block");
			return NULL;
		}

		if (!IsEqualTo("end"))
		{
			m_ErrStack.push(AddLocToErrInfo(GenErrorInfo("end", "'do..end'")));
			return NULL;
		}
		m_curPos++;

		AST_DoEnd* pde = new AST_DoEnd(pb, flag, m_curPos - 1);
		return pde;
	}

	AST_While* Parser::ParserWhile()
	{
		size_t flag = m_curPos++;

		AST_Exp* pe = ParserExp_Base();
		if (!pe)
		{
			m_ErrStack.push(GenErrorInfo("exp", "'while'"));
			return NULL;
		}

		AST_DoEnd* pb = ParserDoEnd();
		if (!pb)
		{
			m_ErrStack.push(GenErrorInfo("do..end", "state"));
			return NULL;
		}

		AST_While* pw = new AST_While(pe, pb, flag, m_curPos - 1);
		return pw;
	}

	AST_Repeat* Parser::ParserRepeat()
	{
		size_t flag = m_curPos++;
		AST_Block* pb = ParserBlock();
		if (!pb)
		{
			m_ErrStack.push(GenErrorInfo("block", "'Repeat'"));
			return NULL;
		}

		if (!IsEqualTo("until"))
		{
			m_ErrStack.push(AddLocToErrInfo(GenErrorInfo("until", "'Repeat'")));
			return NULL;
		}
		m_curPos++;

		AST_Exp* pe = ParserExp_Base();
		if (!pe)
		{
			m_ErrStack.push(GenErrorInfo("exp", "'Repeat'"));
			return NULL;
		}

		AST_Repeat* pr = new AST_Repeat(pb, pe, flag, m_curPos - 1);
		return pr;
	}

	AST_Condition* Parser::ParserCondition()
	{
		size_t flag = m_curPos++;
		AST_Exp* pe = ParserExp_Base();
		if (!pe)
		{
			m_ErrStack.push(GenErrorInfo("exp", "'condition'"));
			return NULL;
		}

		if (!IsEqualTo("then"))
		{
			m_ErrStack.push(GenErrorInfo("then", "'condition'"));
			return NULL;
		}
		m_curPos++;

		AST_Block* pb = ParserBlock();
		if (!pb)
		{
			m_ErrStack.push(GenErrorInfo("block", "'condition'"));
			return NULL;
		}

		AST_Condition* pCon = new AST_Condition(pe, pb, flag, m_curPos - 1);

		while (IsEqualTo("elseif"))
		{
			AST_Elseif_Node* pel = ParserElseIf();
			if (!pel)
			{
				m_ErrStack.push(GenErrorInfo("elseif-then", "'condition'"));
				return NULL;
			}
			pel->SetParent(pCon);
			pCon->AddElseIf(pel);
		}

		if (IsEqualTo("else"))
		{
			AST_Else_Node* pelse = ParserElse();
			if (!pelse)
			{
				m_ErrStack.push(GenErrorInfo("else", "'condition'"));
				return NULL;
			}

			pCon->SetElse(pelse);
			pelse->SetParent(pCon);
		}

		if (!IsEqualTo("end"))
		{
			m_ErrStack.push(GenErrorInfo("end", "'condition'"));
			return NULL;
		}

		m_curPos++;
		return pCon;
	}

	AST_Elseif_Node* Parser::ParserElseIf()
	{
		size_t flag = m_curPos++;

		AST_Exp* pe = ParserExp_Base();
		if (!pe)
		{
			m_ErrStack.push(GenErrorInfo("exp", "'ElseIf'"));
			return NULL;
		}

		if (!IsEqualTo("then"))
		{
			m_ErrStack.push(GenErrorInfo("then", "'ElseIf'"));
			return NULL;
		}

		AST_Block* pb = ParserBlock();
		if (!pb)
		{
			m_ErrStack.push(GenErrorInfo("block", "'ElseIf'"));
			return NULL;
		}

		AST_Elseif_Node* pelseif = new AST_Elseif_Node(pe, pb, flag, m_curPos - 1);
		return pelseif;
	}

	AST_Else_Node* Parser::ParserElse()
	{
		size_t flag = m_curPos++;

		AST_Block * pb = ParserBlock();
		if (!pb)
		{
			m_ErrStack.push(GenErrorInfo("block", "'Else'"));
			return NULL;
		}

		if (!IsEqualTo("end"))
		{
			m_ErrStack.push(GenErrorInfo("end", "'Else'"));
			return NULL;
		}
		m_curPos++;

		AST_Else_Node* pelse = new AST_Else_Node(pb, flag, m_curPos - 1);
		return pelse;
	}

	AST_Stat* Parser::ParserFor()
	{
		size_t flag = m_curPos++;

		//prediction for 3
		if (m_tokenStream[m_curPos].getTokenType() == T_Identify && IsEqualTo(m_curPos + 1, "="))
		{
			AST_Name* pn = new AST_Name(m_tokenStream[m_curPos].getTokenName(), m_curPos);
			m_curPos++; //for =

			AST_Exp* p1 = ParserExp_Base();
			if (!p1)
			{
				m_ErrStack.push(GenErrorInfo("exp1", "for3"));
				return NULL;
			}

			if (!IsEqualTo(","))
			{
				m_ErrStack.push(GenErrorInfo(",", "for3"));
				return NULL;
			}
			m_curPos++;

			AST_Exp* p2 = ParserExp_Base();
			if (!p2)
			{
				PushErrInfo("exp2", "'for3'");
				return NULL;
			}

			AST_Exp* step = NULL;
			if (IsEqualTo(","))
			{
				m_curPos++;
				AST_Exp* step = ParserExp_Base();
				if (!step)
				{
					PushErrInfo("exp step", "'for3'");
					return NULL;
				}
			}

			AST_DoEnd* pdoend = ParserDoEnd();
			if (!pdoend)
			{
				m_ErrStack.push(GenErrorInfo("do...end", "for3"));
				return NULL;
			}

			AST_For3* pfor3 = new AST_For3(pn, p1, p2, step, pdoend, flag, m_curPos - 1);
			return pfor3;
		}
		else
		{
			AST_NameList* pnl = ParserNameList();
			if (!pnl)
			{
				m_ErrStack.push(GenErrorInfo("namelist", "for in"));
				return NULL;
			}

			if (!IsEqualTo("in"))
			{
				m_ErrStack.push(GenErrorInfo("in", "for in"));
				return NULL;
			}
			m_curPos++;



			AST_ExpList* pel = ParserExpList();
			if (!pel)
			{
				m_ErrStack.push(GenErrorInfo("explist", "for in"));
				return NULL;
			}

			AST_DoEnd* pde = ParserDoEnd();
			if (!pde)
			{
				m_ErrStack.push(GenErrorInfo("do...end", "for in"));
				return NULL;
			}

			AST_ForIn* pforin = new AST_ForIn(pnl, pel, pde, flag, m_curPos - 1);
			return pforin;
		}
	}

	AST_FunDef* Parser::ParserFunDef()
	{
		size_t flag = m_curPos++;

		AST_Name* pName = ParserName();
		if (!pName)
		{
			m_ErrStack.push(GenErrorInfo("funcname", "'FunDef'"));
			return NULL;
		}

		AST_FunBody* pf = ParserFunBody();
		if (!pf)
		{
			m_ErrStack.push(GenErrorInfo("funcbody", "'FunDef'"));
			return NULL;
		}

		AST_FunDef* pfd = new AST_FunDef(pName, pf, flag, m_curPos - 1);
		return pfd;
	}

	AST_FunBody* Parser::ParserFunBody()
	{
		/* funcbody ::= ‘(’ [parlist] ‘)’ block end */
		size_t flag = m_curPos;
		if (!IsEqualTo("("))	return NULL;
		m_curPos++;

		AST_ParaList* ppl = NULL;
		if (!IsEqualTo(")"))
		{
			/* have params*/
			ppl = ParserParaList();
			if (!ppl) return NULL;
		}

		if (!IsEqualTo(")")) return NULL;
		m_curPos++;

		AST_Block* pB = ParserBlock();
		if (!pB) return NULL;

		if (!IsEqualTo("end")) return NULL;
		m_curPos++;

		AST_FunBody* pfd = new AST_FunBody(ppl, pB, flag, m_curPos - 1);
		return pfd;
	}

	AST_Local_FunDef* Parser::ParserLocalFunDef()
	{
		size_t flag = m_curPos++;
		AST_FunDef* pfd = ParserFunDef();
		if (!pfd) return NULL;

		AST_Local_FunDef* plf = new AST_Local_FunDef(pfd, flag, m_curPos - 1);
		return plf;
	}

	AST_Local_NameList* Parser::ParserLocalNameList()
	{
		size_t flag = m_curPos++;
		AST_NameList* pnl = ParserNameList();

		if (!pnl) return NULL;

		AST_ExpList* pe = NULL;
		if (IsEqualTo("="))
		{
			m_curPos++;
			pe = ParserExpList();
			if (!pe) return NULL;
		}

		AST_Local_NameList* pln = new AST_Local_NameList(pnl, pe, flag, m_curPos - 1);
		return pln;
	}

	AST_Block* Parser::ParserBlock()
	{
		/* {stat} [retstat] */
		size_t flag = m_curPos;

		AST_Block* pb = new AST_Block(flag, flag);
		AST_Stat* ps = NULL;

		size_t backstrace;
		while (backstrace = m_curPos, ps = ParserStat())
		{
			pb->AddState(ps);
		}

		//backstrace
		m_curPos = backstrace;
		AST_ReStat* pre = NULL;
		if (IsEqualTo("return"))
		{
			pre = ParserReturnStat();
			if (!pre)
			{
				m_ErrStack.push(GenErrorInfo("restat", "'block'"));
				return NULL;
			}
		}

		if (pre)
		{
			pre->SetParent(pb);
		}
		pb->SetReturnStat(pre);
		pb->SetEndPosit(m_curPos - 1);
		return pb;
	}

	AST_ReStat* Parser::ParserReturnStat()
	{
		size_t flag = m_curPos++;

		AST_ExpList* pl = ParserExpList();
		if (!pl)
		{
			return NULL;
		}

		if (IsEqualTo(";"))
		{
			m_curPos++;
		}

		AST_ReStat* pre = new AST_ReStat(pl, flag, m_curPos - 1);
		return pre;
	}

	AST_FunCName* Parser::ParserFunCName()
	{
		size_t flag = m_curPos;
		AST_Name* pn1 = ParserName();
		if (!pn1)
		{
			m_ErrStack.push(GenErrorInfo("Name1", "'funcname'"));
			return NULL;
		}

		AST_FunCName* pfunc = new AST_FunCName(pn1, flag);
		while (IsEqualTo("."))
		{
			m_curPos++;
			AST_Name* pn2 = ParserName();
			if (!pn2)
			{
				m_ErrStack.push(GenErrorInfo("Name2", "'funcname'"));
				return NULL;
			}
			pfunc->AddDotName(pn2);
		}

		if (IsEqualTo(":"))
		{
			AST_Name* pn3 = ParserName();
			if (!pn3)
			{
				m_ErrStack.push(GenErrorInfo("Name3", "'funcname'"));
				return NULL;
			}
			pfunc->SetColonName(pn3);
		}

		return pfunc;
	}

	AST_Args* Parser::ParserArgs()
	{
		/* args ::=  ‘(’ [explist] ‘)’ | tableconstructor | String  */
		size_t flag = m_curPos;

		AST_TableCons* ptable = NULL;

		if (IsEqualTo("("))
		{
			m_curPos++;
			AST_ExpList* pel = ParserExpList();
			if (!pel || !IsEqualTo(")"))
			{
				m_ErrStack.push(GenErrorInfo("(...)", "'args'"));
				return NULL;
			}
			m_curPos++;
			AST_Args_Brack* pab = new AST_Args_Brack(pel, flag, m_curPos - 1);
			return pab;
		}
		else if (IsEqualTo("'") || IsEqualTo("\""))
		{
			AST_String* pstr = ParserString();
			if (!pstr)
			{
				m_ErrStack.push(GenErrorInfo("string", "'args'"));
				return NULL;
			}
			AST_Args_String* pa = new AST_Args_String(pstr, flag, m_curPos - 1);
			return pa;
		}
		else if (ptable = ParserTableCons())
		{
			AST_Args_Table* pt = new AST_Args_Table(ptable, flag, m_curPos - 1);
			return pt;
		}
		else
		{
			m_ErrStack.push(GenErrorInfo("not match", "'args'"));
			return NULL;
		}
	}

	AST_Var* Parser::ParserVar()
	{
		/* to modify...*/
		/* var ::=  Name | prefixexp ‘[’ exp ‘]’ | prefixexp ‘.’ Name  */

		size_t flag = m_curPos;
		if (IsIdentifier()) /* not greedy match*/
		{
			m_curPos++;
			return new AST_Name(m_tokenStream[m_curPos - 1].getTokenName(), flag);
		}

		AST_PrefixExp* ppe = ParserPrefixExp_Fix();
		if (!ppe)
		{
			m_ErrStack.push(GenErrorInfo("prefixexp1", "'var'"));
			return NULL;
		}

		if (IsEqualTo("["))
		{
			m_curPos++;
			AST_Exp* pe = ParserExp_Base();
			if (!pe || IsEqualTo("]"))
			{
				PushErrInfo("#2", "'var'");
				return NULL;
			}
			m_curPos++;
			return new AST_Var_Brack(ppe, pe, flag, m_curPos - 1);
		}

		if (IsEqualTo("."))
		{
			m_curPos++;
			if (!IsIdentifier())
			{
				PushErrInfo("#3-name", "'var'");
				return NULL;
			}
			AST_Name* pn = new AST_Name(m_tokenStream[m_curPos].getTokenName(), m_curPos++);
			return new AST_Var_Dot(ppe, pn, flag, m_curPos - 1);
		}

		PushErrInfo("no match", "'var'");
		return NULL;
	}

	AST_VarList* Parser::ParserVarList()
	{
		size_t flag = m_curPos;
		AST_Var* pv = ParserVar();
		if (!pv)
		{
			PushErrInfo("var", "'varlist'");
			return NULL;
		}

		AST_VarList* pvl = new AST_VarList(pv, flag);

		while (pv = ParserVar())
		{
			pvl->AddVar(pv);
		}
		pvl->SetEndPosit(m_curPos - 1);
		return pvl;
	}

	AST_NameList* Parser::ParserNameList()
	{
		size_t flag = m_curPos;
		AST_Name* pv = ParserName();
		if (!pv)
		{
			PushErrInfo("name", "'namelist'");
			return NULL;
		}

		AST_NameList* pvl = new AST_NameList(pv, flag);

		while (pv = ParserName())
		{
			pvl->AddVar(pv);
		}
		pvl->SetEndPosit(m_curPos - 1);
		return pvl;
	}

	AST_Name* Parser::ParserName()
	{
		if (!IsIdentifier())
		{
			PushErrInfo("not identifier", "...");
			return NULL;
		}

		return new AST_Name(m_tokenStream[m_curPos].getTokenName(), m_curPos++);
	}

	AST_ExpList* Parser::ParserExpList()
	{
		size_t flag = m_curPos;
		AST_Exp* pv = ParserExp_Base();
		if (!pv)
		{
			PushErrInfo("exp", "'explist'");
			return NULL;
		}

		AST_ExpList* pvl = new AST_ExpList(pv, flag);

		while (pv = ParserExp_Base())
		{
			pvl->AddVar(pv);
		}

		pvl->SetEndPosit(m_curPos - 1);
		return pvl;
	}

	AST_Exp* Parser::ParserExp_Base()
	{
		if (IsEqualTo("nil"))
		{
			return new AST_NIL(m_curPos++);
		}
		else if (IsEqualTo("true"))
		{
			return new AST_BOOL(1, m_curPos++);
		}
		else if (IsEqualTo("false"))
		{
			return new AST_BOOL(0, m_curPos++);
		}
		else if (m_tokenStream[m_curPos].getTokenType() == T_ConstNumber)
		{
			return new AST_Number(m_tokenStream[m_curPos].getNumberValue(), m_curPos++);
		}
		else if (m_tokenStream[m_curPos].getTokenType() == T_ConstString)
		{
			return new AST_String(m_tokenStream[m_curPos].getStringValue(), m_curPos++);
		}
		else if (IsEqualTo("..."))
		{
			return new AST_3Dot(m_curPos++);
		}
		else if (AST_LambFunDef* p = ParserLambFunDef())
		{
			return p;
		}
		else if (AST_PrefixExp* pre = ParserPrefixExp_Fix())
		{
			return pre;
		}
		else if (AST_TableCons* pt = ParserTableCons())
		{
			return pt;
		}
		else
		{
			PushErrInfo("not match", "'exp'");
			return NULL;
		}
	}

	AST_PrefixExp* Parser::ParserPrefixExp_Fix()
	{
		/*
		左递归和公因子文法
		prefixexp ::= var | functioncall | ‘(’ exp ‘)’
		functioncall ::=  prefixexp args | prefixexp ‘:’ Name args
		======
		消除左递归和公因子
		prefixexp     :: = ( var | '(' exp ')' )    prefixexp_fix
		prefixexp_fix :: = ( args | ':' Name args ) prefixexp_fix | epsilon

		函数调用蕴含其中
		f g x
		f(g(x)) ? (f(g))(x) ? f(g, x)
		根据args的产生式， 排除第三种f(g, x)
		在前两中，根据functioncall产生式选择： (f(g))(x)
		*/

		size_t flag = m_curPos;

		AST_Brack_Exp* pBExp = NULL;
		AST_Var* pVar = NULL;
		if (IsEqualTo("("))
		{
			m_curPos++;

			AST_Exp* pExp = ParserExp_Base();
			if (!pExp)
			{
				PushErrInfo("exp", "'prefixexp'");
				return NULL;
			}

			if (!IsEqualTo(")"))
			{
				PushErrInfo(")", "'prefixexp'");
				return NULL;
			}
			pBExp = new AST_Brack_Exp(pExp, flag, m_curPos++);
		}
		else
		{
			pVar = ParserVar();
			if (!pVar)
			{
				PushErrInfo("var", "'prefixexp'");
				return NULL;
			}
		}


		/*****recursive => iterate*******/

		//AST_PrefixExp* pPre = pVar ? pVar : pBExp ;
		AST_PrefixExp* pPre = NULL;
		if (pVar)
			pPre = pVar;
		else
			pPre = pBExp;

		AST_FunCall* pFuncall = NULL;
		// f g x =>  f(g) x => (f(g))x
		while (true)
		{
			AST_Args* pArgs = NULL;
			AST_Name* pName = NULL;
			if (IsEqualTo(":")) // ':' Name args
			{
				m_curPos++;
				pName = ParserName();
				pArgs = ParserArgs();
				if (!pName || !pArgs)
				{
					PushErrInfo(": Name args", "'functioncall'");
					return NULL;
				}
				pFuncall = new AST_FunCallName(pPre, pName, pArgs, flag, m_curPos - 1);
			}
			else if (pArgs = ParserArgs()) // args
			{
				pFuncall = new AST_FunCallNoName(pPre, pArgs, flag, m_curPos - 1);
			}
			else
			{
				return pPre;
			}

			pPre = pFuncall;
		}
	}

	AST_FunCall* Parser::ParserFunCall()
	{
		AST_PrefixExp* p = ParserPrefixExp_Fix();
		return dynamic_cast<AST_FunCall*>(p);
	}

	AST_LambFunDef* Parser::ParserLambFunDef()
	{
		if (!IsEqualTo("function"))
		{
			PushErrInfo("function", "'functiondef'");
			return NULL;
		}
		size_t flag = m_curPos++;
		AST_FunBody* pfunb = ParserFunBody();
		if (!pfunb)
		{
			PushErrInfo("funcbody", "'functiondef'");
			return NULL;
		}

		return new AST_LambFunDef(pfunb, flag, m_curPos - 1);
	}

	AST_ParaList* Parser::ParserParaList()
	{
		/* parlist ::= namelist [‘,’ ‘...’] | ‘...’ */
		size_t flag = m_curPos;

		AST_NameList* pnlist = ParserNameList();
		if (pnlist)
		{
			if (IsEqualTo(","))
			{
				m_curPos++;
				if (!IsEqualTo("..."))
				{
					PushErrInfo("namelist: ...", "'parlist'");
					return NULL;
				}
				m_curPos++;

				return new AST_ParaList_Name_Dot(pnlist, flag, m_curPos - 1);
			}
			return new AST_ParaList_Name(pnlist, flag, m_curPos - 1);
		}
		else
		{
			if (IsEqualTo("..."))
			{
				return new AST_ParaList_3Dot(flag, m_curPos - 1);
			}
		}

		return NULL;
	}

	AST_TableCons* Parser::ParserTableCons()
	{
		if (!IsEqualTo("{"))
		{
			PushErrInfo("{", "'tableconstructor'");
			return NULL;
		}
		size_t flag = m_curPos++;

		AST_FieldList* pflist = ParserFieldList();
		if (pflist)
		{
			if (!IsEqualTo("}"))
			{
				PushErrInfo("}", "'tableconstructor'");
				return NULL;
			}
			return new AST_TableCons(pflist, flag, m_curPos++);
		}
		else if (IsEqualTo(flag + 1, "}")) //blank table
		{
			return new AST_TableCons(NULL, flag, flag + 1);
		}
		else
		{
			PushErrInfo("cannot match", "'tableconstructor'");
			return NULL;
		}
	}

	AST_FieldList* Parser::ParserFieldList()
	{
		/* fieldlist ::= field {fieldsep field} [fieldsep] */
		size_t flag = m_curPos;
		AST_Field* pfield = ParserField();
		if (!pfield)
		{
			PushErrInfo("1st field", "'fieldlist'");
			return NULL;
		}
		AST_FieldList* pflist = new AST_FieldList(pfield, flag, m_curPos - 1);

		while (true)
		{
			size_t cpoint = m_curPos; // backstrace point
			AST_FieldStep* pstep = ParserFieldStep();
			if (!pstep)
			{
				m_curPos = cpoint;
				break;
			}

			AST_Field* pfie = ParserField();
			if (!pfie)
			{
				PushErrInfo("field", "'fieldlist'");
				return NULL;
			}
			pflist->AddFFPair(pstep, pfie);
		}

		size_t bt = m_curPos;
		AST_FieldStep* pstep2 = ParserFieldStep();
		if (pstep2)
		{
			pflist->SetLastFieldStep(pstep2);
			return pflist;
		}

		m_curPos = bt;
		return pflist;
	}

	AST_Field* Parser::ParserField()
	{
		/* field ::= ‘[’ exp ‘]’ ‘=’ exp | Name ‘=’ exp | exp */
		// maybe need to modify
		size_t flag = m_curPos;
		if (IsEqualTo("["))
		{
			m_curPos++;

			AST_Exp* pExp = ParserExp_Base();
			if (!pExp)
			{
				PushErrInfo("exp1", "'field'");
				return NULL;
			}

			if (!IsEqualTo("]") || !IsEqualTo(m_curPos + 1, "="))
			{
				PushErrInfo("]=", "'field'");
				return NULL;
			}
			m_curPos += 2;

			AST_Exp* pExp2 = ParserExp_Base();
			if (!pExp2)
			{
				PushErrInfo("exp2", "'field'");
				return NULL;
			}

			return new AST_Field_Brack(pExp, pExp2, flag, m_curPos - 1);
		}
		else if (IsIdentifier() && IsEqualTo(m_curPos + 1, "="))
		{
			AST_Name* pName = ParserName();
			m_curPos++;
			AST_Exp* pExp = ParserExp_Base();
			if (!pExp)
			{
				PushErrInfo("exp3", "'field'");
				return NULL;
			}

			return new AST_Field_Name(pName, pExp, flag, m_curPos - 1);
		}
		else if (AST_Exp* pExp = ParserExp_Base())
		{
			return pExp;
		}
		return NULL;
	}

	AST_FieldStep* Parser::ParserFieldStep()
	{
		if (!IsEqualTo(",") && !IsEqualTo(";"))
		{
			PushErrInfo("not match", "'fieldsep'");
			return NULL;
		}

		return new AST_FieldStep(m_tokenStream[m_curPos].getTokenName(), m_curPos++);
	}

	AST_BiOp* Parser::ParserBiOp()
	{
		if (IsBiOperator())
		{
			std::string str = m_tokenStream[m_curPos].getTokenName();
			return new AST_BiOp(str, LookUpOptPri(str), m_curPos++);
		}
		PushErrInfo("do not support this bioperator", "'binop'");
		return NULL;
	}

	AST_UnOp* Parser::ParserUnOp()
	{
		if (IsUnOperator())
		{
			std::string str = m_tokenStream[m_curPos].getTokenName();
			return new AST_UnOp(str, LookUpOptPri(str), m_curPos++);
		}
		PushErrInfo("do not support this unoperator", "'unop'");
		return NULL;
	}

	AST_Exp* Parser::ParserExp_Fix()
	{
		/* Reverse Polish notation */
		std::stack<AST_Operator*> stack_optor;
		std::deque<AST_Node*>     deResult;

		// -3+4*5^2-3
		// -3+-4
		// 不考虑（）
		// -3+5;  3-5+
		// 3+-5 
		// --4
		// -3^-2^2 + 4    3 4 - ^ -
		// 先生成逆波兰序，再转AST
		while (true)
		{
			AST_Exp* pBase = NULL;

			//operator
			size_t bt = m_curPos;
			if (AST_Operator* op = ParserOperator())
			{
				if (IsPushToOptStack(stack_optor, op))// unopt, 2^-2^2 ,1+-3
				{
					stack_optor.push(op);
				}
				else
				{
					while (!IsPushToOptStack(stack_optor, op))
					{
						deResult.push_back(stack_optor.top());
						stack_optor.pop();
					}
					stack_optor.push(op);
				}
			}
			// base element 
			else if (m_curPos = bt, pBase = ParserExp_Base())
			{
				deResult.push_back(pBase);
			}
			else
			{
				while (!stack_optor.empty())
				{
					deResult.push_back(stack_optor.top());
					stack_optor.pop();
				}
				m_curPos = bt;
				break;
			}
		}


		// convert to AST,  need to add index
		// the full form: -3 + -4 => 3-4-+
		while (deResult.size() > 1)
		{
			//3
			AST_Exp* pE1 = dynamic_cast<AST_Exp*>(deResult.front());
			if (!pE1)
			{
				PushErrInfo("error", "convert reverse polish notation to AST");
				return NULL;
			}
			deResult.pop_front();

			// 3-
			AST_UnOp* uop1 = NULL;
			if (uop1 = dynamic_cast<AST_UnOp*>(deResult.front()))
			{
				deResult.pop_front();
				AST_Exp* puExp = new AST_UnExpr(pE1, uop1, 0, 0);
				deResult.push_front(puExp);
				continue;
			}

			//3- 4
			AST_Exp* pE2 = dynamic_cast<AST_Exp*>(deResult.front());
			if (!pE2)
			{
				PushErrInfo("error", "'convert reverse polish notation to AST'");
				return NULL;
			}
			deResult.pop_front();

			//"3-4+", "3-4-+"
			AST_Operator* op2 = dynamic_cast<AST_Operator*>(deResult.front());
			if (AST_UnOp* uop2 = dynamic_cast<AST_UnOp*>(op2)) //unope
			{
				deResult.pop_front();
				AST_Exp* pE2_tmp = new AST_UnExpr(pE2, uop2, 0, 0);
				deResult.push_front(pE2_tmp);
				deResult.push_front(pE1); // restart
			}
			else if (AST_BiOp* bop = dynamic_cast<AST_BiOp*>(op2))
			{
				deResult.pop_front();
				AST_Exp* pE = new AST_BiExpr(pE1, pE2, bop, 0, 0);
				deResult.push_front(pE);// resatrt
			}
			else
			{
				PushErrInfo("error: op2", "'convert reverse polish notation to AST'");
				return NULL;
			}
		}

		if (deResult.size() == 1)
		{
			return dynamic_cast<AST_Exp*>(deResult.front());
		}
		else
		{
			PushErrInfo("error", "reverse polish notation");
			return NULL;
		}
	}

	AST_Exp* Parser::ParserExp()
	{
		/*
		exp ::=  nil | false | true | Number | String | ‘...’ | functiondef |
		prefixexp | tableconstructor | exp binop exp | unop exp
		*/
		/*
		left-recursive and priority for calculate
		So, change it and add  Reverse Polish notation
		*/

		return ParserExp_Fix();
	}

	AST_String* Parser::ParserString()
	{
		if (m_tokenStream[m_curPos].getTokenType() == T_ConstString)
		{
			return new AST_String(m_tokenStream[m_curPos].getStringValue(), m_curPos++);
		}

		return NULL;
	}

	AST_Operator* Parser::ParserOperator()
	{
		size_t flag  = m_curPos;
		AST_Operator* op = ParserBiOp();
		if (!op)
		{
			m_curPos = flag;
			return ParserUnOp();
		}

		return op;
	}


}