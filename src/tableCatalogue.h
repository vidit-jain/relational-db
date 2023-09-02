#include "table.h"
#include "matrix.h"

/**
 * @brief The TableCatalogue acts like an index of tables existing in the
 * system. Everytime a table is added(removed) to(from) the system, it needs to
 * be added(removed) to(from) the tableCatalogue. 
 *
 */
class TableCatalogue
{

    unordered_map<string, Table*> tables;
    unordered_map<string, Matrix*> matrices;

public:
    TableCatalogue() {}
    void insertTable(Table* table);
    void deleteTable(string tableName);
    Table* getTable(string tableName);
    Matrix* getMatrix(string matrixName);
    bool isTable(string tableName);
    bool isMatrix(string matrixName);
    bool isLoaded(string dataName);
    bool isColumnFromTable(string columnName, string tableName);
    void print();
    ~TableCatalogue();
};