#include "global.h"

/**
 * @brief File contains method to process ORDER BY commands.
 *
 * syntax:
 * <new_table> <- ORDER BY <attribute> ASC|DESC ON <table_name>
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
    parsedQuery.orderByColumnName = tokenizedQuery[4];
    parsedQuery.orderByMultiplier = (tokenizedQuery[5] == "ASC") ? ASC : DESC;

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
    if (!tableCatalogue.isColumnFromTable(parsedQuery.orderByColumnName, parsedQuery.orderByRelationName)) {
        cout << "SEMANTIC ERROR: Column does not exist in relation" << endl;
        return false;
    }
    return true;
}

void executeORDERBY() {
    logger.log("executeORDERBY");

    Table *table = tableCatalogue.getTable(parsedQuery.orderByRelationName);
    auto *resultantTable = new Table(parsedQuery.orderByResultantRelationName, table);
    resultantTable->sort(parsedQuery.orderByColumnName, parsedQuery.orderByMultiplier, parsedQuery.orderByRelationName);
    tableCatalogue.insertTable(resultantTable);
}
