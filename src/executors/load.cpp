#include "global.h"
/**
 * @brief 
 * SYNTAX: LOAD relation_name
 * SYNTAX: LOAD MATRIX matrix_name
 */
bool syntacticParseLOAD()
{
    logger.log("syntacticParseLOAD");
    if (tokenizedQuery.size() == 2) {
        parsedQuery.queryType = LOAD;
        parsedQuery.loadRelationName = tokenizedQuery[1];
    }
    else if (tokenizedQuery.size() == 3 && tokenizedQuery[1] == "MATRIX") {
        parsedQuery.queryType = LOAD;
        parsedQuery.loadMatrixName = tokenizedQuery[2];
    }
    else {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseLOAD()
{
    logger.log("semanticParseLOAD");
    if (!parsedQuery.loadRelationName.empty()) {
        if (tableCatalogue.isTable(parsedQuery.loadRelationName)) {
            cout << "SEMANTIC ERROR: Relation already exists" << endl;
            return false;
        }
        if (!isFileExists(parsedQuery.loadRelationName))
        {
            cout << "SEMANTIC ERROR: Data file doesn't exist" << endl;
            return false;
        }
        return true;
    }
    else {
        if (tableCatalogue.isMatrix(parsedQuery.loadMatrixName)) {
            cout << "SEMANTIC ERROR: Matrix already exists" << endl;
            return false;
        }
        if (!isFileExists(parsedQuery.loadMatrixName))
        {
            cout << "SEMANTIC ERROR: Data file doesn't exist" << endl;
            return false;
        }
        return true;
    }

}

void executeLOAD()
{
    logger.log("executeLOAD");
    if (!parsedQuery.loadRelationName.empty()) {
        Table *table = new Table(parsedQuery.loadRelationName);
        if (table->load())
        {
            tableCatalogue.insertTable(table);
            cout << "Loaded Table. Column Count: " << table->columnCount << " Row Count: " << table->rowCount << endl;
        }
    }
    else {
        Matrix *matrix = new Matrix(parsedQuery.loadMatrixName);
        if (matrix->load())
        {
            tableCatalogue.insertMatrix(matrix);
            cout << "Loaded Table. Dimension: " << matrix->dimension << endl;
        }
        bufferManager.log();
    }
    return;
}