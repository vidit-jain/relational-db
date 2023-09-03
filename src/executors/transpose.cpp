#include "global.h"
/**
 * @brief
 * SYNTAX: TRANSPOSE MATRIX matrix_name
 */
bool syntacticParseTRANSPOSE()
{
    logger.log("syntacticParseTRANSPOSE");
    if (tokenizedQuery.size() != 3 || tokenizedQuery[1] != "MATRIX") {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = TRANSPOSE;
    parsedQuery.transposeMatrixName = tokenizedQuery[2];
    return true;
}

bool semanticParseTRANSPOSE()
{
    logger.log("semanticParseTRANSPOSE");
    if (!tableCatalogue.isMatrix(parsedQuery.transposeMatrixName))
    {
        cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
        return false;
    }
    return true;
}

void executeTRANSPOSE()
{
    logger.log("executeTRANSPOSE");
    Matrix* matrix = tableCatalogue.getMatrix(parsedQuery.transposeMatrixName);
    matrix->transpose();
    bufferManager.log();
    return;
}
