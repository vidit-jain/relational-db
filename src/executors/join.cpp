#include "global.h"
/**
 * @brief 
 * SYNTAX: R <- JOIN relation_name1, relation_name2 ON column_name1 bin_op column_name2
 */
extern bool (*comparators[])(int, int);

bool syntacticParseJOIN()
{
    logger.log("syntacticParseJOIN");
    if (tokenizedQuery.size() != 9 || tokenizedQuery[5] != "ON")
    {
        cout << "SYNTAX ERROR" << endl;
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
    logger.log("executeJOIN");

    if (parsedQuery.joinBinaryOperator < 4) {
        // Table 1 doesn't need to be sorted, no advantage achieved
        auto* table1 = tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
        // Table 2 is sorted according to the binary operator we use
        auto* table2 = new Table("Temp_JOIN_" + parsedQuery.joinSecondRelationName, tableCatalogue.getTable(parsedQuery.joinSecondRelationName));
        tableCatalogue.insertTable(table2);
        SortingStrategy sortingStrategy = (parsedQuery.joinBinaryOperator % 2) ? ASC : DESC;
        table2->sort(parsedQuery.joinSecondColumnName, sortingStrategy, parsedQuery.joinSecondRelationName);

        // Get the corresponding concerned column indices in each of the relations
        int col1 = table1->getColumnIndex(parsedQuery.joinFirstColumnName), col2 = table2->getColumnIndex(parsedQuery.joinSecondColumnName);
        auto columns = table1->columns;
        columns.insert(columns.end(), table2->columns.begin(), table2->columns.end());

        auto* resultantTable = new Table(parsedQuery.joinResultRelationName, columns);
        auto cursor1 = table1->getCursor();
        auto row1 = cursor1.getNext();

        // Find the appropriate comparator function
        bool (*f) (int,int) = *comparators[parsedQuery.joinBinaryOperator];
        while (!row1.empty()) {
            auto cursor2 = table2->getCursor();
            vector<int> row2 = cursor2.getNext(), result;
            while (!row2.empty() && f(row1[col1], row2[col2])) {
                result = row1;
                result.insert(result.end(), row2.begin(), row2.end());
                resultantTable->writeRow<int>(result);
                row2 = cursor2.getNext();
            }
            row1 = cursor1.getNext();
        }
        resultantTable->blockify();
        tableCatalogue.insertTable(resultantTable);
        tableCatalogue.deleteTable(table2->tableName);
    }
    else {
        auto* table1 = new Table("Temp_JOIN_" + parsedQuery.joinFirstRelationName, tableCatalogue.getTable(parsedQuery.joinFirstRelationName));
        auto* table2 = new Table("Temp_JOIN_" + parsedQuery.joinSecondRelationName, tableCatalogue.getTable(parsedQuery.joinSecondRelationName));
        tableCatalogue.insertTable(table1);
        tableCatalogue.insertTable(table2);
        table1->sort(parsedQuery.joinFirstColumnName, ASC, parsedQuery.joinFirstRelationName);
        table2->sort(parsedQuery.joinSecondColumnName, ASC, parsedQuery.joinSecondRelationName);

        int col1 = table1->getColumnIndex(parsedQuery.joinFirstColumnName), col2 = table2->getColumnIndex(parsedQuery.joinSecondColumnName);
        auto columns = table1->columns;
        columns.insert(columns.end(), table2->columns.begin(), table2->columns.end());
        auto* resultantTable = new Table(parsedQuery.joinResultRelationName, columns);

        auto cursor1 = table1->getCursor(), cursor2 = table2->getCursor();
        auto row1 = cursor1.getNext(), row2 = cursor2.getNext();
        if (parsedQuery.joinBinaryOperator == EQUAL) {
            // Two pointer approach
            while (!row1.empty() && !row2.empty()) {
                if (row1[col1] < row2[col2]) row1 = cursor1.getNext();
                else if (row1[col1] > row2[col2]) row2 = cursor2.getNext();
                else {
                    // Different initializations of r1, r2 to avoid matching row1, row2 twice
                    auto c1 = cursor1, c2 = cursor2;
                    auto r1 = c1.getNext(), r2 = row2;
                    vector<int> result;
                    while (!r2.empty() && row1[col1] == r2[col2]) {
                        result = row1;
                        result.insert(result.end(), r2.begin(), r2.end());
                        resultantTable->writeRow<int>(result);
                        r2 = c2.getNext();
                    }
                    while (!r1.empty() && r1[col1] == row2[col2]) {
                        result = r1;
                        result.insert(result.end(), row2.begin(), row2.end());
                        resultantTable->writeRow<int>(result);
                        r1 = c1.getNext();
                    }
                    row1 = cursor1.getNext();
                    row2 = cursor2.getNext();
                }
            }
        }
        else {
            while (!row1.empty()) {
                // row2 corresponds to the first row greater than row1 (may be empty)
                if (!row2.empty() && row1[col1] >= row2[col2]) row2 = cursor2.getNext();
                else {
                    auto a = table2->getCursor();
                    vector<int> b = a.getNext(), result;
                    auto iterate = [&] (Cursor& c, bool (*f)(int, int)) {
                        while (!b.empty() && f(b[col2], row1[col1])) {
                            result = row1;
                            result.insert(result.end(), b.begin(), b.end());
                            resultantTable->writeRow<int>(result);
                            b = c.getNext();
                        }
                    };
                    // Start from the beginning of table2, keep going while row2 < row1
                    iterate(a, *comparators[0]);
                    b = row2;
                    // Start from position of row2, keep going till empty
                    Cursor cc2 = cursor2;
                    iterate(cc2, *comparators[1]);
                    row1 = cursor1.getNext();
                }
            }
        }
        resultantTable->blockify();
        tableCatalogue.insertTable(resultantTable);
        tableCatalogue.deleteTable(table2->tableName);
        tableCatalogue.deleteTable(table1->tableName);

    }
    return;
}