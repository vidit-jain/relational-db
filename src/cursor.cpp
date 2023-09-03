#include "global.h"

Cursor::Cursor(string tableName, int pageIndex, datatype d)
{
    logger.log("Cursor::Cursor");
    this->page = *bufferManager.getPage(tableName, pageIndex, d);
    this->pagePointer = 0;
    this->d = d;
    this->tableName = tableName;
    this->pageIndex = pageIndex;
}

/**
 * @brief This function reads the next row from the page. The index of the
 * current row read from the page is indicated by the pagePointer(points to row
 * in page the cursor is pointing to).
 *
 * @return vector<int> 
 */
vector<int> Cursor::getNext()
{
    logger.log("Cursor::geNext");
    vector<int> result = this->page.getRow(this->pagePointer);
    this->pagePointer++;
    if(result.empty()){
        tableCatalogue.getTable(this->tableName)->getNextPage(this);
        if(!this->pagePointer){
            result = this->page.getRow(this->pagePointer);
            this->pagePointer++;
        }
    }
    return result;
}

/**
 * @brief This function returns the value at the location in the page
 * specified by the parameters
 * @param row
 * @param col
 */
int Cursor::getCell(int row, int col) {
    logger.log("Cursor::getCell");
    return this->page.getCell(row, col);
}

/**
 * @brief Function that loads Page indicated by pageIndex. Now the cursor starts
 * reading from the new page.
 *
 * @param pageIndex 
 */
void Cursor::nextPage(int pageIndex)
{
    logger.log("Cursor::nextPage");
    this->page = *bufferManager.getPage(this->tableName, pageIndex, this->d);
    this->pageIndex = pageIndex;
    this->pagePointer = 0;
}