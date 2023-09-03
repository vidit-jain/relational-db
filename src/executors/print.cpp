#include "global.h"
/**
 * @brief 
 * SYNTAX: PRINT relation_name
 * SYNTAX: PRINT MATRIX matrix_name
 */
bool syntacticParsePRINT()
{
    logger.log("syntacticParsePRINT");
    if (tokenizedQuery.size() == 2) {
        parsedQuery.queryType = PRINT;
        parsedQuery.printRelationName = tokenizedQuery[1];
    }
    else if (tokenizedQuery.size() == 3 && tokenizedQuery[1] == "MATRIX") {
        parsedQuery.queryType = PRINT;
        parsedQuery.printMatrixName = tokenizedQuery[2];
    }
    else {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParsePRINT()
{
    logger.log("semanticParsePRINT");
    if (!parsedQuery.printRelationName.empty()) {
        if (!tableCatalogue.isTable(parsedQuery.printRelationName))
        {
            cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
            return false;
        }
        return true;
    }
    else {
        if (!tableCatalogue.isMatrix(parsedQuery.printMatrixName)) {
            cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
            return false;
        }
        return true;
    }
}

void executePRINT()
{
    logger.log("executePRINT");
    if (!parsedQuery.printRelationName.empty()) {
        Table* table = tableCatalogue.getTable(parsedQuery.printRelationName);
        table->print();
    }
    else {
        Matrix* matrix = tableCatalogue.getMatrix(parsedQuery.printMatrixName);
        matrix->print();
        blockStats.log();
    }
    return;
}
