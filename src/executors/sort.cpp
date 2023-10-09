#include"global.h"

/**
 * @brief File contains method to process SORT commands.
 * 
 * syntax:
 * SORT <table_name> BY <column_name1, column_name2,..., column_namek> IN <ASC|DESC, ASC|DESC,..., ASC|DESC>
 * R <- SORT relation_name BY column_name IN sorting_order
 * 
 * sorting_order = ASC | DESC 
 */
bool syntacticParseSORT() {
    logger.log("syntacticParseSORT");
    auto numTokens = tokenizedQuery.size(), numSortColumns = (numTokens - 4) / 2;
    auto IN_idx = numSortColumns + 3;
    if (tokenizedQuery[2] != "BY" || numTokens < 6 || (numTokens - 4) % 2 || tokenizedQuery[IN_idx] != "IN") {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    for (auto i = IN_idx + 1; i < numTokens; i++)
        if (tokenizedQuery[i] != "ASC" && tokenizedQuery[i] != "DESC") {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
    parsedQuery.queryType = SORT;
    parsedQuery.sortRelationName = tokenizedQuery[1];
    for (auto i = 0; i < numSortColumns; i++) {
        parsedQuery.sortingStrategies.push_back((tokenizedQuery[i] == "ASC") ? ASC : DESC);
        parsedQuery.sortColumnNames.push_back(tokenizedQuery[IN_idx + 1 + i]);
    }
    return true;
}

bool semanticParseSORT() {
    logger.log("semanticParseSORT");

    if (tableCatalogue.isTable(parsedQuery.sortResultRelationName)) {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.sortRelationName)) {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.sortColumnName, parsedQuery.sortRelationName)) {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }

    return true;
}

void executeSORT() {
    logger.log("executeSORT");
    return;
}