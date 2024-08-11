#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <string.h>

#define OPERATION_ADD 1
#define OPERATION_SUBTRACT 2
#define OPERATION_MULTIPLY 3
#define OPERATION_DIVIDE 4


typedef enum {
    Op_Add,
    Op_Subtract,
    Op_Multiply,
    Op_Divide,
    
} OperationType;

typedef enum NodeType {
    NodeType_Program,
    NodeType_Root, // Root node type
    NodeType_FunctionDef, // Function definition
    NodeType_MainFunction, // Special node type for the main function
    NodeType_Functions, // Container for function nodes
    NodeType_Body, // Function body
    NodeType_FunctionCall, // Function call
    NodeType_Expression, // Expressions
    NodeType_Parameter, // Function parameters
    NodeType_VariableDecl, // Variable declaration
    NodeType_Assignment, // Variable assignment
    NodeType_Return, // Return statements
    NodeType_Condition, NodeType_IfBody, NodeType_ElseBody, NodeType_WhileBody, NodeType_ForInit, NodeType_ForIncrement, NodeType_ForBody, NodeType_If, NodeType_IfElse, NodeType_While, NodeType_For,
    NodeType_Statements, // Container for multiple statements
    NodeType_ControlStructure,
	NodeType_ParameterList, // List of parameters
    NodeType_Identifier, // Identifiers
    NodeType_Array,
    NodeType_Loop,
	NodeType_Number,
	NodeType_Break,
    NodeType_Continue,
	NodeType_ArrayAccess,
	NodeType_ArrayDeclaration,
    NodeType_ArrayUsage,
	NodeType_Variable,
	NodeType_Iteration,
	NodeType_Constant // Constants like numbers
} NodeType;

typedef struct ArrayMetadata {
    int dimensions[2];  // Supports 2D arrays 
} ArrayMetadata;

typedef struct ASTNode {
    NodeType type;
    char* name;
    char* value;
    struct ASTNode* condition;
    struct ASTNode** children;
    int childCount;
	int capacity;
	int* dimSize;
    struct ASTNode* parent;
    struct ASTNode** params;
    int paramCount;
    struct ASTNode* body;
    char* dataType;
    struct ASTNode* functions; // Container for functions
    struct ASTNode* mainFunction; // container for main
	struct ASTNode* initializationExpression;
	struct ASTNode* increment; // Specifically for 'for' loops
    
    struct ASTNode* left;  // Left child for binary operations
    struct ASTNode* right; // Right child for binary operations
	OperationType operation;
    
	// Add array-specific members
    int dimensions;
    char *arrayType;
    
    struct ASTNode *initExpr;
    char *arrayName;
    struct ASTNode *indexExpr;
	int indexCount;
	struct ASTNode** indices;
    // Other necessary fields
	struct ArrayMetadata* extra;
} ASTNode;




typedef struct MatchEntry {
    char* nodeName1;          // Name of the node from AST1
    char* nodeName2;          // Name of the node from AST2
    int dimensionsMatch; 
	int dataTypeMatch;	// Score for dimensions matching
    int initializationMatch;  // Score for initialization matching
    int indexMatch;           // Score for index matching (useful for array usages)
    char* details;            // Additional details about what was compared
    int totalScore;           // Sum of all individual scores
	int declarationMatch;
} MatchEntry;


typedef struct MatchTable {
    MatchEntry* entries;   // Pointer to an array of MatchEntry
    int count;             // Number of entries currently stored
    int capacity;          // Total capacity of the match table
} MatchTable;



typedef struct MatchTable MatchTable;



ASTNode* createASTNode(NodeType type, char* name);
ASTNode* createFunctionNode(char* name, ASTNode* paramList, ASTNode* body);
ASTNode* createFunctionCallNode(char* name, ASTNode* params);
ASTNode* createMainFunctionNode(char* name, ASTNode* compoundStatement);
ASTNode* createParameterNode(char* type);
ASTNode* createForNode(ASTNode* init, ASTNode* cond, ASTNode* incr, ASTNode* body);
ASTNode* createArrayNode(char* type, char* name, char* size);
ASTNode* create2DArrayNode(char* type, char* name, char* size1, char* size2);
ASTNode** collectNodesWrapper(ASTNode* root, int size);
MatchTable* createMatchTable(ASTNode** nodes1, ASTNode** nodes2, int size1, int size2);
void printAST(ASTNode* node);
int countParams(ASTNode* paramList);
void freeASTNode(ASTNode* node);
const char* getOperationName(int operationCode);

void addASTChild(ASTNode* parent, ASTNode* child);
int isFunctionDefinition(ASTNode* node);
int compareASTNodes(ASTNode *node1, ASTNode *node2);
int compareSameTypeNodes(ASTNode *node1, ASTNode *node2);
int compareDifferentTypeNodes(ASTNode* node1, ASTNode* node2);
int compareAssignmentAndFunctionCall(ASTNode* assign, ASTNode* funcCall);
int compareAssignmentAndReturn(ASTNode* assign, ASTNode* ret);

int deepCompareASTNodes(ASTNode *node1, ASTNode *node2);
int calculateFunctionSimilarity(ASTNode *node1, ASTNode *node2);


int compareReturnStatements(ASTNode *node1, ASTNode *node2);
int compareExpressions(ASTNode *expr1, ASTNode *expr2);
int compareFunctionBodies(ASTNode *body1, ASTNode *body2);
int recursiveBodyComparison(ASTNode* body1, ASTNode* body2);
int compareFunctionCalls(ASTNode* call1, ASTNode* call2);
int compareMainFunctions(ASTNode *main1, ASTNode *main2);
int compareStatementBlocks(ASTNode* node1, ASTNode* node2);
int genericNodeComparison(ASTNode* node1, ASTNode* node2);
int compareParameters(ASTNode** params1, ASTNode** params2, int paramCount1, int paramCount2);
int compareBodies(ASTNode* body1, ASTNode* body2);
int compareBodyNodes(ASTNode* node1, ASTNode* node2);
int compareValues(ASTNode* value1, ASTNode* value2);
int compareAssignments(ASTNode* node1, ASTNode* node2);
int compareFunctionDefinition(ASTNode* node1, ASTNode* node2);
int evaluateFunctionCallContext(ASTNode* call, ASTNode* context);
int isCommutative(const char* opName);
int compareConstants(ASTNode* node1, ASTNode* node2);
MatchTable* initializeMatchTable();
void updateMatchTable(MatchTable* table, const char* name1, const char* name2, int dimMatch, int initMatch, int indexMatch);
MatchEntry* getMatchEntry(MatchTable* table, const char* name1, const char* name2);
void finalizeMatchTable(MatchTable* table);
int basicSemanticMatch(ASTNode* node1, ASTNode* node2);
int compareASTs(ASTNode *root1, ASTNode *root2);
int compareNestedLoops(ASTNode* body1, ASTNode* body2);
int countNodeType(ASTNode* node, NodeType type);
int compareArrayAccesses(ASTNode* access1, ASTNode* access2, MatchTable* table);
int compareMultidimensionalArrays(ASTNode* node1, ASTNode* node2);
int compareControlStatements(ASTNode* stmt1, ASTNode* stmt2);
int compareMainFunctions(ASTNode* main1, ASTNode* main2);
int compareExpressionsDeep(ASTNode* expr1, ASTNode* expr2);
int isDefaultInitialized(ASTNode* node);

int compareArrayUsages(ASTNode* usage1, ASTNode* usage2, MatchTable* table);
int compareArrayDeclarations(ASTNode* decl1, ASTNode* decl2, MatchTable* table);
int compareArrayDimensions(ASTNode* node1, ASTNode* node2, MatchTable* table);
int compareArrayInitializations(ASTNode* node1, ASTNode* node2, MatchTable* table);
int compareArrayIndices(ASTNode* node1, ASTNode* node2, MatchTable* table);
int compareArrays(ASTNode* node1, ASTNode* node2);
int compareExpressionPatterns(ASTNode* expr1, ASTNode* expr2);
void initMatchTable(MatchTable* table, int initialCapacity);
void addMatchEntry(MatchTable* table, MatchEntry entry);
void freeMatchTable(MatchTable* table);
int countNodes(ASTNode* root, NodeType type);
void traverseAndCollect(ASTNode* node, NodeType type, ASTNode*** nodes, int* count, int* capacity);
int countNodesOfType(ASTNode* root, NodeType type);
ASTNode** collectNodesOfType(ASTNode* root, NodeType type, int* count);
void collectNodes(ASTNode* root, ASTNode** array, int* index);
int compareSingleASTNode(ASTNode* node1, ASTNode* node2);
ASTNode* createArrayAccessNode(char* arrayName, char* dataType, ASTNode** indices, int indexCount);
ASTNode* findElseBranch(ASTNode* node);

int computeASTDepth(ASTNode* node);
ASTNode* createArrayDeclarationNode(char* name, char* type, int* dimSize, int numDimensions, ASTNode* initExpr);
ASTNode* deepCloneASTNode(ASTNode* original);
int compareArrayNodes(ASTNode* node1, ASTNode* node2, MatchTable* matchTable);


#endif

