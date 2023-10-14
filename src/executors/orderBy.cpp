#include "global.h"

/**
 * @brief File contains method to process SORT commands.
 *
 * syntax:
 * SORT <table_name> BY <column_name1, column_name2,..., column_namek> IN <ASC|DESC, ASC|DESC,..., ASC|DESC>
 *
 */
bool syntacticParseORDERBY() {
    logger.log("syntacticParseORDERBY");
    auto numTokens = tokenizedQuery.size();
    if (tokenizedQuery[3] != "BY" || tokenizedQuery[6] != "ON" || numTokens != 8 || (tokenizedQuery[5] != "ASC" && tokenizedQuery[5] != "DESC")) {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = ORDERBY;
    parsedQuery.orderByRelationName = tokenizedQuery[7];
    parsedQuery.orderByResultantRelationName = tokenizedQuery[0];

    parsedQuery.sortingStrategies.push_back((tokenizedQuery[5] == "ASC") ? ASC : DESC);
    parsedQuery.sortColumnNames.push_back(tokenizedQuery[4]);
    return true;
}

bool semanticParseORDERBY() {
    logger.log("semanticParseORDERBY");

    if (!tableCatalogue.isTable(parsedQuery.orderByRelationName)) {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (tableCatalogue.isTable(parsedQuery.orderByResultantRelationName)) {
        cout << "SEMANTIC ERROR: Resultant Relation already exists" << endl;
        return false;
    }
    if (!tableCatalogue.isColumnFromTable(parsedQuery.sortColumnNames[0], parsedQuery.orderByRelationName)) {
        cout << "SEMANTIC ERROR: Column does not exist in relation" << endl;
        return false;
    }
    return true;
}

void executeORDERBY() {
    logger.log("executeORDERBY");

    Table *table = tableCatalogue.getTable(parsedQuery.orderByRelationName);
    auto *resultantTable = new Table(parsedQuery.orderByResultantRelationName, table);
    vector<int> colMultipliers = {parsedQuery.sortingStrategies[0]};
    resultantTable->sort(parsedQuery.sortColumnNames, colMultipliers, parsedQuery.orderByRelationName);
    tableCatalogue.insertTable(resultantTable);
}
