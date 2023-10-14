#include "global.h"
/**
 * @brief 
 * SYNTAX: R <- JOIN relation_name1, relation_name2 ON column_name1 bin_op column_name2
 */
bool syntacticParseJOIN()
{
    logger.log("syntacticParseJOIN");
    if (tokenizedQuery.size() != 9 || tokenizedQuery[5] != "ON")
    {
        cout << "SYNTAC ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = JOIN;
    parsedQuery.joinResultRelationName = tokenizedQuery[0];
    parsedQuery.joinFirstRelationName = tokenizedQuery[3];
    parsedQuery.joinSecondRelationName = tokenizedQuery[4];
    parsedQuery.joinFirstColumnName = tokenizedQuery[6];
    parsedQuery.joinSecondColumnName = tokenizedQuery[8];

    string binaryOperator = tokenizedQuery[7];
    if (binaryOperator == "<")
        parsedQuery.joinBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.joinBinaryOperator = GREATER_THAN;
    else if (binaryOperator == ">=" || binaryOperator == "=>")
        parsedQuery.joinBinaryOperator = GEQ;
    else if (binaryOperator == "<=" || binaryOperator == "=<")
        parsedQuery.joinBinaryOperator = LEQ;
    else if (binaryOperator == "==")
        parsedQuery.joinBinaryOperator = EQUAL;
    else if (binaryOperator == "!=")
        parsedQuery.joinBinaryOperator = NOT_EQUAL;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseJOIN()
{
    logger.log("semanticParseJOIN");

    if (tableCatalogue.isTable(parsedQuery.joinResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.joinFirstRelationName) || !tableCatalogue.isTable(parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.joinFirstColumnName, parsedQuery.joinFirstRelationName) || !tableCatalogue.isColumnFromTable(parsedQuery.joinSecondColumnName, parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    return true;
}

void executeJOIN()
{
    auto *table1 = new Table("Temp_JOIN_" + parsedQuery.joinFirstRelationName, tableCatalogue.getTable(parsedQuery.joinFirstRelationName));
    auto *table2 = new Table("Temp_JOIN_" + parsedQuery.joinSecondRelationName, tableCatalogue.getTable(parsedQuery.joinSecondRelationName));
    tableCatalogue.insertTable(table1);
    tableCatalogue.insertTable(table2);
    table1->sort(parsedQuery.joinFirstColumnName, ASC, parsedQuery.joinFirstRelationName);
    table2->sort(parsedQuery.joinSecondColumnName, ASC, parsedQuery.joinSecondRelationName);

    int col1 = table1->getColumnIndex(parsedQuery.joinFirstColumnName);
    int col2 = table2->getColumnIndex(parsedQuery.joinSecondColumnName);

    auto columns = table1->columns;
    columns.insert(columns.end(), table2->columns.begin(), table2->columns.end());

    auto* resultantTable = new Table(parsedQuery.joinResultRelationName, columns);
    auto cursor1 = table1->getCursor();

    auto row1 = cursor1.getNext();

    while (!row1.empty()) {
        auto cursor2 = table2->getCursor();
        vector<int> row2 = cursor2.getNext(), result;
        while (!row2.empty() && row1[col1] > row2[col2]) {
            result = row1;
            result.insert(result.end(), row2.begin(), row2.end());
            resultantTable->writeRow<int>(result);
            row2 = cursor2.getNext();
        }
        row1 = cursor1.getNext();
    }
    resultantTable->blockify();
    tableCatalogue.insertTable(resultantTable);
    tableCatalogue.deleteTable(table1->tableName);
    tableCatalogue.deleteTable(table2->tableName);
    logger.log("executeJOIN");
    return;
}