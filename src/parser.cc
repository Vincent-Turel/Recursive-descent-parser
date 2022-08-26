#include <utility>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <regex>

typedef enum {OPN_CURLY_BRACE, CLS_CURLY_BRACE, SEMICOLON, OPN_BRACE,
    CLS_BRACE, OPN_PAREN, CLS_PAREN, ASSIGN, EQ, DIFF, IF, ELSE, WHILE, BREAK,
    OR, AND, EXCL_MARK, INF, SUPP, INF_EQ, SUPP_EQ, PLUS, MINUS, TIME, DIV,
    ID, NUM, BASIC, REAL, TRUE, FALSE, EOS, PROGRAM, BLOCK, DECLARATIONS,
    DECLARATION, TYPE, TYPE_CLS, STATEMENTS, STATEMENT, LOC, LOC_CLS, BOOL, BOOL_CLS,
    JOIN, JOIN_CLS, EQUALITY, EQUALITY_CLS,RELATION, EXPRESSION, EXPRESSION_CLS, TERM,
    TERM_CLS, UNARY, FACTOR} Token;

static const char *Token_Str[] = {
        "OPN_CURLY_BRACE", "CLS_CURLY_BRACE", "SEMICOLON", "OPN_BRACE",
        "CLS_BRACE", "OPN_PAREN", "CLS_PAREN", "ASSIGN", "EQ", "DIFF", "IF", "ELSE", "WHILE", "BREAK",
        "OR", "AND", "EXCL_MARK", "INF", "SUPP", "INF_EQ", "SUPP_EQ", "PLUS", "MINUS", "TIME", "DIV",
        "ID", "NUM", "BASIC", "REAL", "TRUE", "FALSE", "EOS", "Program", "Block", "Declarations",
        "Declaration", "Type", "Type_cls", "Statements", "Statement", "Loc", "Loc_cls", "Bool", "Bool_cls",
        "Join", "Join_cls", "Equality", "Equality_cls","Relation", "Expression", "Expression_cls", "Term",
        "Term_cls", "Unary", "Factor"
};

class Node  {
private:
    Token token;
    std::string lexeme;
    std::vector<Node*> childs;

public:
    explicit Node(Token t, std::string& text) {
        lexeme = text;
        token = t;
    }

    void AddChild(Node *p)  {
        if (p != nullptr) childs.push_back(p);
    }

    void Print(int indentLevel) {
        const int numSpaces = 3;
        printf("%*s", numSpaces, "");
        printf("%*s%s", indentLevel*numSpaces, "", Token_Str[token]);
        if (!lexeme.empty())
            printf(" : '%s' ", lexeme.c_str());
        printf("\n");
        for (auto child : childs) {
            child->Print(indentLevel + 1);
        }
    }
};

struct Unit {
    Token token;
    std::string lexeme;
    int line;

    Unit(Token t, std::string s, int l) {
        token = t;
        lexeme = std::move(s);
        line = l;
    }
};

void ScanInput();
Token ScanToken(const std::string&);
Unit* GetToken();
void MustBe(Token);
Node* Program();
Node* Block();
Node* Decls();
Node* Decl();
Node* Type();
Node* TypeClosure();
Node* Stmts();
Node* Stmt();
Node* Loc();
Node* LocClosure();
Node* Bool();
Node* BoolClosure();
Node* Join();
Node* JoinClosure();
Node* Equality();
Node* EqualityClosure();
Node* Rel();
Node* Expr();
Node* ExprClosure();
Node* Term();
Node* TermClosure();
Node* Unary();
Node* Factor();


std::vector<Unit*> units;
Unit *t = nullptr;
int count;

void ScanInput() {
    std::string line;
    int line_count = 1;
    while (std::getline(std::cin, line))
    {
        std::istringstream iss(line);
        for (std::string s; iss >> s; )
            units.push_back(new Unit(ScanToken(s), s, line_count));
        line_count++;
    }
    units.push_back(new Unit(EOS, *new std::string(), line_count));
    count = 0;
}

Token ScanToken(const std::string& string) {
    if (string == "int" || string == "bool" || string == "double" || string == "char") return BASIC;
    if (string == "if") return IF;
    if (string == "else") return ELSE;
    if (string == "while") return WHILE;
    if (string == "break") return BREAK;
    if (string == "=") return ASSIGN;
    if (string == "==") return EQ;
    if (string == ">") return SUPP;
    if (string == ">=") return SUPP_EQ;
    if (string == "<") return INF;
    if (string == "<=") return INF_EQ;
    if (string == "!=") return DIFF;
    if (string == "&&") return AND;
    if (string == "||") return OR;
    if (string == "+") return PLUS;
    if (string == "-") return MINUS;
    if (string == "*") return TIME;
    if (string == "/") return DIV;
    if (string == "!") return EXCL_MARK;
    if (string == "true") return TRUE;
    if (string == "false") return FALSE;
    if (string == "{") return OPN_CURLY_BRACE;
    if (string == "}") return CLS_CURLY_BRACE;
    if (string == "[") return OPN_BRACE;
    if (string == "]") return CLS_BRACE;
    if (string == "(") return OPN_PAREN;
    if (string == ")") return CLS_PAREN;
    if (string == ";") return SEMICOLON;
    if (std::regex_match (string, std::regex("[0-9]+\\.[0-9]*"))) return REAL;
    if (std::regex_match(string, std::regex("[0-9]+"))) return NUM;
    return ID;
}

Unit* GetToken() {
    return units[count++];
}

void MustBe(Token token) {
    if (t->token != token) {
        printf("Token rejected line %i. \n", t->line);
        printf("Got : %s with lexeme '%s'\n", Token_Str[t->token], t->lexeme.c_str());
        printf("Was expecting : %s\n", Token_Str[token]);
        exit(1); }
    t = GetToken();
}

Node* Program() {
    Node *current = new Node(PROGRAM, *new std::string());
    current->AddChild(Block());
    return current;
}

Node* Block() {
    Node *current = new Node(BLOCK, *new std::string());
    current->AddChild(new Node(t->token, t->lexeme));
    MustBe(OPN_CURLY_BRACE);
    current->AddChild(Decls());
    current->AddChild(Stmts());
    current->AddChild(new Node(t->token, t->lexeme));
    MustBe(CLS_CURLY_BRACE);
    return current;
}

Node* Decls() {
    Node *current = new Node(DECLARATIONS, *new std::string());
    if (t->token == BASIC) {
        current->AddChild(Decl());
        current->AddChild(Decls());
    }
    return current;
}

Node* Decl() {
    Node *current = new Node(DECLARATION, *new std::string());
    current->AddChild(Type());
    current->AddChild(new Node(t->token, t->lexeme));
    MustBe(ID);
    current->AddChild(new Node(t->token, t->lexeme));
    MustBe(SEMICOLON);
    return current;
}

Node* Type() {
    Node *current = new Node(TYPE, *new std::string());
    current->AddChild(new Node(t->token, t->lexeme));
    MustBe(BASIC);
    current->AddChild(TypeClosure());
    return current;
}

Node* TypeClosure() {
    Node *current = new Node(TYPE_CLS, *new std::string());
    if (t->token == OPN_BRACE) {
        current->AddChild(new Node(t->token, t->lexeme));
        MustBe(OPN_BRACE);
        current->AddChild(new Node(t->token, t->lexeme));
        MustBe(NUM);
        current->AddChild(new Node(t->token, t->lexeme));
        MustBe(CLS_BRACE);
        current->AddChild(TypeClosure());
        return current;
    }
    return nullptr;
}

Node* Stmts() {
    Node *current = new Node(STATEMENTS, *new std::string());
    if (t->token == OPN_CURLY_BRACE || t->token == IF || t->token == WHILE || t->token == ID) {
        current->AddChild(Stmt());
        current->AddChild(Stmts());
    }
    return current;
}

Node* Stmt() {
    Node *current = new Node(STATEMENT, *new std::string());
    switch (t->token) {
        case IF:
            current->AddChild(new Node(t->token, t->lexeme));
            t = GetToken();
            current->AddChild(new Node(t->token, t->lexeme));
            MustBe(OPN_PAREN);
            current->AddChild(Bool());
            current->AddChild(new Node(t->token, t->lexeme));
            MustBe(CLS_PAREN);
            current->AddChild(Stmt());
            if (t->token == ELSE) {
                current->AddChild(new Node(t->token, t->lexeme));
                t = GetToken();
                current->AddChild(Stmt());
            }
            break;
        case WHILE:
            current->AddChild(new Node(t->token, t->lexeme));
            t = GetToken();
            current->AddChild(new Node(t->token, t->lexeme));
            MustBe(OPN_PAREN);
            current->AddChild(Bool());
            current->AddChild(new Node(t->token, t->lexeme));
            MustBe(CLS_PAREN);
            current->AddChild(Stmt());
            break;
        case OPN_CURLY_BRACE:
            current->AddChild(Block());
            break;
        default:
            current->AddChild(Loc());
            current->AddChild(new Node(t->token, t->lexeme));
            MustBe(ASSIGN);
            current->AddChild(Bool());
            current->AddChild(new Node(t->token, t->lexeme));
            MustBe(SEMICOLON);
            break;
    }
    return current;
}

Node* Loc() {
    Node *current = new Node(LOC, *new std::string());
    current->AddChild(new Node(t->token, t->lexeme));
    MustBe(ID);
    current->AddChild(LocClosure());
    return current;
}

Node* LocClosure() {
    Node *current = new Node(LOC_CLS, *new std::string());
    if (t->token == OPN_BRACE) {
        current->AddChild(new Node(t->token, t->lexeme));
        t = GetToken();
        current->AddChild(Bool());
        current->AddChild(new Node(t->token, t->lexeme));
        MustBe(CLS_BRACE);
        current->AddChild(LocClosure());
        return current;
    }
    return nullptr;
}

Node* Bool() {
    Node *current = new Node(BOOL, *new std::string());
    current->AddChild(Join());
    current->AddChild(BoolClosure());
    return current;
}

Node* BoolClosure() {
    Node *current = new Node(BOOL_CLS, *new std::string());
    if (t->token == OR) {
        current->AddChild(new Node(t->token, t->lexeme));
        t = GetToken();
        current->AddChild(Join());
        current->AddChild(BoolClosure());
        return current;
    }
    return nullptr;
}

Node* Join() {
    Node *current = new Node(JOIN, *new std::string());
    current->AddChild(Equality());
    current->AddChild(EqualityClosure());
    return current;
}

Node* JoinClosure() {
    Node *current = new Node(JOIN_CLS, *new std::string());
    if (t->token == AND) {
        current->AddChild(new Node(t->token, t->lexeme));
        t = GetToken();
        current->AddChild(Equality());
        current->AddChild(JoinClosure());
        return current;
    }
    return nullptr;
}

Node* Equality() {
    Node *current = new Node(EQUALITY, *new std::string());
    current->AddChild(Rel());
    current->AddChild(EqualityClosure());
    return current;
}

Node* EqualityClosure() {
    Node *current = new Node(EQUALITY_CLS, *new std::string());
    if (t->token == EQ || t->token == DIFF) {
        current->AddChild(new Node(t->token, t->lexeme));
        t = GetToken();
        current->AddChild(Rel());
        current->AddChild(EqualityClosure());
        return current;
    }
    return nullptr;
}

Node* Rel() {
    Node *current = new Node(RELATION, *new std::string());
    current->AddChild(Expr());
    if (t->token == INF || t->token == INF_EQ || t->token == SUPP || t->token == SUPP_EQ) {
        current->AddChild(new Node(t->token, t->lexeme));
        t = GetToken();
        current->AddChild(Expr());
    }
    return current;
}

Node* Expr() {
    Node *current = new Node(EXPRESSION, *new std::string());
    current->AddChild(Term());
    current->AddChild(ExprClosure());
    return current;
}

Node* ExprClosure() {
    Node *current = new Node(EXPRESSION_CLS, *new std::string());
    if (t->token == PLUS || t->token == MINUS) {
        current->AddChild(new Node(t->token, t->lexeme));
        t = GetToken();
        current->AddChild(Term());
        current->AddChild(ExprClosure());
        return current;
    }
    return nullptr;
}

Node* Term() {
    Node *current = new Node(TERM, *new std::string());
    current->AddChild(Unary());
    current->AddChild(TermClosure());
    return current;
}

Node* TermClosure() {
    Node *current = new Node(TERM_CLS, *new std::string());
    if (t->token == TIME || t->token == DIV) {
        current->AddChild(new Node(t->token, t->lexeme));
        t = GetToken();
        current->AddChild(Unary());
        current->AddChild(TermClosure());
        return current;
    }
    return nullptr;
}

Node* Unary() {
    Node *current = new Node(UNARY, *new std::string());
    if (t->token == EXCL_MARK || t->token == MINUS) {
        current->AddChild(new Node(t->token, t->lexeme));
        t = GetToken();
        current->AddChild(Unary());
    } else {
        current->AddChild(Factor());
    }
    return current;
}

Node* Factor() {
    Node *current = new Node(FACTOR, *new std::string());
    if (t->token == OPN_PAREN) {
        current->AddChild(new Node(t->token, t->lexeme));
        t = GetToken();
        current->AddChild(Bool());
        current->AddChild(new Node(t->token, t->lexeme));
        MustBe(CLS_PAREN);
    } else if (t->token == NUM || t->token == REAL || t->token == FALSE || t->token == TRUE) {
        current->AddChild(new Node(t->token, t->lexeme));
        t = GetToken();
    } else {
        current->AddChild(Loc());
    }
    return current;
}

int main()
{
    std::cout << "Beginning parsing..." << std::endl << std::endl;
    ScanInput();
    t = GetToken();
    Node* ParseTree = Program();
    if (t->token != EOS) {
        std::cout << "String not accepted" << std::endl;
        return 1;
    }
    std::cout << "String accepted" << std::endl << std::endl;
    ParseTree->Print(0);
    return 0;
}

/************ New grammar modified for recursive descent parsing **************

program ::= block

block ::= { decls stmts }

decls ::= decl decls | ε

decl ::= type ID ;

type ::= BASIC typeClosure

typeClosure ::= [ NUM ] typeClosure | ε

stmts ::= stmt stmts | ε

stmt ::= loc = bool ;
       | IF ( bool ) stmt
       | IF ( bool ) stmt ELSE stmt
       | WHILE ( bool ) stmt
       | block

loc ::= ID locClosure

locClosure ::= [ bool ] locClosure | ε

bool ::= join boolClosure

boolClosure ::= || join boolClosure | ε

join ::= equality joinClosure

joinClosure ::= && equality joinClosure | ε

equality ::= rel equalityClosure

equalityClosure ::= == rel equalityClosure | != rel equalityClosure | ε

rel ::= expr < expr
      | expr <= expr
      | expr >= expr
      | expr > expr
      | expr

expr ::= term exprClosure

exprClosure ::= + term exprClosure | - term exprClosure | ε

term ::= unary termClosure

termClosure ::= * unary termClosure | / unary termClosure | ε

unary ::= ! unary
        | - unary
        | factor

 factor ::= ( bool )
          | loc
          | NUM
          | REAL
          | TRUE
          | FALSE

************ TESTING **************

{
int integer ;
bool boolean ;

integer = 5 ;
boolean = true ;

if ( boolean ) integer = integer * 2 ;
boolean = ( integer == 5 ) ;
}

This small program gives me a valid parsing and print the corresponding tree.

{
int integer ;
integer = break ;
}

This one does not parse because we cannot assign the keyword 'break' to a variable.

{
bool boolean ;
boolean = true ;

int integer ;
integer = 3 ;
}

This one does not parse either because we cannot declare a variable once we've done a statement.

bool boolean ;

Finally, this line does not parse because the program is not inside a block.

*****************************/
