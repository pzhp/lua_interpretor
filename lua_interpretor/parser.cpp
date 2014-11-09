
#include <sstream>
#include "parser.h"


namespace lua_in
{

Parser::Parser(void):m_curPos(0)
{
}

Parser::~Parser(void)
{

}

AST_Stat* Parser::ParserStat()
{
	if (IsEqualTo(";"))
	{
		m_curPos++;
		return new AST_Delim(";", m_curPos, m_curPos);
	}
	else if(IsEqualTo("::"))
	{
		return ParserLabel();
	}
	else if (IsEqualTo("break"))
	{
		return new AST_Break("break", m_curPos, m_curPos++);
	}
	else if(IsEqualTo("goto"))
	{
        return ParserGoto();
	}
	else if(IsEqualTo("do"))
    {
        return ParserDoEnd();
    }
    else if(IsEqualTo("while"))
    {
        return ParserWhile();
    }
    else if(IsEqualTo("repeat"))
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
	else if (IsEqualTo("local") && IsEqualTo(m_curPos+1,"function"))
	{
		return ParserLocalFunDef();
	}
	else if (IsEqualTo("local"))
	{
		return ParserLocalNameList();
	}
    else
	{
		/*varlist ¡®=¡¯ explist | functioncall*/		
		size_t flag = m_curPos;
		AST_AssignStat* pAssign = ParserAssignStat();
		if (pAssign)
		{
			return pAssign;
		}
		else // backtrace
		{
			m_curPos = flag;
			m_ErrStack.clear();
			AST_FunCall* pfuncall = ParserFunCall();
			if (pfuncall)
			{
				return pfuncall;
			}
			else
			{
				m_ErrStack.push("cannot match syntax pattern");
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
	if(!pvarlist)
	{
		m_ErrStack.push("Error: parser varlist in \"varlist = explist\"");
		return NULL;
	}
	
	if (m_tokenStream[m_curPos] == std::string("="))
	{
		
		AST_Operator* pop = new AST_Operator("=",m_curPos,m_curPos);
		AST_ExpList* pexplist = ParserExpList();
		if(!pexplist) 
		{
			m_ErrStack.push("Error: parser explist in \"varlist = explist\"");
			return NULL;
		}
		
		AST_AssignStat* ptmp = new AST_AssignStat(pvarlist, pexplist,, bindex, m_curPos-1);
		
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
		ssm<<m_tokenStream[m_curPos].toString()<<"is not an idenfier";
		m_ErrStack.push(ssm.str());
		return NULL;
	}
	AST_Name* pname = new AST_Name(m_tokenStream[m_curPos].getTokenName(),m_curPos,m_curPos);
	m_curPos++;

	if(!IsEqualTo(m_curPos,"::"))
	{
		m_ErrStack.push("cannot match '::' in label pattern");
		return NULL;
	}
	AST_Operator* so2 = new AST_Operator("::", m_curPos, m_curPos);
	m_curPos++;

	AST_Label* sl = new AST_Label(so1, pname, so2, flag, m_curPos-1);
	so1->SetParent(sl);
	so2->SetParent(sl);
	pname->SetParent(sl);
	return sl;
}

AST_Stat* Parser::ParserGoto()
{
	size_t flag = m_curPos++;
	if (m_tokenStream[m_curPos].getTokenType() != T_Identify)
	{
		m_ErrStack.push(GetTokenParserErr("parser 'Goto'"));
		return NULL;
	}

	AST_Name* psn = new AST_Name(m_tokenStream[m_curPos].getTokenName(),m_curPos);
	AST_Goto* sg = new AST_Goto(psn,flag,m_curPos++);
	psn->SetParent(sg);
	return psn;	
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
        m_ErrStack.push(AddLocToErrInfo(GenErrorInfo("end",AST_DoEnd::m_descr)));
        return NULL;
    }
    m_curPos++;

    AST_DoEnd* pde = new AST_DoEnd(pb,flag,m_curPos-1);
    pb->SetParent(pde);
    return pb;
}

AST_While* Parser::ParserWhile()
{
    size_t flag = m_curPos++;

    AST_Exp* pe = ParserExp();
    if (!pe)
    {
        m_ErrStack.push(GenErrorInfo("exp",AST_While::m_descr));
        return NULL;
    }
    
    AST_Block* pb = ParserDoEnd();
    if (!pb)
    {
        m_ErrStack.push(GenErrorInfo("do..end", AST_While::m_descr));
        return NULL;
    }
    
    AST_While* pw = new AST_While(pe, pb, flag, m_curPos-1);
    pe->SetParent(pw);
    pb->SetParent(pw);
    return pw;
}

AST_Repeat* Parser::ParserRepeat()
{
    size_t flag = m_curPos++;
    AST_Block* pb = ParserBlock();
    if (!pb)
    {
        m_ErrStack.push(GenErrorInfo("block",AST_Repeat::m_descr));
        return NULL;
    }

    if (!IsEqualTo("until"))
    {
        m_ErrStack.push(AddLocToErrInfo(GenErrorInfo("until", AST_Repeat::m_descr)));
        return NULL;
    }    
    m_curPos++;

    AST_Exp* pe = ParserExp();
    if (!pe)
    {
        m_ErrStack.push(GenErrorInfo("exp",AST_Repeat::m_descr));
        return NULL;
    }

    AST_Repeat* pr = new AST_Repeat(pb, pe, flag, m_curPos-1);
    pb->SetParent(pr);
    pe->SetParent(pr);
    return pr;
}

AST_Condition* Parser::ParserCondition()
{
    size_t flag = m_curPos++;
    AST_Exp* pe = ParserExp();
    if (!pe)
    {
        m_ErrStack.push(GenErrorInfo("exp",AST_Condition::m_descr));
        return NULL;
    }

    if (!IsEqualTo("then"))
    {
        m_ErrStack.push(GenErrorInfo("then",AST_Condition::m_descr));
        return NULL;
    }
    m_curPos++;

    AST_Block* pb = ParserBlock();
    if (!pb)
    {
        m_ErrStack.push(GenErrorInfo("block",AST_Condition::m_descr));
        return NULL;
    }
    
    AST_Condition* pCon = new AST_Condition(pe, pb, flag, m_curPos-1);

    while (IsEqualTo("elseif"))
    {
        AST_Elseif_Node* pel = ParserElseIf();
        if (!pel)
        {
            m_ErrStack.push(GenErrorInfo("elseif-then",AST_Condition::m_descr));
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
           m_ErrStack.push(GenErrorInfo("else", AST_Condition::m_descr));
           return NULL;
       }

        pCon->SetElse(pelse);
        pelse->SetParent(pCon);
    }

    if (!IsEqualTo("end"))
    {
        m_ErrStack.push(GenErrorInfo("end",AST_Condition::m_descr));
        return NULL;
    }
    
    m_curPos++;
    return pCon;
}

AST_Elseif_Node* Parser::ParserElseIf()
{
    size_t flag = m_curPos++;

    AST_Exp* pe = ParserExp();
    if (!pe)
    {
        m_ErrStack.push(GenErrorInfo("exp", AST_Elseif_Node::m_descr));
        return NULL;
    }

    if(!IsEqualTo("then"))
    {
        m_ErrStack.push(GenErrorInfo("then", AST_Elseif_Node::m_descr));
        return NULL;
    }
    
    AST_Block* pb = ParserBlock();
    if (!pb)
    {
        m_ErrStack.push(GenErrorInfo("block", AST_Elseif_Node::m_descr));
        return NULL;
    }

    AST_Elseif_Node* pelseif = new AST_Elseif_Node(pe, pb, flag, m_curPos-1);
    pe->SetParent(pelseif);
    pb->SetParent(pelseif);
    return pelseif;
}

AST_Else_Node* Parser::ParserElse()
{
    size_t flag = m_curPos++;
   
    AST_Block * pb = ParserBlock();
    if (!pb)
    {
        m_ErrStack.push(GenErrorInfo("block",AST_Else_Node::m_descr));
        return NULL;
    }
    
    if (!IsEqualTo("end"))
    {
        m_ErrStack.push(GenErrorInfo("end",AST_Else_Node::m_descr));
        return NULL;
    }
    m_curPos++;

    AST_Else_Node* pelse = new AST_Else_Node(pelse, flag, m_curPos-1);
    pb->SetParent(pelse);
    return pelse;    
}

AST_Stat* Parser::ParserFor()
{
    size_t flag = m_curPos++;
    
    //prediction for 3
    if(m_tokenStream[m_curPos].getTokenType() == T_Identify && IsEqualTo(m_curPos+1,"="))
    {
        AST_Name* pn = new AST_Name(m_tokenStream[m_curPos].getTokenName(),m_curPos, m_curPos);
        m_curPos++; //for =

        AST_Exp* p1 = ParserExp();
        if (!p1)
        {
            m_ErrStack.push(GenErrorInfo("exp1","for3"))
            return NULL;
        }

        if (!IsEqualTo(","))
        {
            m_ErrStack.push(GenErrorInfo(",","for3"));
            return NULL;
        }
        m_curPos++;

        AST_Exp* p2 = ParserExp();
        if (!p2)
        {
            m_ErrStack.push("exp2","for3");
            return NULL;
        }
        
        AST_Exp* step = NULL;
        if (IsEqualTo(","))
        {
            m_curPos++;
            AST_Exp* step = ParserExp();
            if (!step)
            {
                m_ErrStack.push("exp step","for3");
                return NULL;
            }
        }

        AST_DoEnd* pdoend = ParserDoEnd();
        if (!pdoend)
        {
            m_ErrStack.push(GenErrorInfo("do...end","for3"));
            return NULL;
        }

        AST_Stat* pfor3 = new AST_For3(p1, p2, step, pdoend, flag, m_curPos-1);
        p1->SetParent(pfor3);
        p2->SetParent(pfor3);
        if(step) step->SetParent(pfor3);
        pdoend->SetParent(pfor3);
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

        AST_ForIn* pforin = new AST_ForIn(pnl,pel, pde, flag, m_curPos-1);
        pnl->SetParent(pforin);
        pel->SetParent(pforin);
        pde->SetParent(pforin);
        return pforin;
    }   
}

AST_FunDef* Parser::ParserFunDef()
{
	size_t flag = m_curPos++;
	
	AST_Name* pName = ParserName();
	if (!pName)
	{
		m_ErrStack.push(GenErrorInfo("funcname", AST_FunDef::m_descr));
		return NULL;
	}

	AST_FunBody* pf = ParserFunBody();
	if (!pf)
	{
		m_ErrStack.push(GenErrorInfo("funcbody", AST_FunDef::m_descr));
		return NULL;
	}

	AST_FunDef* pfd = new AST_FunDef(pName, pf, flag, m_curPos - 1);
	pName->SetParent(pfd);
	pf->SetParent(pfd);
	return pfd;
}

AST_FunBody* Parser::ParserFunBody()
{
	/* funcbody ::= ¡®(¡¯ [parlist] ¡®)¡¯ block end */
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

	AST_FunBody* pfd = new AST_FunBody(ppl, pB, flag, m_curPos-1);
	ppl->SetParent(pfd);
	pB->SetParent(pfd);
	return pfd;
}

AST_Local_FunDef* Parser::ParserLocalFunDef()
{
	size_t flag = m_curPos++;
	AST_FunDef* pfd = ParserFunDef();
	if (!pfd) return NULL;
	
	AST_Local_FunDef* plf = new AST_Local_FunDef(pfd, flag, m_curPos-1);
	pfd->SetParent(plf);
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

	AST_Local_NameList* pln = new AST_Local_NameList(pnl, pe, flag, m_curPos-1);
	pnl->SetParent(pln);
	pe->SetParent(pln);
	return pln;
}

AST_Block* Parser::ParserBlock()
{
	/* {stat} [retstat] */
	size_t flag = m_curPos;

	AST_Block* pb = new AST_Block(flag);
	AST_Stat* ps = NULL;
	
	size_t backstrace; 
	while (backstrace= m_curPos, ps = ParserStat())
	{
		ps->SetParent(pb);
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

	AST_ReStat* pre = new AST_ReStat(pl,flag, m_curPos-1);
	pl->SetParent(pre);
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
	/* args ::=  ¡®(¡¯ [explist] ¡®)¡¯ | tableconstructor | String  */
	size_t flag = m_curPos;
	
	AST_TableCons* ptable = NULL;
	
	if (IsEqualTo("("))
	{
		m_curPos++;
		AST_ExpList* pel = ParserExpList();
		if ( !pel || !IsEqualTo(")") )
		{
			m_ErrStack.push(GenErrorInfo("(...)", "'args'"));
			return NULL;
		}
		m_curPos++;
		AST_Args_Brack* pab = new AST_Args_Brack(pel, flag, m_curPos-1);
		return pab;
	}
	else if (IsEqualTo("'")|| IsEqualTo("\""))
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
	/* var ::=  Name | prefixexp ¡®[¡¯ exp ¡®]¡¯ | prefixexp ¡®.¡¯ Name  */
	
	size_t flag = m_curPos;
	if (IsIdentifier()) /* not greedy match*/
	{
		m_curPos++;
		return new AST_Name(m_tokenStream[m_curPos-1].getTokenName(),flag);
	}

	AST_PrefixExp* ppe = ParserPrefixExp();
	if (!ppe)
	{
		m_ErrStack.push(GenErrorInfo("prefixexp1","'var'"));
		return NULL;
	}

	if (IsEqualTo("["))
	{
		m_curPos++;
		AST_Exp* pe = ParserExp();
		if (!pe || IsEqualTo("]"))
		{
			PushErrInfo("#2", "'var'");
			return NULL;
		}
		m_curPos++;
		return new AST_Var_Brack(ppe, pe, flag, m_curPos-1);
	}

	if (IsEqualTo("."))
	{
		m_curPos++;
		if (!IsIdentifier())
		{
			PushErrInfo("#3-name", "'var'");
			return NULL;
		}
		AST_Name* pn = new AST_Name(m_tokenStream[m_curPos].getTokenName(),m_curPos++);
		return new AST_Var_Dot(ppe, pn, flag, m_curPos-1);
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

	while ( pv = ParserVar() )
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
	AST_Exp* pv = ParserExp();
	if (!pv)
	{
		PushErrInfo("exp", "'explist'");
		return NULL;
	}

	AST_ExpList* pvl = new AST_ExpList(pv, flag);

	while (pv = ParserExp())
	{
		pvl->AddVar(pv);
	}

	pvl->SetEndPosit(m_curPos - 1);
	return pvl;
}

AST_Exp* Parser::ParserExp()
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
		return new AST_Number(m_tokenStream[m_curPos].getNumberValue(),m_curPos++);
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
	else if (AST_PrefixExp* pre = ParserPrefixExp())
	{
		return pre;
	}
	else if (AST_TableCons* pt = ParserTableCons())
	{
		return pt;
	}
	else if (AST_BiExpr* bip = ParserBiExpr())
	{
		return bip;
	}
	else if (AST_UnExpr* pu = ParserUnExpr())
	{
		return pu;
	}
	else
	{
		PushErrInfo("not match", "'exp'");
		return NULL;
	}
}

AST_PrefixExp* Parser::ParserPrefixExp()
{

}


}