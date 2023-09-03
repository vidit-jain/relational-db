#include "global.h"
/**
 * @brief
 * SYNTAX: COMPUTE matrix_name
 */
bool syntacticParseCOMPUTE()
{
    logger.log("syntacticParseCOMPUTE");
    if (tokenizedQuery.size() != 2) {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = COMPUTE;
    parsedQuery.computeMatrixName = tokenizedQuery[1];
    return true;
}

bool semanticParseCOMPUTE()
{
    logger.log("semanticParseCOMPUTE");
    if (!tableCatalogue.isMatrix(parsedQuery.computeMatrixName))
    {
        cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
        return false;
    }
    return true;
}

void executeCOMPUTE()
{
    logger.log("executeCOMPUTE");
    Matrix* matrix = tableCatalogue.getMatrix(parsedQuery.computeMatrixName);
    string matrixResultName = parsedQuery.computeMatrixName + "_RESULT";
    Matrix* matrixResult = new Matrix(matrixResultName, matrix);
    matrixResult->compute(matrix->matrixName);
    tableCatalogue.insertMatrix(matrixResult);
    bufferManager.log();
    return;
}
