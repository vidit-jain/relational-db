#include "global.h"
/**
 * @brief 
 * SYNTAX: LIST TABLES/MATRICES
 */
bool syntacticParseLIST()
{
    logger.log("syntacticParseLIST");
    if (tokenizedQuery.size() != 2 || (tokenizedQuery[1] != "TABLES" && tokenizedQuery[1] != "MATRICES"))
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = LIST;
    parsedQuery.queryData = tokenizedQuery[1];
    return true;
}

bool semanticParseLIST()
{
    logger.log("semanticParseLIST");
    return true;
}

void executeLIST()
{
    logger.log("executeLIST");
    tableCatalogue.print(parsedQuery.queryData);
}