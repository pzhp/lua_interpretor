
#ifndef TOKEN_H_
#define TOKEN_H_

#include <string>
#include <iostream>
#include <tuple>

namespace lua_in
{
	enum TokenType
	{
		T_ConstNumber,
		T_ConstString,
		T_Boolean,

		T_Operators,
		T_Identify,
		T_Keywords,
		T_Delimiter,  // , :
		T_EOF,
		T_Unknown
	};

	enum TokenValue
	{
		//keywords
		V_And, V_Break, V_Do, V_Else, V_Elseif, V_End,
		V_False, V_For, V_Function, V_Goto, V_If, V_In,
		V_Then, V_True, V_Until, V_While, V_NIL,

		//operators
		V_Plus, V_Minus, V_Mutiply, V_Divide, V_Mod, V_Uparrow, V_Sharp, //+, -, *, /, %,  ^, #
		V_Equal, V_NotEqual, V_LessEqual, V_GreaterEqual, V_Less, V_Greater, V_Assign, //==, ~=, <=, >=, <, >, =

		//dilimiter
		V_LeftParen, V_RightParen, V_LeftSquare, V_RightSquare, V_LeftBrace, V_RightBrace, //(, ), [, ], {, }

		V_ColonTwo, V_SemiColon, V_Colon, V_Comma, V_DotOne, V_DotTwo, V_DotThree, //:: ; : , . .. ... 

		//IO routine
		V_Print, V_PrintLn, V_Read, V_ReadLn,

		// number, string
		V_ConstNumber,
		V_ConstString,

		V_UnReserved
	};

	class TokenLocation
	{
	public:
		TokenLocation();
		TokenLocation(const std::string filename, int line, int column);
		std::string toString() const;

	private:
		std::string m_filename;
		int m_line;
		int m_column;
	};

	class Token
	{
	public:
		Token(void);
		~Token(void);
		Token(TokenType type, TokenValue val, const TokenLocation& location, std::string name, int symbolPre=-1);
		Token(TokenType type, TokenValue val, const TokenLocation& location, std::string name, double numValue);
		Token(TokenType type, TokenValue val, const TokenLocation& location, std::string name, std::string strValue);
		Token(std::tuple<TokenValue, TokenType, int> tuple_val, TokenLocation& location, std::string name);

		inline TokenType			getTokenType() const {return m_type;}
		inline TokenValue			getTokenValue() const {return m_value;}
		inline const TokenLocation& getTokenLocation() const {return m_location;}
		inline std::string			getTokenName() const {return m_name;}
		inline int					getSymbolPre() const {return m_symPriority;}
		inline double				getNumberValue() const {return m_number_value;}
		inline std::string			getStringValue() const {return m_str_value;}

		inline void SetTokenErrorReason(const std::string& str){m_errorReason = str;}
		bool IsErrorToken()const;
		std::string toString() const;
	
	private:
		TokenType		m_type;
		TokenValue		m_value;
		TokenLocation   m_location;
		std::string		m_name; //description in source file
		int				m_symPriority;

		double			m_number_value;
		std::string		m_str_value;
		std::string		m_errorReason; //if it is error token, save the reason 
	};
}
#endif