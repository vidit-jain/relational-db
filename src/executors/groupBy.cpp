#include "global.h"

bool lesser_than(int x, int y) { return x < y; }
bool greater_than(int x, int y) { return x > y; }
bool leq(int x, int y) { return x <= y; }
bool geq(int x, int y) { return x >= y; }
bool equal(int x, int y) { return x == y; }
bool not_equal(int x, int y) { return x != y; }
bool (*comparators[])(int, int) = {lesser_than, greater_than, leq, geq, equal, not_equal};

long long min(long long x, long long y) { return std::min(x, y); }
long long max(long long x, long long y) { return std::max(x, y); }
long long sum(long long x, long long y) { return x + y; }
long long (*accumulators[])(long long, long long) = {min, max, sum, sum};

optional<pair<AggregateFunction, string>> parseAggregateFunction(string agg) {
    int len = agg.length();
    optional<pair<AggregateFunction, string>> parsedValues;
    pair<AggregateFunction, string> val;
    if(len < 6 || agg[3] != '(' || agg[len-1] != ')') {
        parsedValues.reset();
        return parsedValues;
    }

    string func = agg.substr(0, 3);
    if (func == "MIN")
        val.first = MIN;
    else if (func == "MAX")
        val.first = MAX;
    else if (func == "SUM")
        val.first = SUM;
    else if (func == "AVG")
        val.first = AVG;
    else {
        parsedValues.reset();
        return parsedValues;
    }

    string s = agg.substr(4, len - 5);
    val.second = s;
    parsedValues = val;
    return parsedValues;
}

/**
 * @brief File contains method to process GROUP BY commands.
 *
 * syntax:
 * <new_table> <- GROUP BY <grouping_attribute> FROM <table_name> HAVING <aggregate(attribute)> <bin_op> <attribute_value> RETURN <aggregate_func(attribute)>
 *
 */
bool syntacticParseGROUPBY() {
    logger.log("syntacticParseGROUPBY");
    auto numTokens = tokenizedQuery.size();

    if(tokenizedQuery[3] != "BY" || tokenizedQuery[5] != "FROM" || tokenizedQuery[7] != "HAVING" || tokenizedQuery[11] != "RETURN") {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    parsedQuery.queryType = GROUPBY;
    parsedQuery.groupByResultantRelationName = tokenizedQuery[0];
    parsedQuery.groupByGroupingAttribute = tokenizedQuery[4];
    parsedQuery.groupByRelationName = tokenizedQuery[6];

    bool goodValue = (tokenizedQuery[10][0] == '-' || ('0' <= tokenizedQuery[10][0] && tokenizedQuery[10][0] <= '9'));
    for(int i = 1; i < tokenizedQuery[10].length(); i++)
        goodValue &= ('0' <= tokenizedQuery[10][i] && tokenizedQuery[10][i] <= '9');

    if (goodValue) {
        parsedQuery.groupByAttributeValue = stoi(tokenizedQuery[10]);
    }
    else {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    string binaryOperator = tokenizedQuery[9];
    if (binaryOperator == "<")
        parsedQuery.groupByBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.groupByBinaryOperator = GREATER_THAN;
    else if (binaryOperator == ">=" || binaryOperator == "=>")
        parsedQuery.groupByBinaryOperator = GEQ;
    else if (binaryOperator == "<=" || binaryOperator == "=<")
        parsedQuery.groupByBinaryOperator = LEQ;
    else if (binaryOperator == "==")
        parsedQuery.groupByBinaryOperator = EQUAL;
    else if (binaryOperator == "!=")
        parsedQuery.groupByBinaryOperator = NOT_EQUAL;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    auto parsedAggregateFunction = parseAggregateFunction(tokenizedQuery[8]);
    if(parsedAggregateFunction.has_value())
        parsedQuery.groupByHavingAggregateFunction = parsedAggregateFunction->first,
        parsedQuery.groupByHavingAttribute = parsedAggregateFunction->second;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    parsedAggregateFunction = parseAggregateFunction(tokenizedQuery[12]);
    if(parsedAggregateFunction.has_value())
        parsedQuery.groupByReturnAggregateFunction = parsedAggregateFunction->first,
        parsedQuery.groupByReturnAttribute = parsedAggregateFunction->second;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    return true;
}

bool semanticParseGROUPBY() {
    logger.log("semanticParseGROUPBY");

    if (tableCatalogue.isTable(parsedQuery.groupByResultantRelationName)) {
        cout << "SEMANTIC ERROR: Resultant Relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.groupByRelationName)) {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.groupByGroupingAttribute, parsedQuery.groupByRelationName)) {
        cout << "SEMANTIC ERROR: Column " << parsedQuery.groupByGroupingAttribute << " does not exist in relation" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.groupByHavingAttribute, parsedQuery.groupByRelationName)) {
        cout << "SEMANTIC ERROR: Column " << parsedQuery.groupByHavingAttribute <<" does not exist in relation" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.groupByReturnAttribute, parsedQuery.groupByRelationName)) {
        cout << "SEMANTIC ERROR: Column does not exist in relation" << endl;
        return false;
    }

    return true;
}

void executeGROUPBY() {
    logger.log("executeGROUPBY");

    string tempTableName = "Temp_GROUP_BY_" + parsedQuery.groupByRelationName;

    Table *table = tableCatalogue.getTable(parsedQuery.groupByRelationName);
    auto *tempTable = new Table(tempTableName, table);
    tableCatalogue.insertTable(tempTable);
    tempTable->sort(parsedQuery.groupByGroupingAttribute, ASC, parsedQuery.groupByRelationName);

    const BinaryOperator binaryOperator = parsedQuery.groupByBinaryOperator;
    const AggregateFunction havingAggregateFunction = parsedQuery.groupByHavingAggregateFunction;
    const AggregateFunction returnAggregateFunction = parsedQuery.groupByReturnAggregateFunction;

    const int groupingAttribute = tempTable->getColumnIndex(parsedQuery.groupByGroupingAttribute);
    const int havingAttribute = tempTable->getColumnIndex(parsedQuery.groupByHavingAttribute);
    const int returnAttribute = tempTable->getColumnIndex(parsedQuery.groupByReturnAttribute);
    string aggregateOutputColumnName;
    const int aggregateOutputColumn = 1;

    if (returnAggregateFunction == MIN)
        aggregateOutputColumnName = "MIN" + parsedQuery.groupByReturnAttribute;
    else if (returnAggregateFunction == MAX)
        aggregateOutputColumnName = "MAX" + parsedQuery.groupByReturnAttribute;
    else if (returnAggregateFunction == SUM)
        aggregateOutputColumnName = "SUM" + parsedQuery.groupByReturnAttribute;
    else if (returnAggregateFunction == AVG)
        aggregateOutputColumnName = "AVG" + parsedQuery.groupByReturnAttribute;

    auto *resultantTable = new Table(parsedQuery.groupByResultantRelationName, vector<string>{parsedQuery.groupByGroupingAttribute, aggregateOutputColumnName});

    Cursor cursor = tempTable->getCursor();
    vector<int> row = cursor.getNext();
    int prevGroup = row[groupingAttribute];
    long long accum = row[havingAttribute];
    long long ret = row[returnAttribute];
    const long long val = parsedQuery.groupByAttributeValue;
    int numRowsInGroup = 1;

    for(int i = 1; i < tempTable->rowCount; i++) {
        row = cursor.getNext();
        if (row[groupingAttribute] == prevGroup) {
            accum = accumulators[havingAggregateFunction](accum, row[havingAttribute]);
            ret = accumulators[returnAggregateFunction](ret, row[returnAttribute]);
            numRowsInGroup++;
        }
        else {
            if (havingAggregateFunction == AVG)
                accum /= numRowsInGroup;
            if(returnAggregateFunction == AVG)
                ret /= numRowsInGroup;
            vector<int> resultantRow {prevGroup, (int) ret};
            if (comparators[binaryOperator](accum, val)) {
                resultantTable->writeRow(resultantRow);
            }

            prevGroup = row[groupingAttribute];
            accum = row[havingAttribute];
            ret = row[returnAttribute];
            numRowsInGroup = 1;
        }
    }
    if (havingAggregateFunction == AVG)
        accum /= numRowsInGroup;
    if(returnAggregateFunction == AVG)
        ret /= numRowsInGroup;
    vector<int> resultantRow {prevGroup, (int) ret};
    if (comparators[binaryOperator](accum, val)) {
        resultantTable->writeRow(resultantRow);
    }

    resultantTable->blockify();
    tableCatalogue.insertTable(resultantTable);
    tableCatalogue.deleteTable(tempTableName);
}
