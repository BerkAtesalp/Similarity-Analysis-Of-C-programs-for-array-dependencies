#include <stdio.h>
#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define NodeType_Any -1

const int initial_capacity = 10;

ASTNode* globalRoot = NULL;  // Define and initialize globalRoot

const char* getOperationName(int operationCode) {
    switch (operationCode) {
        case OPERATION_ADD: return "ADD";
        case OPERATION_SUBTRACT: return "SUBTRACT";
        case OPERATION_MULTIPLY: return "MULTIPLY";
        case OPERATION_DIVIDE: return "DIVIDE";
        default: return "NONE";
    }
}


int max(int a, int b) {
    return (a > b) ? a : b;
}

int min(int a, int b) {
    return a < b ? a : b;
}

ASTNode* createASTNode(NodeType type, char* name) {
    fprintf(stderr, "Attempting to create a new AST Node. Type: %d, Name: %s\n", type, name ? name : "NULL");

    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed for ASTNode.\n");
        return NULL;
    }

    node->type = type;
    node->name = name ? strdup(name) : NULL;
    node->children = NULL;
    node->childCount = 0;
    node->params = NULL;
    node->paramCount = 0;
    node->body = NULL;
    node->functions = NULL;
    node->mainFunction = NULL;
    node->condition = NULL;
    node->dataType = NULL;  // This will be set later for array accesses
    node->value = NULL;
    node->parent = NULL;
    node->indices = NULL;
    node->indexCount = 0;
    node->dimensions = 0;
    node->arrayType = NULL;
    node->dimSize = NULL;
    node->initExpr = NULL;
    node->arrayName = NULL;  // Will be set specifically for array accesses
    node->indexExpr = NULL;  // Will be set specifically for array accesses

    fprintf(stderr, "Node created successfully. Initial attributes set.\n");

    // Initialize children array for specific node types
    if (type == NodeType_Functions || type == NodeType_Statements || type == NodeType_Body || type == NodeType_ParameterList) {
        node->children = malloc(sizeof(ASTNode*) * 10);
        if (!node->children) {
            fprintf(stderr, "Failed to allocate memory for children nodes.\n");
            freeASTNode(node);
            return NULL;
        }
        fprintf(stderr, "Children array initialized with initial capacity.\n");
    }

    // Setup for array or array declarations
    if (type == NodeType_Array || type == NodeType_ArrayDeclaration) {
        node->dimSize = malloc(sizeof(int) * 2); // Assuming 2D arrays at most
        if (!node->dimSize) {
            fprintf(stderr, "Failed to allocate memory for dimension sizes.\n");
            freeASTNode(node);
            return NULL;
        }
        fprintf(stderr, "Dimension sizes array initialized for Array node.\n");
    }

    // Additional setup for array accesses
    // Additional setup for array accesses
	if (type == NodeType_ArrayAccess) {
		node->arrayName = strdup(name); // Assuming the name is the array name for simplicity
    
		// Allocate memory for index expressions array assuming a maximum of 2 indices for simplicity
		node->indices = malloc(sizeof(ASTNode*) * 2);
		if (!node->indices) {
			fprintf(stderr, "Failed to allocate memory for index expressions.\n");
			freeASTNode(node);
			return NULL;
    }
    node->indexCount = 0; // Initialize index count to 0, to be set when indices are actually added
    fprintf(stderr, "Setup for array access node complete with index expressions array initialized.\n");
}


    // Setup for functions and loops
    if (type == NodeType_FunctionDef || type == NodeType_For || type == NodeType_While) {
        node->params = malloc(sizeof(ASTNode*) * 5);
        node->paramCount = 0;
        if (!node->params) {
            fprintf(stderr, "Failed to allocate memory for parameters.\n");
            freeASTNode(node);
            return NULL;
        }
        fprintf(stderr, "Parameter array initialized for function or loop node.\n");
    }

    fprintf(stderr, "AST Node fully initialized and ready for use.\n");
    return node;
}



void freeASTNode(ASTNode* node) {
    if (!node) return;

    // Free simple dynamically allocated memory
    free(node->name);
    free(node->dataType);
    free(node->arrayType);

    // Free the array of children nodes
    if (node->children) {
        for (int i = 0; i < node->childCount; i++) {
            freeASTNode(node->children[i]);  // Recursively free each child
        }
        free(node->children);
    }

    // Free the parameters if any
    if (node->params) {
        for (int i = 0; i < node->paramCount; i++) {
            freeASTNode(node->params[i]);  // Recursively free each parameter
        }
        free(node->params);
    }

    // Free dimension sizes, these are integers and should not be freed individually
    if (node->dimSize) {
        free(node->dimSize);  // Only free the pointer to the array of ints
    }

    // Free indices if they are pointers to ASTNodes, otherwise just free the array
    if (node->indices) {
        if (node->type == NodeType_ArrayAccess) {
            for (int i = 0; i < node->indexCount; i++) {
                freeASTNode(node->indices[i]);  // Assuming indices are pointers to ASTNodes
            }
        }
        free(node->indices);
    }

    // Finally, free the node itself
    free(node);
}


int countNodes(ASTNode* root, NodeType type) {
    if (!root) return 0;

    int count = 0;
    int capacity = 100; // Initial stack capacity
    ASTNode** stack = malloc(capacity * sizeof(ASTNode*));
    if (!stack) {
        fprintf(stderr, "Memory allocation failed for stack in countNodes.\n");
        return -1; // Memory allocation failure
    }

    int top = 0;
    stack[top++] = root;

    while (top > 0) {
        ASTNode* current = stack[--top];

        // Count only nodes of the specified type
        if (current->type == type) {
            count++;
        }

        // Add children to stack
        for (int i = 0; i < current->childCount; i++) {
            if (current->children[i]) {
                if (top == capacity) { // Check if stack needs expansion
                    capacity *= 2;
                    ASTNode** newStack = realloc(stack, capacity * sizeof(ASTNode*));
                    if (!newStack) {
                        fprintf(stderr, "Stack resizing failed in countNodes.\n");
                        free(stack);
                        return -1; // Handle reallocation failure
                    }
                    stack = newStack;
                }
                stack[top++] = current->children[i];
            }
        }
    }

    free(stack);
    return count;
}



void addASTChild(ASTNode* parent, ASTNode* child) {
    if (parent == NULL || child == NULL) {
        fprintf(stderr, "Error: Trying to add a child to a NULL parent or child is NULL.\n");
        return;
    }

    // Initialize children array if it's the first child
    if (parent->children == NULL) {
        int initial_capacity = 10;  // Define an initial capacity if not defined
        parent->children = malloc(sizeof(ASTNode*) * initial_capacity);
        if (parent->children == NULL) {
            fprintf(stderr, "Memory allocation failed for children array.\n");
            return;
        }
        parent->childCount = 0;
        parent->capacity = initial_capacity;  // Now initializing capacity
        fprintf(stderr, "Children array initialized with initial capacity of %d.\n", initial_capacity);
    }

    // Expand the children array if it is full
    if (parent->childCount == parent->capacity) {
        int new_capacity = parent->capacity + 10; // Increase capacity by a fixed increment, say 10
        ASTNode** new_children = realloc(parent->children, new_capacity * sizeof(ASTNode*));
        if (new_children == NULL) {
            fprintf(stderr, "Memory reallocation failed for children array.\n");
            return;
        }
        parent->children = new_children;
        parent->capacity = new_capacity;
        fprintf(stderr, "Children array expanded to new capacity of %d.\n", new_capacity);
    }

    // Add the new child to the array
    parent->children[parent->childCount++] = child;
    fprintf(stderr, "Successfully added child node to parent node. Total children now: %d.\n", parent->childCount);
}






int isCommutative(const char* opName) {
    if (!opName) {
        fprintf(stderr, "Null operator name provided to isCommutative function.\n");
        return 0;
    }

    static const char* commutativeOps[] = {"+", "*", "&&", "||", "&", "|", "^", NULL};
    for (int i = 0; commutativeOps[i]; i++) {
        if (strcmp(opName, commutativeOps[i]) == 0) return 1;
    }

    return 0;
}




ASTNode* deepCloneASTNode(ASTNode* original) {
    if (!original) return NULL;

    ASTNode* clone = createASTNode(original->type, original->name);
    if (!clone) return NULL;

    

    return clone;
}

ASTNode* createFunctionNode(char* name, ASTNode* paramList, ASTNode* body) {
    ASTNode* node = createASTNode(NodeType_FunctionDef, name);
    if (!node) {
        fprintf(stderr, "Failed to create function node.\n");
        return NULL;
    }

    node->children = malloc(sizeof(ASTNode*) * 2); // Allocate space for parameter list and body
    if (!node->children) {
        fprintf(stderr, "Memory allocation failed for children in function node.\n");
        freeASTNode(node);
        return NULL;
    }

    // Deep clone the parameter list to ensure isolation
    ASTNode* clonedParamList = deepCloneASTNode(paramList);
    if (clonedParamList) {
        node->children[0] = clonedParamList;
        node->paramCount = countNodes(clonedParamList, NodeType_Parameter); // Count only parameter nodes
    } else {
        node->children[0] = NULL;
        node->paramCount = 0;
    }

    // Deep clone the body to ensure that changes do not affect other nodes
    ASTNode* clonedBody = deepCloneASTNode(body);
    node->children[1] = clonedBody;

    node->childCount = 2; // Always two children: parameters and body

    fprintf(stderr, "Function node created: %s with %d parameters and body\n", name, node->paramCount);
    return node;
}


ASTNode* createFunctionCallNode(char* name, ASTNode* params) {
    ASTNode* node = createASTNode(NodeType_FunctionCall, name);
    if (!node) {
        fprintf(stderr, "Memory allocation failed for function call node.\n");
        return NULL;
    }

    if (params) {
        node->params = malloc(sizeof(ASTNode*) * params->childCount);
        if (!node->params) {
            fprintf(stderr, "Memory allocation failed for parameters in function call node.\n");
            freeASTNode(node);
            return NULL;
        }

        for (int i = 0; i < params->childCount; i++) {
            // Clone each parameter to avoid reference issues in different parts of the tree
            ASTNode* clonedParam = deepCloneASTNode(params->children[i]);
            if (clonedParam) {
                node->params[i] = clonedParam;
            } else {
                fprintf(stderr, "Failed to clone parameter for function call.\n");
                freeASTNode(node);
                return NULL;
            }
        }
        node->paramCount = params->childCount;
    }

    fprintf(stderr, "Function call node created: %s with %d parameters\n", name, node->paramCount);
    return node;
}





ASTNode* createMainFunctionNode(char* name, ASTNode* compoundStatement) {
    ASTNode* node = createASTNode(NodeType_MainFunction, name);
    if (!node) {
        fprintf(stderr, "Failed to create main function node.\n");
        return NULL;
    }

    // Ensure the compound statement is not NULL before attaching
    if (compoundStatement) {
        node->body = deepCloneASTNode(compoundStatement); // Clone to ensure independence
        fprintf(stderr, "Main function node created: %s with attached body.\n", name);
    } else {
        fprintf(stderr, "No body provided for main function node: %s. Creating an empty body.\n", name);
        node->body = createASTNode(NodeType_Body, "EmptyBody");
        if (!node->body) {
            freeASTNode(node);
            fprintf(stderr, "Failed to create an empty body for the main function node.\n");
            return NULL;
        }
    }

    return node;
}


int compareArrayDimensions(ASTNode* node1, ASTNode* node2, MatchTable* table) {
    if (!node1 || !node2) {
        fprintf(stderr, "Error: One or both array nodes are null.\n");
        return 0;
    }

    MatchEntry entry = {node1->name, node2->name, 0, 0, 0, 0};

    // Check dimensions count
    if (node1->dimensions != node2->dimensions) {
        fprintf(stderr, "Dimension mismatch for arrays %s and %s: %d vs %d\n", node1->name, node2->name, node1->dimensions, node2->dimensions);
        return 0; // No match if the dimension counts differ
    }

    int dimScore = 0;
    for (int i = 0; i < node1->dimensions; i++) {
        if (node1->dimSize[i] == node2->dimSize[i]) {
            dimScore += 100 / node1->dimensions; // Evenly distribute score across dimensions
        } else {
            fprintf(stderr, "Dimension size mismatch at dimension %d for arrays %s and %s: %d vs %d\n", i+1, node1->name, node2->name, node1->dimSize[i], node2->dimSize[i]);
        }
    }

    entry.dimensionsMatch = dimScore; // Store the computed score
    entry.totalScore += entry.dimensionsMatch; // Update total score with dimensions match score
    addMatchEntry(table, entry); // Add the match entry to the table

    fprintf(stderr, "Dimension comparison score for arrays '%s' and '%s': %d\n", node1->name, node2->name, dimScore);

    return dimScore; // Return the dimension score
}





ASTNode* createParameterNode(char* type) {
    ASTNode* node = createASTNode(NodeType_Parameter, type);
    if (!node) {
        fprintf(stderr, "Failed to create parameter node.\n");
        return NULL;
    }

    // Initialize more attributes if necessary
    // Example: Adding default values or type checks
    node->dataType = strdup(type);  // Ensure the type is copied and managed independently
    node->value = NULL;  // Placeholder for default value or further extension

    fprintf(stderr, "Parameter node created: Type %s\n", type);
    return node;
}



ASTNode* createForNode(ASTNode* init, ASTNode* cond, ASTNode* incr, ASTNode* body) {
    ASTNode* node = createASTNode(NodeType_For, "for");
    if (!node) {
        fprintf(stderr, "Failed to create 'for' loop node.\n");
        return NULL;
    }

    // Ensure all components are non-null; provide default empty nodes if null
    init = init ? init : createASTNode(NodeType_ForInit, "EmptyInit");
    cond = cond ? cond : createASTNode(NodeType_Expression, "True"); // Default condition to true
    incr = incr ? incr : createASTNode(NodeType_ForIncrement, "EmptyIncr");
    body = body ? body : createASTNode(NodeType_Body, "EmptyBody");

    // Check for any failures in creating default nodes
    if (!init || !cond || !incr || !body) {
        fprintf(stderr, "Failed to create default components for 'for' loop node.\n");
        freeASTNode(node);
        if (init) freeASTNode(init);
        if (cond) freeASTNode(cond);
        if (incr) freeASTNode(incr);
        if (body) freeASTNode(body);
        return NULL;
    }

    // Allocate space for four children: init, condition, increment, and body
    node->children = malloc(sizeof(ASTNode*) * 4);
    if (!node->children) {
        fprintf(stderr, "Memory allocation failed for children in 'for' node.\n");
        freeASTNode(node);
        freeASTNode(init);
        freeASTNode(cond);
        freeASTNode(incr);
        freeASTNode(body);
        return NULL;
    }

    node->children[0] = init;
    node->children[1] = cond;
    node->children[2] = incr;
    node->children[3] = body;
    node->childCount = 4;

    fprintf(stderr, "'For' node created with init, condition, increment, and body.\n");
    return node;
}


ASTNode* createArrayDeclarationNode(char* name, char* type, int* dimSize, int numDimensions, ASTNode* initExpr) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed for array declaration node.\n");
        return NULL;
    }
    
    node->type = NodeType_ArrayDeclaration;
    node->name = strdup(name);
    node->dataType = strdup(type);
    node->dimensions = numDimensions;
    node->dimSize = malloc(sizeof(int) * numDimensions);
    if (!node->dimSize) {
        fprintf(stderr, "Failed to allocate memory for dimension sizes.\n");
        free(node->name);
        free(node->dataType);
        free(node);
        return NULL;
    }
    for (int i = 0; i < numDimensions; i++) {
        node->dimSize[i] = dimSize[i];
    }
    node->initExpr = initExpr;
    node->children = NULL;
    node->childCount = 0;

    return node;
}

// Function to initialize the match table with an initial capacity
void initMatchTable(MatchTable* table, int initialCapacity) {
    table->entries = (MatchEntry*)malloc(sizeof(MatchEntry) * initialCapacity);
    if (table->entries == NULL) {
        fprintf(stderr, "Failed to allocate memory for match table entries.\n");
        exit(EXIT_FAILURE);
    }
    table->count = 0;
    table->capacity = initialCapacity;
}

// Function to add an entry to the match table
void addMatchEntry(MatchTable* table, MatchEntry entry) {
    if (table->count >= table->capacity) {
        // Resize the table if necessary
        int newCapacity = table->capacity * 2;
        MatchEntry* newEntries = (MatchEntry*)realloc(table->entries, sizeof(MatchEntry) * newCapacity);
        if (newEntries == NULL) {
            fprintf(stderr, "Failed to reallocate memory for match table entries.\n");
            exit(EXIT_FAILURE);
        }
        table->entries = newEntries;
        table->capacity = newCapacity;
    }
    table->entries[table->count] = entry;
    table->count++;
}

// Function to free the match table
void freeMatchTable(MatchTable* table) {
    free(table->entries);
    table->entries = NULL;
    table->count = 0;
    table->capacity = 0;
}



ASTNode* createArrayNode(char* type, char* name, char* size) {
    if (!type || !name || !size) {
        fprintf(stderr, "Invalid parameters for creating an array node.\n");
        return NULL;
    }

    // Ensure the node is created as an array declaration type
    ASTNode* node = createASTNode(NodeType_ArrayDeclaration, name);
    if (!node) {
        fprintf(stderr, "Failed to create array declaration node for %s.\n", name);
        return NULL;
    }

    // Duplicate the type for the data type of the node
    node->dataType = strdup(type);
    if (!node->dataType) {
        fprintf(stderr, "Memory allocation failed for array type in %s.\n", name);
        freeASTNode(node);
        return NULL;
    }

    // Initialize dimensions size array and set the first dimension
    node->dimensions = 1;  // This node represents a single dimension array
    node->dimSize = malloc(sizeof(int) * node->dimensions);  // Allocate memory for one dimension
    if (!node->dimSize) {
        fprintf(stderr, "Failed to allocate memory for dimension size in %s.\n", name);
        free(node->dataType);
        freeASTNode(node);
        return NULL;
    }

    // Convert the size from string to integer and store it
    node->dimSize[0] = atoi(size);

    fprintf(stderr, "Array node created: Type %s, Name %s, Size %s\n", type, name, size);
    return node;
}



ASTNode* create2DArrayNode(char* type, char* name, char* size1, char* size2) {
    if (!type || !name || !size1 || !size2) {
        fprintf(stderr, "Invalid parameters for creating a 2D array node.\n");
        return NULL;
    }

    ASTNode* node = createASTNode(NodeType_ArrayDeclaration, name);
    if (!node) {
        fprintf(stderr, "Failed to create 2D array declaration node for %s.\n", name);
        return NULL;
    }

    node->dataType = strdup(type);
    if (!node->dataType) {
        fprintf(stderr, "Memory allocation failed for array type in %s.\n", name);
        freeASTNode(node);
        return NULL;
    }

    // Allocate memory for two dimensions
    node->dimensions = 2;
    node->dimSize = malloc(sizeof(int) * node->dimensions);
    if (!node->dimSize) {
        fprintf(stderr, "Failed to allocate memory for dimensions in %s.\n", name);
        free(node->dataType);
        freeASTNode(node);
        return NULL;
    }

    // Convert dimension sizes from string to integer and store them
    node->dimSize[0] = atoi(size1);
    node->dimSize[1] = atoi(size2);

    fprintf(stderr, "2D Array node created: Type %s, Name %s, Sizes %s and %s\n", type, name, size1, size2);
    return node;
}



ASTNode* createArrayAccessNode(char* arrayName, char* dataType, ASTNode** indices, int indexCount) {
    if (!arrayName || !dataType || !indices || indexCount < 1) {
        fprintf(stderr, "Invalid parameters for creating an array access node.\n");
        return NULL;
    }

    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed for array access node.\n");
        return NULL;
    }

    node->type = NodeType_ArrayAccess;
    node->name = strdup(arrayName);
    node->dataType = strdup(dataType);
    node->indices = (ASTNode**)malloc(sizeof(ASTNode*) * indexCount);
    if (!node->indices) {
        fprintf(stderr, "Failed to allocate memory for indices.\n");
        free(node->name);
        free(node->dataType);
        free(node);
        return NULL;
    }

    for (int i = 0; i < indexCount; i++) {
        if (!indices[i]) {
            fprintf(stderr, "Null index expression found.\n");
            while (i-- > 0) {  // Free any already assigned indices to avoid memory leaks
                free(node->indices[i]);
            }
            free(node->indices);
            free(node->name);
            free(node->dataType);
            free(node);
            return NULL;
        }
        node->indices[i] = indices[i];  // Assume indices[i] is already a pointer to a valid ASTNode
    }
    node->indexCount = indexCount;

    return node;
}



int compareASTNodes(ASTNode* node1, ASTNode* node2) {
    if (!node1 || !node2) {
        fprintf(stderr, "Comparison failed: One or both nodes are null.\n");
        return 0; // Return immediately if either node is null.
    }

    fprintf(stderr, "Comparing nodes of type %d and %d\n", node1->type, node2->type);

    int score = 0;

    // Check for type matching and provide a high initial score for matching types.
    if (node1->type == node2->type) {
        fprintf(stderr, "Node types match. Initial score set to 30.\n");
        score += 30; // Types match, allocate base score.

        if (node1->name && node2->name && strcmp(node1->name, node2->name) == 0) {
            fprintf(stderr, "Node names match (%s). Adding 20 to score.\n", node1->name);
            score += 20; // Increase score for matching names.
        }

        if (node1->value && node2->value && strcmp(node1->value, node2->value) == 0) {
            fprintf(stderr, "Node values match (%s). Adding 10 to score.\n", node1->value);
            score += 10; // Values match, increase score.
        }

        if (node1->dataType && node2->dataType && strcmp(node1->dataType, node2->dataType) == 0) {
            fprintf(stderr, "Node data types match (%s). Adding 10 to score.\n", node1->dataType);
            score += 10; // Data types match, further increase score.
        }

        int childScoreSum = 0;
        for (int i = 0; i < min(node1->childCount, node2->childCount); i++) {
            int childScore = compareASTNodes(node1->children[i], node2->children[i]);
            fprintf(stderr, "Child %d score: %d\n", i + 1, childScore);
            childScoreSum += childScore; // Sum child scores.
        }

        if (node1->childCount > 0 && node2->childCount > 0) {
            score += (childScoreSum / min(node1->childCount, node2->childCount)); // Average child scores.
        }
    } else {
        fprintf(stderr, "Node types do not match. Minimal score of 5 allocated.\n");
        score += 5; // Minimal score for non-matching types to recognize effort.
    }

    fprintf(stderr, "Total score before normalization: %d\n", score);
    int normalizedScore = (score * 100) / (70 + node1->childCount * 100); // Adjusted normalization
    fprintf(stderr, "Normalized score: %d%%\n", normalizedScore);

    return normalizedScore;
}




int compareFunctionCalls(ASTNode* call1, ASTNode* call2) {
    if (!call1 || !call2) return 0; // Check for null pointers.
    if (call1->type != NodeType_FunctionCall || call2->type != NodeType_FunctionCall) return 0; // Ensure both nodes are function calls.

    int score = 0;

    // Check if the function names match (simple case).
    if (strcmp(call1->name, call2->name) == 0) {
        score += 20; // Basic match score for the same function name.

        // Check parameter count
        if (call1->paramCount == call2->paramCount) {
            score += 10; // Same number of parameters.
            for (int i = 0; i < call1->paramCount; i++) {
                score += compareASTNodes(call1->params[i], call2->params[i]); // Compare each parameter.
            }
        } else {
            // Handle cases for overloaded functions or different argument counts gracefully
            int minParams = min(call1->paramCount, call2->paramCount);
            for (int i = 0; i < minParams; i++) {
                score += compareASTNodes(call1->params[i], call2->params[i]); // Compare up to the lesser count.
            }
            score -= abs(call1->paramCount - call2->paramCount) * 5; // Deduct points for each unmatched parameter.
        }
    }

    return score; // Return the final computed score.
}

void printAST(ASTNode* root) {
    if (!root) {
        printf("AST is NULL.\n");
        return;
    }

    void printASTNode(ASTNode* node, int level) {
        if (!node) return; // Guard against null node references.

        for (int i = 0; i < level; i++) printf("  "); // Indent based on the depth level.

        printf("Node Type: %d, Name: %s", node->type, node->name ? node->name : "Unnamed");

        if (node->value) {
            printf(", Value: %s", node->value); // Print value if available.
        }

        if (node->dataType) {
            printf(", Data Type: %s", node->dataType); // Print data type if available.
        }

        if (node->paramCount > 0) {
            printf(", Params: [");
            for (int i = 0; i < node->paramCount; i++) {
                if (i > 0) printf(", ");
                printf("%s", node->params[i]->name);
            }
            printf("]");
        }

        if (node->childCount > 0) {
            printf(", Children: %d\n", node->childCount);
            for (int i = 0; i < node->childCount; i++) {
                printASTNode(node->children[i], level + 1); // Recursive call to print children.
            }
        } else {
            printf("\n");
        }
    }

    printf("Printing AST:\n");
    printASTNode(root, 0); // Start from the root at level 0.
}


int compareFunctionBodies(ASTNode* body1, ASTNode* body2) {
    if (!body1 || !body2) {
        fprintf(stderr, "Comparison failed: One or both function bodies are null.\n");
        return 0; // Early exit if any body is null.
    }

    fprintf(stderr, "Starting comparison of function bodies.\n");

    int score = 0;
    int baseScore = 20; // Base score for matching the count of statements.
    int detailedScore = 0;

    // Compare the number of statements in both function bodies.
    if (body1->childCount == body2->childCount) {
        score += baseScore; // Assign base score for having the exact number of statements.
    } else {
        // Provide partial points for similar but not identical counts to handle minor changes in code.
        int countDifference = abs(body1->childCount - body2->childCount);
        score += baseScore - (countDifference * 2); // Subtract points based on the difference in count.
        fprintf(stderr, "Partial score due to difference in statement counts.\n");
    }

    // Compare each statement in the function bodies for detailed structural similarity.
    int maxStatementsToCompare = min(body1->childCount, body2->childCount);
    for (int i = 0; i < maxStatementsToCompare; i++) {
        int statementScore = compareASTNodes(body1->children[i], body2->children[i]);
        detailedScore += statementScore;
        fprintf(stderr, "Statement %d comparison score: %d\n", i, statementScore);
    }

    if (maxStatementsToCompare > 0) {
        // Normalize the detailed score by the number of statements compared to average their impact.
        detailedScore /= maxStatementsToCompare;
    }

    // Add the normalized detailed score to the total score.
    score += detailedScore;

    // Normalize the total score to 100 scale considering the base score and the possible maximum score.
    int maxPossibleScore = baseScore + 100; // Assuming each statement could contribute up to 100 points.
    if (body1->childCount > 0) {
        maxPossibleScore = baseScore + (body1->childCount * 100); // Adjust max score based on the number of children.
    }
    int normalizedScore = (score * 100) / maxPossibleScore;

    fprintf(stderr, "Final normalized score for function bodies: %d%%\n", normalizedScore);

    return normalizedScore;
}

int countNodesOfType(ASTNode* root, NodeType type) {
    if (!root) return 0;

    int count = 0;
    int capacity = 100; // Initial stack capacity
    ASTNode** stack = malloc(capacity * sizeof(ASTNode*));
    if (!stack) {
        fprintf(stderr, "Memory allocation failed for stack in countNodesOfType.\n");
        return -1; // Memory allocation failure
    }

    int top = 0;
    stack[top++] = root;

    while (top > 0) {
        ASTNode* current = stack[--top];

        // Count only nodes of the specified type
        if (current->type == type) {
            count++;
        }

        // Add children to stack
        for (int i = 0; i < current->childCount; i++) {
            if (current->children[i]) {
                if (top == capacity) { // Check if stack needs expansion
                    capacity *= 2;
                    ASTNode** newStack = realloc(stack, capacity * sizeof(ASTNode*));
                    if (!newStack) {
                        fprintf(stderr, "Stack resizing failed in countNodesOfType.\n");
                        free(stack);
                        return -1; // Handle reallocation failure
                    }
                    stack = newStack;
                }
                stack[top++] = current->children[i];
            }
        }
    }

    free(stack);
    return count;
}


int compareExpressions(ASTNode *expr1, ASTNode *expr2) {
    if (!expr1 || !expr2) {
        fprintf(stderr, "Error: One of the expression nodes is null.\n");
        return 0; // Early exit if any expression is null.
    }

    int score = 0;

    // Check for type matching and handle different expression types with specific logic.
    if (expr1->type != expr2->type) {
        fprintf(stderr, "Type mismatch: %d vs %d\n", expr1->type, expr2->type);
        return 0; // Exit if expression types don't match.
    }

    // Constants or identifiers comparison
    if (expr1->type == NodeType_Constant || expr1->type == NodeType_Identifier) {
        if (strcmp(expr1->name, expr2->name) == 0) {
            score += 100; // Exact match for identifiers or constants.
            fprintf(stderr, "Exact match for identifiers or constants.\n");
        } else if (expr1->value && expr2->value) { // Compare values if available
            double val1 = strtod(expr1->value, NULL);
            double val2 = strtod(expr2->value, NULL);
            double tolerance = 0.01; // Tolerance for comparing floating-point numbers
            if (val1 == val2) {
                score += 100; // Numeric values match exactly.
                fprintf(stderr, "Numeric values match exactly.\n");
            } else if (fabs(val1 - val2) <= tolerance) { // Compare within a small tolerance
                score += 75; // Award a score for values that are close within a specified tolerance.
                fprintf(stderr, "Numeric values match within tolerance of %.2f.\n", tolerance);
            }
        }
    }

    // Detailed comparison for binary and unary expressions
    if (expr1->type == NodeType_Expression && expr1->children && expr2->children) {
        fprintf(stderr, "Comparing compound expressions.\n");
        // Handle commutative operations where the order of operands doesn't matter
        if (isCommutative(expr1->name) && strcmp(expr1->name, expr2->name) == 0) {
            int normalOrder = compareExpressions(expr1->children[0], expr2->children[0]) +
                              compareExpressions(expr1->children[1], expr2->children[1]);
            int reverseOrder = compareExpressions(expr1->children[0], expr2->children[1]) +
                               compareExpressions(expr1->children[1], expr2->children[0]);
            score += max(normalOrder, reverseOrder);
            fprintf(stderr, "Commuted score: %d\n", score);
        } else if (strcmp(expr1->name, expr2->name) == 0) {
            score += compareExpressions(expr1->children[0], expr2->children[0]) * 0.6 +
                     compareExpressions(expr1->children[1], expr2->children[1]) * 0.4;
        }
    }

    // Normalize and cap the score
    score = (score > 100) ? 100 : score;
    fprintf(stderr, "Normalized score for expressions: %d\n", score);

    return score;
}




int compareExpressionsDeep(ASTNode* expr1, ASTNode* expr2) {
    if (!expr1 || !expr2) return 0; // Null checks

    // Basic type and name comparison
    if (expr1->type != expr2->type || strcmp(expr1->name, expr2->name) != 0)
        return 0;

    // Recursively compare children nodes if both expressions have children
    if (expr1->children && expr2->children) {
        int score = 0;
        for (int i = 0; i < min(expr1->childCount, expr2->childCount); i++) {
            score += compareExpressionsDeep(expr1->children[i], expr2->children[i]);
        }
        return score / max(1, min(expr1->childCount, expr2->childCount)); // Normalize score by number of children
    }

    // If no children, compare values directly (if applicable)
    if (expr1->value && expr2->value) {
        return strcmp(expr1->value, expr2->value) == 0 ? 100 : 0;
    }

    return 100; // If reached here, expressions matched at all checked levels
}

int compareArrayNodes(ASTNode* node1, ASTNode* node2, MatchTable* matchTable) {
    if (!node1 || !node2) {
        fprintf(stderr, "Error: One or both array nodes are null.\n");
        return 0;
    }

    int score = 0;
    int possibleScore = 300;  // Assuming each sub-comparison can contribute up to 100 points

    // Compare dimensions
    int dimScore = compareArrayDimensions(node1, node2, matchTable);
    score += dimScore;

    // Compare initializations
    if (node1->initExpr && node2->initExpr) {
        int initScore = compareArrayInitializations(node1, node2, matchTable);
        score += initScore;
    }

    // Compare indices if applicable
    if (node1->indices && node2->indices) {
        int indexScore = compareArrayIndices(node1, node2, matchTable);
        score += indexScore;
    }

    fprintf(stderr, "Total array comparison score: %d out of %d\n", score, possibleScore);
    return score;
}




int isDefaultInitialized(ASTNode* node) {
    if (!node) return 0; // If node is NULL, no initialization exists.

    // Check if the node is a simple constant initialization, which could be considered a default initialization
    if (node->type == NodeType_Constant) {
        // Assuming 'value' is a string that can be compared to a default value like "0" or other similar defaults
        if (strcmp(node->value, "0") == 0 || strcmp(node->value, "NULL") == 0) {
            return 1; // Recognize zero or NULL as default initializations
        }
    }
    // For more complex initializations involving multiple values or constructors
    else if (node->type == NodeType_Expression && node->name && strcmp(node->name, "init") == 0) {
        // Check if all child nodes of an expression are default values
        int allDefault = 1;
        for (int i = 0; i < node->childCount; i++) {
            if (!isDefaultInitialized(node->children[i])) {
                allDefault = 0;
                break;
            }
        }
        return allDefault;
    }

    return 0; // Default case if the initialization does not match known default patterns
}


int compareArrayInitializations(ASTNode* node1, ASTNode* node2, MatchTable* table) {
    if (!node1 || !node2) {
        fprintf(stderr, "Error: One or both array initialization nodes are null.\n");
        return 0;
    }

    // Initialize the entry for the match table with the relevant node names
    MatchEntry entry = {node1->name, node2->name, 0, 0, 0, 0};

    // Compare initialization expressions if both nodes have an initialization
    if (node1->initExpr && node2->initExpr) {
        int initScore = compareExpressionsDeep(node1->initExpr, node2->initExpr); // Hypothetical function for deep comparison of expressions
        entry.initializationMatch = initScore; // Record the score in the match entry
        entry.totalScore += entry.initializationMatch; // Increment the total score with the initialization score

        fprintf(stderr, "Initialization comparison score for nodes '%s' and '%s': %d\n", node1->name, node2->name, initScore);
    } else {
        fprintf(stderr, "One or both nodes lack initialization expressions.\n");
    }

    // Add the completed entry to the match table
    addMatchEntry(table, entry);

    return entry.initializationMatch; // Return the initialization score from the match entry
}




int compareArrayIndices(ASTNode* node1, ASTNode* node2, MatchTable* table) {
    if (!node1 || !node2) {
        fprintf(stderr, "Error: One or both array index nodes are null.\n");
        return 0;
    }

    // Initialize variables to keep track of the index comparison scores
    int totalScore = 0;
    int countMatchedIndices = 0;

    // Determine the minimum number of indices to compare
    int minIndexCount = min(node1->indexCount, node2->indexCount);

    if (minIndexCount > 0) { // Ensure there are indices to compare
        for (int i = 0; i < minIndexCount; i++) {
            if (compareExpressionsDeep(node1->indices[i], node2->indices[i])) {
                totalScore += 100; // Assume 100 points per matching index expression
                countMatchedIndices++;
            }
        }

        // Calculate the score based on the proportion of matched indices to total indices
        totalScore = (totalScore / minIndexCount); // Normalize the score

        // Log the index comparison results
        fprintf(stderr, "Compared %d indices with a total score of %d.\n", minIndexCount, totalScore);
    } else {
        fprintf(stderr, "No indices to compare for either array node.\n");
    }

    // Update the match table with the index comparison score
    updateMatchTable(table, node1->name, node2->name, 0, 0, totalScore); // Update only the index match score

    return totalScore;
}

void traverseAndCollect(ASTNode* node, NodeType type, ASTNode*** collectedNodes, int* count, int* capacity) {
    if (!node) return;

    fprintf(stderr, "Visiting node %s of type %d.\n", node->name, node->type);

    if (node->type == type) {
        if (*count >= *capacity) {
            // Increase capacity
            int oldCapacity = *capacity;
            *capacity *= 2;
            *collectedNodes = realloc(*collectedNodes, (*capacity) * sizeof(ASTNode*));
            if (!*collectedNodes) {
                fprintf(stderr, "Memory allocation failed during array resizing from %d to %d.\n", oldCapacity, *capacity);
                return;
            }
        }
        (*collectedNodes)[*count] = node;
        fprintf(stderr, "Collected node %s of type %d at index %d.\n", node->name, node->type, *count);
        (*count)++;
    }

    for (int i = 0; i < node->childCount; i++) {
        traverseAndCollect(node->children[i], type, collectedNodes, count, capacity);
    }
}



ASTNode** collectNodesOfType(ASTNode* root, NodeType type, int* count) {
    if (!root) {
        fprintf(stderr, "collectNodesOfType: root is NULL.\n");
        *count = 0;
        return NULL;
    }

    int capacity = 10; // Start with some capacity
    ASTNode** collectedNodes = malloc(capacity * sizeof(ASTNode*));
    if (!collectedNodes) {
        fprintf(stderr, "Memory allocation failed for node collection.\n");
        *count = 0;
        return NULL;
    }

    *count = 0; // Initialize count
    traverseAndCollect(root, type, &collectedNodes, count, &capacity);

    if (*count == 0) {
        fprintf(stderr, "No nodes of specified type (%d) found.\n", type);
        free(collectedNodes);
        return NULL;
    }

    fprintf(stderr, "Collected %d nodes of type %d.\n", *count, type);
    return collectedNodes;
}




MatchTable* initializeMatchTable() {
    MatchTable* table = malloc(sizeof(MatchTable));
    if (!table) {
        fprintf(stderr, "Memory allocation failed for MatchTable\n");
        return NULL;
    }
    table->entries = malloc(sizeof(MatchEntry) * 10); // Initial capacity
    table->count = 0;
    table->capacity = 10;
    return table;
}

// Update the MatchTable with new match data
void updateMatchTable(MatchTable* table, const char* name1, const char* name2, int dimMatch, int initMatch, int indexMatch) {
    if (table->count >= table->capacity) {
        // Resize the table if necessary
        table->capacity *= 2;
        table->entries = realloc(table->entries, sizeof(MatchEntry) * table->capacity);
        if (!table->entries) {
            fprintf(stderr, "Failed to reallocate memory for MatchTable entries\n");
            return;
        }
    }
    MatchEntry entry = {
        .nodeName1 = strdup(name1),
        .nodeName2 = strdup(name2),
        .dimensionsMatch = dimMatch,
        .initializationMatch = initMatch,
        .indexMatch = indexMatch,
        .totalScore = dimMatch + initMatch + indexMatch
    };
    table->entries[table->count++] = entry;
}

// Retrieve a MatchEntry from the MatchTable
MatchEntry* getMatchEntry(MatchTable* table, const char* name1, const char* name2) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->entries[i].nodeName1, name1) == 0 &&
            strcmp(table->entries[i].nodeName2, name2) == 0) {
            return &table->entries[i];
        }
    }
    return NULL;
}

// Clean up and finalize the MatchTable
void finalizeMatchTable(MatchTable* table) {
    for (int i = 0; i < table->count; i++) {
        free(table->entries[i].nodeName1);
        free(table->entries[i].nodeName2);
    }
    free(table->entries);
    free(table);
}


int compareExpressionPatterns(ASTNode* expr1, ASTNode* expr2) {
    if (!expr1 || !expr2) return 0; // Handle NULL expressions

    int score = 0;

    // Compare only identifiers, which are used as indices in the array access
    if (expr1->type == NodeType_Identifier && expr2->type == NodeType_Identifier) {
        if (strcmp(expr1->name, expr2->name) == 0) {
            score += 20; // Exact match of identifiers
            fprintf(stderr, "Matching indices (%s), score: 20\n", expr1->name);
        } else {
            fprintf(stderr, "Indices do not match: %s vs %s\n", expr1->name, expr2->name);
        }
    } else {
        fprintf(stderr, "Non-identifier nodes detected, comparison not applicable for indices.\n");
    }

    return score;
}



int compareArrayDeclarations(ASTNode* decl1, ASTNode* decl2, MatchTable* table) {
    if (!decl1 || !decl2) {
        fprintf(stderr, "Error: One or both array declaration nodes are null.\n");
        return 0;
    }

    MatchEntry entry = {
        .nodeName1 = decl1->name,
        .nodeName2 = decl2->name,
        .dimensionsMatch = 0,
        .initializationMatch = 0,
        .declarationMatch = 0,
        .totalScore = 0,
        .details = "Detailed comparison of array declarations"
    };

    // Compare data types
    if (strcmp(decl1->dataType, decl2->dataType) == 0) {
        fprintf(stderr, "Data types match: %s\n", decl1->dataType);
        entry.declarationMatch = 20; // Base score for matching data types
    }

    // Compare names
    if (strcmp(decl1->name, decl2->name) == 0) {
        fprintf(stderr, "Names match: %s\n", decl1->name);
        entry.declarationMatch += 30; // Additional score for matching names
    }

    // Compare dimensions
    if (decl1->dimensions == decl2->dimensions) {
        fprintf(stderr, "Dimensions count match: %d dimensions\n", decl1->dimensions);
        entry.dimensionsMatch = 20; // Base score for matching dimension count
        for (int i = 0; i < decl1->dimensions; i++) {
            if (decl1->dimSize[i] == decl2->dimSize[i]) {
                entry.dimensionsMatch += 10; // Additional points for each exact matching dimension size
            } else {
                fprintf(stderr, "Mismatch in dimension %d sizes: %d vs %d\n", i + 1, decl1->dimSize[i], decl2->dimSize[i]);
            }
        }
    }

    
    // Calculate total score
    entry.totalScore = entry.declarationMatch + entry.dimensionsMatch + entry.initializationMatch;

    // Log the comparison details
    fprintf(stderr, "Array declaration comparison for %s and %s:\n", decl1->name, decl2->name);
    fprintf(stderr, "Declaration Score: %d, Dimensions Score: %d",
            entry.declarationMatch, entry.dimensionsMatch, entry.totalScore);

    // Add the match entry to the match table
    addMatchEntry(table, entry);

    return entry.totalScore;
}


int compareArrayAccesses(ASTNode* access1, ASTNode* access2, MatchTable* table) {
    if (!access1 || !access2) {
        fprintf(stderr, "Error: One or both array access nodes are null.\n");
        return 0;
    }

    fprintf(stderr, "Starting comparison between array accesses.\n");

    MatchEntry entry = {
        .nodeName1 = access1->name,
        .nodeName2 = access2->name,
        .indexMatch = 0,
        .initializationMatch = 0,
        .totalScore = 0,
        .details = "Detailed comparison of array accesses"
    };

    // Compare indices
    if (access1->indexCount == access2->indexCount) {
        for (int i = 0; i < access1->indexCount; i++) {
            int indexScore = compareExpressionPatterns(access1->indices[i], access2->indices[i]);
            entry.indexMatch += indexScore;
            
        }
    } else {
        fprintf(stderr, "Mismatch in the number of indices.\n");
        return 0;  // Index count mismatch might be critical enough to stop further comparison.
    }

    

    entry.totalScore = entry.indexMatch;

    fprintf(stderr, "Total score for comparison: %d\n", entry.totalScore);
    addMatchEntry(table, entry);

    return entry.totalScore;
}



// Evaluate additional complexity bonus based on the depth and complexity of conditions or bodies
int evaluateComplexityBonus(ASTNode* node1, ASTNode* node2, int baseScore) {
    int bonus = 0;
    int depth1 = computeASTDepth(node1);
    int depth2 = computeASTDepth(node2);
    bonus += (depth1 + depth2) / 2; // Simple metric to add depth-based complexity bonus
    if (baseScore > 50) { // Higher base scores suggest more critical sections
        bonus += 10;
    }
    return bonus;
}

// Helper function to compute depth of an AST
int computeASTDepth(ASTNode* node) {
    if (!node || node->childCount == 0) return 0;
    int maxDepth = 0;
    for (int i = 0; i < node->childCount; i++) {
        int childDepth = computeASTDepth(node->children[i]);
        if (childDepth > maxDepth) {
            maxDepth = childDepth;
        }
    }
    return 1 + maxDepth;
}

// Helper function to find the 'else' branch in if-else structures
ASTNode* findElseBranch(ASTNode* node) {
    for (int i = 0; i < node->childCount; i++) {
        if (node->children[i]->type == NodeType_ElseBody) {
            return node->children[i];
        }
    }
    return NULL;
}

void collectArrayDeclarations(ASTNode* root, ASTNode** arrayDeclarations, int* count) {
    if (root == NULL) return;

    // Check if the current node is an array declaration
    if (root->type == NodeType_ArrayDeclaration) {
        arrayDeclarations[*count] = root;
        (*count)++;
    }

    // Recursively search in children nodes
    for (int i = 0; i < root->childCount; i++) {
        collectArrayDeclarations(root->children[i], arrayDeclarations, count);
    }
}



int compareArrayUsages(ASTNode* usage1, ASTNode* usage2, MatchTable* table) {
    if (!usage1 || !usage2) {
        fprintf(stderr, "Error: One or both array usage nodes are null.\n");
        return 0;
    }

    // Start with a basic check: the array name and the usage type (read or write)
    int usageScore = 0;
    if (strcmp(usage1->name, usage2->name) == 0) {
        usageScore += 30; // Basic score for using the same array

        // Check if the usage context matches (e.g., both are reads or both are writes)
        if (usage1->dataType == usage2->dataType) {
            usageScore += 20; // Additional points for matching usage context
        }

        // Compare each index used in the array access if they exist
        int indexComparisonScore = 0;
        if (usage1->indexCount > 0 && usage2->indexCount > 0) {
            int matchedIndices = 0;
            int minIndexCount = min(usage1->indexCount, usage2->indexCount);
            for (int i = 0; i < minIndexCount; i++) {
                if (compareExpressionsDeep(usage1->indices[i], usage2->indices[i])) {
                    matchedIndices++;
                }
            }
            if (minIndexCount > 0) {
                indexComparisonScore = (matchedIndices * 50) / minIndexCount; // Normalize the index match score
            }
        }
        usageScore += indexComparisonScore; // Add the normalized index match score
    }

    fprintf(stderr, "Comparing array usages '%s' and '%s': Score=%d\n", usage1->name, usage2->name, usageScore);

    // Correct initialization of the match entry with proper memory handling
    MatchEntry entry = {
        .nodeName1 = strdup(usage1->name),
        .nodeName2 = strdup(usage2->name),
        .dimensionsMatch = 0,
        .initializationMatch = 0,
        .indexMatch = 0,
        .totalScore = usageScore
    };

    addMatchEntry(table, entry);

    return usageScore;
}


// Example function assuming each matching comparison contributes its score directly to the total score
// without individual normalization before the final percentage calculation.
int compareASTs(ASTNode *root1, ASTNode *root2) {
    if (!root1 || !root2) {
        fprintf(stderr, "Comparison failed: One of the roots is null.\n");
        return 0;
    }

    fprintf(stderr, "Comparing node types: %d vs %d\n", root1->type, root2->type);

    MatchTable* matchTable = initializeMatchTable();
    if (!matchTable) {
        fprintf(stderr, "Failed to initialize match table.\n");
        return 0;
    }

    int totalScore = 0, totalPossibleScore = 0;

    // Collect and compare array declarations
    int numberOfDeclarations1 = 0, numberOfDeclarations2 = 0;
    ASTNode** allDeclarations1 = collectNodesOfType(root1, NodeType_ArrayDeclaration, &numberOfDeclarations1);
    ASTNode** allDeclarations2 = collectNodesOfType(root2, NodeType_ArrayDeclaration, &numberOfDeclarations2);

    if (!allDeclarations1 || !allDeclarations2) {
        fprintf(stderr, "Failed to collect nodes for comparison.\n");
    } else {
        fprintf(stderr, "Found %d array declarations in first AST, %d in second AST.\n", numberOfDeclarations1, numberOfDeclarations2);
        for (int i = 0; i < numberOfDeclarations1; i++) {
            for (int j = 0; j < numberOfDeclarations2; j++) {
                fprintf(stderr, "\nComparing array declarations: %s and %s\n", allDeclarations1[i]->name, allDeclarations2[j]->name);
                int score = compareArrayDeclarations(allDeclarations1[i], allDeclarations2[j], matchTable);
                fprintf(stderr, "\nScore for this comparison: %d\n\n", score);
                totalScore += score;
                totalPossibleScore += 100;
            }
        }
    }

    // Collect and compare array accesses
    int numberOfAccesses1 = 0, numberOfAccesses2 = 0;
    ASTNode** allAccesses1 = collectNodesOfType(root1, NodeType_ArrayAccess, &numberOfAccesses1);
    ASTNode** allAccesses2 = collectNodesOfType(root2, NodeType_ArrayAccess, &numberOfAccesses2);

    if (!allAccesses1 || !allAccesses2) {
        fprintf(stderr, "Failed to collect array access nodes for comparison.\n");
    } else {
        fprintf(stderr, "Found %d array accesses in first AST, %d in second AST.\n", numberOfAccesses1, numberOfAccesses2);
        for (int i = 0; i < numberOfAccesses1; i++) {
            for (int j = 0; j < numberOfAccesses2; j++) {
                fprintf(stderr, "\nComparing array accesses: %s and %s\n", allAccesses1[i]->name, allAccesses2[j]->name);
                int score = compareArrayAccesses(allAccesses1[i], allAccesses2[j], matchTable);
                
                totalScore += score;
                totalPossibleScore += 100;
            }
        }
    }

    // Clean up
    free(allDeclarations1);
    free(allDeclarations2);
    free(allAccesses1);
    free(allAccesses2);

    // Normalize the total score to a percentage if there was at least one comparable element
    if (totalPossibleScore > 0) {
    int base_similarity = (totalScore * 100) / totalPossibleScore;
    int adjusted_similarity = (base_similarity * 2) > 100 ? 100 : (base_similarity * 2);
    fprintf(stderr, "\nCalculated similarity: %d%%\n", adjusted_similarity);
    finalizeMatchTable(matchTable);
    return adjusted_similarity;
}


    fprintf(stderr, "No comparable elements found, returning 0.\n");
    finalizeMatchTable(matchTable);
    return 0;
}














