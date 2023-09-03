#include "global.h"

/**
 * @brief 
 * SYNTAX: EXPORT <relation_name> 
 */

bool syntacticParseEXPORT()
{
    logger.log("syntacticParseEXPORT");
    if (tokenizedQuery.size() == 2) {
        parsedQuery.queryType = EXPORT;
        parsedQuery.exportRelationName = tokenizedQuery[1];
    }
    else if (tokenizedQuery.size() == 3 && tokenizedQuery[1] == "MATRIX") {
        parsedQuery.queryType = EXPORT;
        parsedQuery.exportMatrixName = tokenizedQuery[2];
    }
    else {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseEXPORT()
{
    logger.log("semanticParseEXPORT");
    //Table should exist
    if (!parsedQuery.exportRelationName.empty()) {
        if (tableCatalogue.isTable(parsedQuery.exportRelationName))
            return true;
        cout << "SEMANTIC ERROR: No such relation exists" << endl;
        return false;
    }
    else {
        if (tableCatalogue.isMatrix(parsedQuery.exportMatrixName))
            return true;
        cout << "SEMANTIC ERROR: No such matrix exists" << endl;
        return false;
    }
}

void executeEXPORT()
{
    logger.log("executeEXPORT");
    if (!parsedQuery.exportRelationName.empty()) {
        Table* table = tableCatalogue.getTable(parsedQuery.exportRelationName);
        table->makePermanent();
    }
    else {
        Matrix* matrix = tableCatalogue.getMatrix(parsedQuery.exportMatrixName);
        matrix->makePermanent();
        blockStats.log();
    }
    return;
}