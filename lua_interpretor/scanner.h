#ifndef SCANNER_H_
#define SCANNER_H_

#include <fstream>
#include <string>
#include <vector>

#include "error.h"
#include "token.h"
#include "dictinary.h"



namespace lua_in
{

class Scanner
{
public:
	Scanner(void);
	Scanner(std::string in_fileName);
	~Scanner(void);

	void RunScanner();
	Token GetNextToken();

private:
	Token ProcessNumberState(char ch);   //123, 12.8 etc.
	Token ProcessStringState(char ch);   // const string б░ бо
	Token ProcessIndentifyState(char ch);//include reserved
	Token ProcessOperatorAndDelim(char ch);//such as + .

	void HandleComemnt(char ch);

	char GetNextChar();
	void PutBackChar(char ch);
	char PeekNextChar();
	
	void StampLocation();
	void RecoverFromError();//skip to next space char

private:

	enum State_Flag
	{
		S_Begin,
		S_Error, //error token
		S_Accept,
		S_EOF,
		S_Fatal //  irreversible error, stop lex analysis
	};

	long		  m_line;
	long		  m_column;
	TokenLocation m_curLoc;  //record current recognize begin location
	
	std::string   m_infileName;
	std::ifstream m_infileStream;

	std::string   m_buffer;  //save the temp char
	State_Flag    m_currState;
	std::string   m_strErrReason;
	bool          m_isErrRecover; //

	std::vector<Token> m_vecToken;
	Dictinary     m_dict;
};

}

#endif

