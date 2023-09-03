#include "global.h"
/**
 * @brief 
 * SYNTAX: RENAME column_name TO column_name FROM relation_name
 * SYNTAX: RENAME MATRIX oldMatrixName newMatrixName
 */
bool syntacticParseRENAME()
{
    logger.log("syntacticParseRENAME");
    if (tokenizedQuery.size() == 4 && tokenizedQuery[1] == "MATRIX") {
        parsedQuery.queryType = RENAME;
        parsedQuery.renameFromMatrixName = tokenizedQuery[2];
        parsedQuery.renameToMatrixName = tokenizedQuery[3];
    }
    else if (tokenizedQuery.size() == 6 && tokenizedQuery[2] == "TO" && tokenizedQuery[4] == "FROM")
    {
        parsedQuery.queryType = RENAME;
        parsedQuery.renameFromColumnName = tokenizedQuery[1];
        parsedQuery.renameToColumnName = tokenizedQuery[3];
        parsedQuery.renameRelationName = tokenizedQuery[5];
    }
    else {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseRENAME()
{
    logger.log("semanticParseRENAME");
    if (!parsedQuery.renameRelationName.empty()) {
        if (!tableCatalogue.isTable(parsedQuery.renameRelationName))
        {
            cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
            return false;
        }

        if (!tableCatalogue.isColumnFromTable(parsedQuery.renameFromColumnName, parsedQuery.renameRelationName))
        {
            cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
            return false;
        }

        if (tableCatalogue.isColumnFromTable(parsedQuery.renameToColumnName, parsedQuery.renameRelationName))
        {
            cout << "SEMANTIC ERROR: Column with name already exists" << endl;
            return false;
        }
        return true;
    }
    else {
        if (!tableCatalogue.isMatrix(parsedQuery.renameFromMatrixName)) {
            cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
            return false;
        }
        if (tableCatalogue.isMatrix(parsedQuery.renameToMatrixName) || \
        tableCatalogue.isTable(parsedQuery.renameToMatrixName))
        {
            cout << "SEMANTIC ERROR: Matrix or Table with name already exists" << endl;
            return false;
        }
        return true;
    }
}

void executeRENAME()
{
    logger.log("executeRENAME");
    if (!parsedQuery.renameRelationName.empty()) {
        Table* table = tableCatalogue.getTable(parsedQuery.renameRelationName);
        table->renameColumn(parsedQuery.renameFromColumnName, parsedQuery.renameToColumnName);
    }
    else {
        tableCatalogue.renameMatrix(parsedQuery.renameFromMatrixName, parsedQuery.renameToMatrixName);
        blockStats.log();
    }
    return;
}