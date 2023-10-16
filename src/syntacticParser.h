#include "tableCatalogue.h"

using namespace std;

enum QueryType
{
    CLEAR,
    COMPUTE,
    CROSS,
    DISTINCT,
    EXPORT,
    GROUPBY,
    INDEX,
    JOIN,
    LIST,
    LOAD,
    PRINT,
    PROJECTION,
    RENAME,
    SELECTION,
    SORT,
    SOURCE,
    SYMMETRY,
    TRANSPOSE,
    ORDERBY,
    UNDETERMINED
};

enum AggregateFunction
{
    MIN = 0,
    MAX = 1,
    SUM = 2,
    AVG = 3,
    NO_AGG_FUNC
};

enum BinaryOperator
{
    LESS_THAN = 0,
    GREATER_THAN = 1,
    LEQ = 2,
    GEQ = 3,
    EQUAL = 4,
    NOT_EQUAL = 5,
    NO_BINOP_CLAUSE
};

enum SortingStrategy
{
    ASC = 1,
    DESC = -1,
    NO_SORT_CLAUSE = 0
};

enum SelectType
{
    COLUMN,
    INT_LITERAL,
    NO_SELECT_CLAUSE
};

class ParsedQuery
{

public:
    QueryType queryType = UNDETERMINED;
    string queryData = "";

    string clearRelationName = "";

    string crossResultRelationName = "";
    string crossFirstRelationName = "";
    string crossSecondRelationName = "";

    string distinctResultRelationName = "";
    string distinctRelationName = "";

    string exportRelationName = "";

    string groupByResultantRelationName = "";
    string groupByGroupingAttribute = "";
    string groupByRelationName = "";
    string groupByHavingAttribute = "";
    AggregateFunction groupByHavingAggregateFunction = NO_AGG_FUNC;
    BinaryOperator groupByBinaryOperator = NO_BINOP_CLAUSE;
    int groupByAttributeValue = 0;
    AggregateFunction groupByReturnAggregateFunction = NO_AGG_FUNC;
    string groupByReturnAttribute = "";

    IndexingStrategy indexingStrategy = NOTHING;
    string indexColumnName = "";
    string indexRelationName = "";

    BinaryOperator joinBinaryOperator = NO_BINOP_CLAUSE;
    string joinResultRelationName = "";
    string joinFirstRelationName = "";
    string joinSecondRelationName = "";
    string joinFirstColumnName = "";
    string joinSecondColumnName = "";

    string loadRelationName = "";

    string printRelationName = "";

    string projectionResultRelationName = "";
    vector<string> projectionColumnList;
    string projectionRelationName = "";

    string renameFromColumnName = "";
    string renameToColumnName = "";
    string renameRelationName = "";

    SelectType selectType = NO_SELECT_CLAUSE;
    BinaryOperator selectionBinaryOperator = NO_BINOP_CLAUSE;
    string selectionResultRelationName = "";
    string selectionRelationName = "";
    string selectionFirstColumnName = "";
    string selectionSecondColumnName = "";
    int selectionIntLiteral = 0;

    vector<SortingStrategy> sortingStrategies;
    vector<string> sortColumnNames;
    string sortRelationName = "";

    string orderByRelationName = "";
    string orderByResultantRelationName = "";
    string orderByColumnName = "";
    SortingStrategy orderByMultiplier = NO_SORT_CLAUSE;

    string sourceFileName = "";
    string loadMatrixName = "";
    string printMatrixName = "";
    string transposeMatrixName = "";
    string symmetryMatrixName = "";
    string computeMatrixName = "";
    string exportMatrixName = "";
    string renameFromMatrixName = "";
    string renameToMatrixName = "";

    ParsedQuery();
    void clear();
};

bool syntacticParse();
bool syntacticParseCLEAR();
bool syntacticParseCROSS();
bool syntacticParseDISTINCT();
bool syntacticParseEXPORT();
bool syntacticParseGROUPBY();
bool syntacticParseINDEX();
bool syntacticParseJOIN();
bool syntacticParseLIST();
bool syntacticParseLOAD();
bool syntacticParsePRINT();
bool syntacticParsePROJECTION();
bool syntacticParseRENAME();
bool syntacticParseSELECTION();
bool syntacticParseSORT();
bool syntacticParseSOURCE();
bool syntacticParseSYMMETRY();
bool syntacticParseTRANSPOSE();
bool syntacticParseCOMPUTE();
bool syntacticParseORDERBY();

bool isFileExists(string tableName);
bool isQueryFile(string fileName);
