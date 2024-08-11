%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ast.h"
#include "y.tab.h"

extern int yylineno;
extern char* yytext;
void yyerror(const char *s);
int yylex(void);
extern FILE *yyin;
extern int yyparse(void);
extern ASTNode* globalRoot;

static ASTNode* currentFunctionBody = NULL;

ASTNode* g_localRoot = NULL;

ASTNode* parse(const char* filename) {
    yyin = fopen(filename, "r");
    if (!yyin) {
        perror("File opening error");
        return NULL;
    }
    char rootNodeName[256];
    snprintf(rootNodeName, sizeof(rootNodeName), "%s Root", filename);

    ASTNode* localRoot = createASTNode(NodeType_Root, rootNodeName);
    if (!localRoot) {
        fprintf(stderr, "Failed to create root node.\n");
        fclose(yyin);
        return NULL;
    }
    
    g_localRoot = localRoot;
    
    fprintf(stderr, "Log: Created local Root for parsing file: %s\n", filename);
    printf("Parsing file: %s\n", filename);
    fprintf(stderr, "Starting parsing process.\n");
    int parseResult = yyparse();
    fclose(yyin);
    
    if (parseResult != 0) {
        fprintf(stderr, "Parsing failed with error %d\n", parseResult);
        freeASTNode(localRoot);
        return NULL;
    }

    fprintf(stderr, "Finished parsing file: %s\n", filename);
    return g_localRoot;
}
%}

%union {
    char* sval;
    ASTNode* ast;
}

%token <sval> IDENTIFIER NUMBER STRING_LITERAL
%token INT RETURN MAIN IF ELSE WHILE FOR VOID
%token PLUS MINUS TIMES DIVIDE ASSIGN SEMICOLON LPAREN RPAREN COMMA LBRACKET RBRACKET LBRACE RBRACE PLUSPLUS MINUSMINUS
%token LT GT LE GE EQ NE AND OR NOT MOD BITAND BITOR XOR SHL SHR
%token PRINTF 


%nonassoc SEMICOLON
%nonassoc LOWER_THAN_SEMICOLON
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%nonassoc LOWER_THAN_RPAREN

%right RPAREN
%right ASSIGN
%right NOT
%left AND OR
%left BITAND BITOR XOR
%left SHL SHR
%left LBRACKET RBRACKET
%left LPAREN
%left PLUS MINUS
%left TIMES DIVIDE MOD
%left LT GT LE GE
%left EQ NE




%type <ast> program main_function function_definition printf_statement array_access param_list param_list_nonempty function_call statements statement compound_statement expression parameter_list parameter array_declaration for_initialization for_condition for_increment

%%

program:
    function_definition {
        if (!g_localRoot->functions) {
            g_localRoot->functions = createASTNode(NodeType_Functions, "Functions");
            addASTChild(g_localRoot, g_localRoot->functions);
        }
        addASTChild(g_localRoot->functions, $1);
    }
    | program function_definition {
        if (!g_localRoot->functions) {
            g_localRoot->functions = createASTNode(NodeType_Functions, "Functions");
            addASTChild(g_localRoot, g_localRoot->functions);
        }
        addASTChild(g_localRoot->functions, $2);
        $$ = $1;
    }
    | main_function {
        addASTChild(g_localRoot, $1);
    }
    | program main_function {
        addASTChild(g_localRoot, $2);
        $$ = $1;
    }
	
	| array_declaration {
       addASTChild(g_localRoot, $1);
    }
    | program array_declaration {
        addASTChild(g_localRoot, $2);
        $$ = $1;
    }
;

main_function:
    INT MAIN LPAREN RPAREN compound_statement
    {
        $$ = createMainFunctionNode("Main", $5);
        currentFunctionBody = $5; // Set the current function body to the compound statement
		fprintf(stderr, "Log: Created main function node with body\n");
		currentFunctionBody = NULL;
    }
;

compound_statement:
    LBRACE statements RBRACE 
    {
        $$ = createASTNode(NodeType_Body, "Body");
        addASTChild($$, $2); // Attach statements to the body
        fprintf(stderr, "Log: Created compound statement with body\n");
    }
;




printf_statement:
    PRINTF LPAREN STRING_LITERAL COMMA IDENTIFIER RPAREN SEMICOLON {
        ASTNode* printfNode = createFunctionCallNode("printf", NULL);
        addASTChild(printfNode, createASTNode(NodeType_Constant, $3));
        addASTChild(printfNode, createASTNode(NodeType_Identifier, $5));
        $$ = printfNode;
    }
    | PRINTF LPAREN STRING_LITERAL RPAREN SEMICOLON {
        ASTNode* printfNode = createFunctionCallNode("printf", NULL);
        addASTChild(printfNode, createASTNode(NodeType_Constant, $3));
        $$ = printfNode;
    }
    ;




statements:
    statement { $$ = $1; }
    | statements statement { addASTChild($1, $2); $$ = $1; }
    ;

statement:
    INT IDENTIFIER ASSIGN expression SEMICOLON {
        ASTNode* assign = createASTNode(NodeType_Assignment, "=");
        addASTChild(assign, createASTNode(NodeType_Identifier, $2));
        addASTChild(assign, $4);
        $$ = assign;
    }
    | INT IDENTIFIER ASSIGN function_call SEMICOLON {
        ASTNode* assign = createASTNode(NodeType_Assignment, "=");
        addASTChild(assign, createASTNode(NodeType_Identifier, $2));
        addASTChild(assign, $4);
        $$ = assign;
    }
    | IDENTIFIER ASSIGN expression SEMICOLON {
        ASTNode* assign = createASTNode(NodeType_Assignment, "=");
        addASTChild(assign, createASTNode(NodeType_Identifier, $1));
        addASTChild(assign, $3);
        $$ = assign;
    }
    | IDENTIFIER LBRACKET expression RBRACKET ASSIGN expression SEMICOLON {
    ASTNode* indices[1] = {$3};
    ASTNode* arrayUsage = createArrayAccessNode($1, "int", indices, 1);
    if (!arrayUsage) {
        yyerror("Failed to create array access node");
        YYABORT;
    }

    ASTNode* assign = createASTNode(NodeType_Assignment, "=");
    if (!assign) {
        yyerror("Failed to create assignment node");
        YYABORT;
    }
    addASTChild(assign, arrayUsage);
    addASTChild(assign, $6);

    if (currentFunctionBody) {
        addASTChild(currentFunctionBody, assign);
    }
    $$ = assign;
}
	| IDENTIFIER LBRACKET expression RBRACKET LBRACKET expression RBRACKET ASSIGN expression SEMICOLON {
    ASTNode* indices[2] = {$3, $6};
    ASTNode* arrayUsage = createArrayAccessNode($1, "int", indices, 2);
    if (!arrayUsage) {
        yyerror("Failed to create array access node");
        YYABORT;
    }

    ASTNode* assign = createASTNode(NodeType_Assignment, "=");
    if (!assign) {
        yyerror("Failed to create assignment node");
        YYABORT;
    }
    addASTChild(assign, arrayUsage);
    addASTChild(assign, $9);

    if (currentFunctionBody) {
        addASTChild(currentFunctionBody, assign);
    }
    $$ = assign;
}
    | IDENTIFIER ASSIGN function_call SEMICOLON %prec LOWER_THAN_SEMICOLON {
        ASTNode* assign = createASTNode(NodeType_Assignment, "=");
        addASTChild(assign, createASTNode(NodeType_Identifier, $1));
        addASTChild(assign, $3);
        $$ = assign;
    }
    | RETURN expression SEMICOLON {
        ASTNode* ret = createASTNode(NodeType_Return, "return");
        addASTChild(ret, $2);
        $$ = ret;
    }
    | expression SEMICOLON { $$ = $1; }
    | IF LPAREN expression RPAREN statement %prec LOWER_THAN_ELSE {
        ASTNode* ifCondNode = createASTNode(NodeType_Condition, "ifCond");
        addASTChild(ifCondNode, $3);
        ASTNode* ifBodyNode = createASTNode(NodeType_IfBody, "ifBody");
        addASTChild(ifBodyNode, $5);
        ASTNode* ifNode = createASTNode(NodeType_If, "ifNode");
        addASTChild(ifNode, ifCondNode);
        addASTChild(ifNode, ifBodyNode);
        $$ = ifNode;
    }
    | IF LPAREN expression RPAREN statement ELSE statement %prec ELSE {
        ASTNode* ifCondNode = createASTNode(NodeType_Condition, "ifCond");
        addASTChild(ifCondNode, $3);
        ASTNode* ifBodyNode = createASTNode(NodeType_IfBody, "ifBody");
        addASTChild(ifBodyNode, $5);
        ASTNode* elseBodyNode = createASTNode(NodeType_ElseBody, "elseBody");
        addASTChild(elseBodyNode, $7);
        ASTNode* ifElseNode = createASTNode(NodeType_IfElse, "ifElseNode");
        addASTChild(ifElseNode, ifCondNode);
        addASTChild(ifElseNode, ifBodyNode);
        addASTChild(ifElseNode, elseBodyNode);
        $$ = ifElseNode;
    }
    | WHILE LPAREN expression RPAREN statement {
        ASTNode* whileCondNode = createASTNode(NodeType_Condition, "whileCond");
        addASTChild(whileCondNode, $3);
        ASTNode* whileBodyNode = createASTNode(NodeType_WhileBody, "whileBody");
        addASTChild(whileBodyNode, $5);
        ASTNode* whileNode = createASTNode(NodeType_While, "whileNode");
        addASTChild(whileNode, whileCondNode);
        addASTChild(whileNode, whileBodyNode);
        $$ = whileNode;
    }
    | FOR LPAREN for_initialization SEMICOLON for_condition SEMICOLON for_increment RPAREN compound_statement {
        ASTNode* forNode = createASTNode(NodeType_For, "for");
        if (forNode) {
            addASTChild(forNode, $3); // initialization
            addASTChild(forNode, $5); // condition
            addASTChild(forNode, $7); // increment
            addASTChild(forNode, $9); // body
            $$ = forNode;
        }
    }
    | function_definition { $$ = $1; }
    | function_call SEMICOLON { $$ = $1; } %prec LOWER_THAN_SEMICOLON
    | array_declaration { $$ = $1; }
    | array_access { $$ = $1; }
    | printf_statement { $$ = $1; }  // Treat printf_statement as a part of statement
    | INT IDENTIFIER LBRACKET NUMBER RBRACKET { // for 1D arrays
        ASTNode* array_decl = createArrayNode("int", $2, $4);
        if (currentFunctionBody) {
            addASTChild(currentFunctionBody, array_decl);
            fprintf(stderr, "Added 1D array to current function body.\n");
        } else {
            addASTChild(g_localRoot, array_decl); // Adding directly under the global root or a designated global scope node.
            fprintf(stderr, "Added 1D global array declaration.\n");
        }
        $$ = array_decl;
    }
    | INT IDENTIFIER LBRACKET NUMBER RBRACKET LBRACKET NUMBER RBRACKET { // for 2D arrays
        ASTNode* array_decl = create2DArrayNode("int", $2, $4, $7);
        if (currentFunctionBody) {
            addASTChild(currentFunctionBody, array_decl);
            fprintf(stderr, "Added 2D array to current function body.\n");
        } else {
            addASTChild(g_localRoot, array_decl); // Adding directly under the global root or a designated global scope node.
            fprintf(stderr, "Added 2D global array declaration.\n");
        }
        $$ = array_decl;
    }
;



function_definition:
    INT IDENTIFIER LPAREN param_list RPAREN compound_statement
    {
        ASTNode* functionNode = createFunctionNode($2, $4, $6);
        currentFunctionBody = $6; // Set the body of the function
        addASTChild(globalRoot, functionNode); // Add this function to the global AST root
        fprintf(stderr, "Log: Created function node %s with body\n", $2);
        $$ = functionNode;
        currentFunctionBody = NULL; // Reset after function is handled
    }
;


function_call:
    IDENTIFIER LPAREN parameter_list RPAREN {
        $$ = createFunctionCallNode($1, $3);
        fprintf(stderr, "Log: Created function call node: %s with parameters.\n", $1);
    }
    | PRINTF LPAREN STRING_LITERAL COMMA expression RPAREN {
        ASTNode* printfNode = createFunctionCallNode("printf", NULL);
        addASTChild(printfNode, createASTNode(NodeType_Constant, $3));
        addASTChild(printfNode, $5);
        $$ = printfNode;
    }
;

for_initialization:
    INT IDENTIFIER ASSIGN expression {
        ASTNode* initNode = createASTNode(NodeType_Assignment, "=");
        addASTChild(initNode, createASTNode(NodeType_Identifier, $2));
        addASTChild(initNode, $4);
        $$ = initNode;
    }
    | IDENTIFIER ASSIGN expression {
        ASTNode* initNode = createASTNode(NodeType_Assignment, "=");
        addASTChild(initNode, createASTNode(NodeType_Identifier, $1));
        addASTChild(initNode, $3);
        $$ = initNode;
    }
;

for_condition:
    expression { $$ = $1; }
;

for_increment:
    IDENTIFIER PLUSPLUS {
        ASTNode* incrNode = createASTNode(NodeType_Expression, "++");
        addASTChild(incrNode, createASTNode(NodeType_Identifier, $1));
        $$ = incrNode;
    }
    | IDENTIFIER MINUSMINUS {
        ASTNode* decrNode = createASTNode(NodeType_Expression, "--");
        addASTChild(decrNode, createASTNode(NodeType_Identifier, $1));
        $$ = decrNode;
    }
    | IDENTIFIER ASSIGN expression {
        ASTNode* assignNode = createASTNode(NodeType_Assignment, "=");
        addASTChild(assignNode, createASTNode(NodeType_Identifier, $1));
        addASTChild(assignNode, $3);
        $$ = assignNode;
    }
;






parameter:
    INT IDENTIFIER {
        ASTNode* paramNode = createParameterNode("int");
        addASTChild(paramNode, createASTNode(NodeType_Identifier, $2));
        $$ = paramNode;
    }
    | INT IDENTIFIER LBRACKET RBRACKET {
        ASTNode* paramNode = createParameterNode("int[]");
        addASTChild(paramNode, createASTNode(NodeType_Identifier, $2));
        $$ = paramNode;
    }
;

param_list:
    /* Handle the empty parameter list case. */
    { $$ = createASTNode(NodeType_ParameterList, "Empty Param List"); }
    | param_list_nonempty { $$ = $1; }
;

param_list_nonempty:
    parameter
    { 
        ASTNode* pList = createASTNode(NodeType_ParameterList, "Param List");
        addASTChild(pList, $1);
        $$ = pList;
    }
    | param_list_nonempty COMMA parameter
    {
        addASTChild($1, $3);
        $$ = $1;
    }
;


parameter_list:
    /* Boş parametre listesi için uygun bir AST düğümü oluşturuluyor. */
    {
        $$ = createASTNode(NodeType_ParameterList, "Empty Param List");
        fprintf(stderr, "Created an empty parameter list node.\n");
    }
    | parameter {
        ASTNode* pList = createASTNode(NodeType_ParameterList, "Param List");
        addASTChild(pList, $1);
        $$ = pList;
    }
    | parameter_list COMMA parameter {
        addASTChild($1, $3);
        $$ = $1;
    }
;

expression:
    IDENTIFIER { $$ = createASTNode(NodeType_Identifier, $1); } %prec LOWER_THAN_RPAREN
    | NUMBER { $$ = createASTNode(NodeType_Constant, $1); }
    | STRING_LITERAL { $$ = createASTNode(NodeType_Constant, $1); }
	| LPAREN expression RPAREN { $$ = $2; }
    | expression PLUS expression {
        ASTNode* plusExpr = createASTNode(NodeType_Expression, "+");
        addASTChild(plusExpr, $1);
        addASTChild(plusExpr, $3);
        $$ = plusExpr;
    }
    | expression MINUS expression {
        ASTNode* minusExpr = createASTNode(NodeType_Expression, "-");
        addASTChild(minusExpr, $1);
        addASTChild(minusExpr, $3);
        $$ = minusExpr;
    }
    | expression TIMES expression {
        ASTNode* timesExpr = createASTNode(NodeType_Expression, "*");
        addASTChild(timesExpr, $1);
        addASTChild(timesExpr, $3);
        $$ = timesExpr;
    }
	
    | expression DIVIDE expression {
        ASTNode* divideExpr = createASTNode(NodeType_Expression, "/");
        addASTChild(divideExpr, $1);
        addASTChild(divideExpr, $3);
        $$ = divideExpr;
    }
    | expression MOD expression {
        ASTNode* modExpr = createASTNode(NodeType_Expression, "%");
        addASTChild(modExpr, $1);
        addASTChild(modExpr, $3);
        $$ = modExpr;
    }
    | IDENTIFIER LBRACKET expression RBRACKET {
    ASTNode* indices[1] = {$3};  // Create an array with a single index
    // Assuming the data type is known, e.g., "int"
    $$ = createArrayAccessNode($1, "int", indices, 1);  // Pass the array and the count of indices
}
	| IDENTIFIER LBRACKET expression RBRACKET LBRACKET expression RBRACKET {
    ASTNode* indices[2] = {$3, $6};  // Create an array with two indices
    // Assuming the data type is known, e.g., "int"
    $$ = createArrayAccessNode($1, "int", indices, 2);  // Pass the array and the count of indices
}



    | expression LT expression {
        ASTNode* lessThanExpr = createASTNode(NodeType_Expression, "<");
        addASTChild(lessThanExpr, $1);
        addASTChild(lessThanExpr, $3);
        $$ = lessThanExpr;
    }
    | expression GT expression {
        ASTNode* greaterThanExpr = createASTNode(NodeType_Expression, ">");
        addASTChild(greaterThanExpr, $1);
        addASTChild(greaterThanExpr, $3);
        $$ = greaterThanExpr;
    }
    | expression LE expression {
        ASTNode* lessEqExpr = createASTNode(NodeType_Expression, "<=");
        addASTChild(lessEqExpr, $1);
        addASTChild(lessEqExpr, $3);
        $$ = lessEqExpr;
    }
    | expression GE expression {
        ASTNode* greaterEqExpr = createASTNode(NodeType_Expression, ">=");
        addASTChild(greaterEqExpr, $1);
        addASTChild(greaterEqExpr, $3);
        $$ = greaterEqExpr;
    }
    | expression EQ expression {
        ASTNode* equalExpr = createASTNode(NodeType_Expression, "==");
        addASTChild(equalExpr, $1);
        addASTChild(equalExpr, $3);
        $$ = equalExpr;
    }
    | expression NE expression {
        ASTNode* notEqualExpr = createASTNode(NodeType_Expression, "!=");
        addASTChild(notEqualExpr, $1);
        addASTChild(notEqualExpr, $3);
        $$ = notEqualExpr;
    }
;



array_declaration:
    INT IDENTIFIER LBRACKET NUMBER RBRACKET SEMICOLON
    { 
        fprintf(stderr, "Parsing 1D array declaration: %s[%s]\n", $2, $4);
        ASTNode* array_decl = createArrayNode("int", $2, $4);
        if (currentFunctionBody) {
            addASTChild(currentFunctionBody, array_decl);
            fprintf(stderr, "Added 1D array to function body.\n");
        } else {
            fprintf(stderr, "Error: No current function body found for 1D array.\n");
            addASTChild(g_localRoot, array_decl); // Add to global root if not within a function.
        }
        $$ = array_decl;
    }
  | INT IDENTIFIER LBRACKET NUMBER RBRACKET LBRACKET NUMBER RBRACKET SEMICOLON
    {
        fprintf(stderr, "Parsing 2D array declaration: %s[%s][%s]\n", $2, $4, $7);
        ASTNode* array_decl = create2DArrayNode("int", $2, $4, $7);
        if (currentFunctionBody) {
            addASTChild(currentFunctionBody, array_decl);
            fprintf(stderr, "Added 2D array to function body.\n");
        } else {
            fprintf(stderr, "Error: No current function body found for 2D array.\n");
            addASTChild(g_localRoot, array_decl); // Add to global root if not within a function.
        }
        $$ = array_decl;
    }
;


array_access:
    IDENTIFIER LBRACKET expression RBRACKET {
        // Single index array access
        ASTNode* indices[1] = {$3};
        ASTNode* arrayAccessNode = createArrayAccessNode($1, "int", indices, 1); // Assuming the data type is known, e.g., "int"
        if (currentFunctionBody) {
            addASTChild(currentFunctionBody, arrayAccessNode); // Add to the current function body if it exists
        } else {
            fprintf(stderr, "Error: No current function body found for single index array access.\n");
        }
        $$ = arrayAccessNode;
    }
    | IDENTIFIER LBRACKET expression RBRACKET LBRACKET expression RBRACKET {
        // Two-dimensional array access
        ASTNode* indices[2] = {$3, $6}; // Capture both indices
        ASTNode* arrayAccessNode = createArrayAccessNode($1, "int", indices, 2); // Assuming the data type is known, e.g., "int"
        if (currentFunctionBody) {
            addASTChild(currentFunctionBody, arrayAccessNode); // Add to the current function body if it exists
        } else {
            fprintf(stderr, "Error: No current function body found for two-dimensional array access.\n");
        }
        $$ = arrayAccessNode;
    }
;





%%

void yyerror(const char *s) {
    fprintf(stderr, "%s at line %d before '%s'\n", s, yylineno, yytext);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <file1.c> <file2.c>\n", argv[0]);
        return EXIT_FAILURE;
    }

    ASTNode* root1 = parse(argv[1]);
    if (!root1) {
        fprintf(stderr, "Error: Parsing failed for %s.\n", argv[1]);
        return EXIT_FAILURE;
    }

    ASTNode* root2 = parse(argv[2]);
    if (!root2) {
        fprintf(stderr, "Error: Parsing failed for %s.\n", argv[2]);
        freeASTNode(root1);
        return EXIT_FAILURE;
    }

    printf("AST for %s:\n", argv[1]);
    printAST(root1);
    printf("AST for %s:\n", argv[2]);
    printAST(root2);

    int similarityScore = compareASTs(root1, root2);
    printf("Total similarity score between %s and %s is: %d%%\n", argv[1], argv[2], similarityScore);

    freeASTNode(root1);
    freeASTNode(root2);

    return EXIT_SUCCESS;
}
