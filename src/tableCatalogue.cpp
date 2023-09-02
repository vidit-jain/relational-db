#include "global.h"

void TableCatalogue::insertTable(Table* table)
{
    logger.log("TableCatalogue::~insertTable"); 
    this->tables[table->tableName] = table;
}
void TableCatalogue::deleteTable(string tableName)
{
    logger.log("TableCatalogue::deleteTable"); 
    this->tables[tableName]->unload();
    delete this->tables[tableName];
    this->tables.erase(tableName);
}
Table* TableCatalogue::getTable(string tableName)
{
    logger.log("TableCatalogue::getTable"); 
    Table *table = this->tables[tableName];
    return table;
}
Matrix* TableCatalogue::getMatrix(string matrixName)
{
    logger.log("MatrixCatalogue::getMatrix");
    Matrix *matrix = this->matrices[matrixName];
    return matrix;
}
bool TableCatalogue::isTable(string tableName)
{
    logger.log("TableCatalogue::isTable"); 
    if (this->tables.count(tableName))
        return true;
    return false;
}

bool TableCatalogue::isMatrix(string matrixName)
{
    logger.log("TableCatalogue::isMatrix");
    if (this->matrices.count(matrixName))
        return true;
    return false;
}

bool TableCatalogue::isLoaded(string dataName) {
    logger.log("TableCatalogue::isLoaded");
    return isTable(dataName) || isMatrix(dataName);
}

bool TableCatalogue::isColumnFromTable(string columnName, string tableName)
{
    logger.log("TableCatalogue::isColumnFromTable"); 
    if (this->isTable(tableName))
    {
        Table* table = this->getTable(tableName);
        if (table->isColumn(columnName))
            return true;
    }
    return false;
}

void TableCatalogue::print()
{
    logger.log("TableCatalogue::print"); 
    cout << "\nRELATIONS" << endl;

    int rowCount = 0;
    for (auto rel : this->tables)
    {
        cout << rel.first << endl;
        rowCount++;
    }
    printRowCount(rowCount);
}

TableCatalogue::~TableCatalogue(){
    logger.log("TableCatalogue::~TableCatalogue"); 
    for(auto table: this->tables){
        table.second->unload();
        delete table.second;
    }
}
