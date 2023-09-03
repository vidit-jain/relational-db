#include "global.h"
/**
 * @brief
 * SYNTAX: CHECKSYMMETRY matrix_name
 */
bool syntacticParseSYMMETRY()
{
    logger.log("syntacticParseSYMMETRY");
    if (tokenizedQuery.size() != 2) {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = SYMMETRY;
    parsedQuery.symmetryMatrixName = tokenizedQuery[1];
    return true;
}

bool semanticParseSYMMETRY()
{
    logger.log("semanticParseSYMMETRY");
    if (!tableCatalogue.isMatrix(parsedQuery.symmetryMatrixName))
    {
        cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
        return false;
    }
    return true;
}

void executeSYMMETRY()
{
    logger.log("executeSYMMETRY");
    Matrix* matrix = tableCatalogue.getMatrix(parsedQuery.symmetryMatrixName);
    bool symmetry = matrix->symmetry();
    if (symmetry) cout << "TRUE" << endl;
    else cout << "FALSE" << endl;

    blockStats.log();
    return;
}
