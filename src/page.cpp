#include "global.h"
/**
 * @brief Construct a new Page object. Never used as part of the code
 *
 */
Page::Page()
{
    this->pageName = "";
    this->tableName = "";
    this->pageIndex = -1;
    this->rowCount = 0;
    this->columnCount = 0;
    this->dirty = 0;
    this->rows.clear();
}

/**
 * @brief Construct a new Page:: Page object given the table name and page
 * index. When tables are loaded they are broken up into blocks of BLOCK_SIZE
 * and each block is stored in a different file named
 * "<tablename>_Page<pageindex>". For example, If the Page being loaded is of
 * table "R" and the pageIndex is 2 then the file name is "R_Page2". The page
 * loads the rows (or tuples) into a vector of rows (where each row is a vector
 * of integers).
 *
 * @param tableName 
 * @param pageIndex 
 */
Page::Page(string tableName, int pageIndex, datatype d)
{
    logger.log("Page::Page");
    this->dirty = 0;
    this->tableName = tableName;
    this->pageIndex = pageIndex;
    this->pageName = "../data/temp/" + this->tableName + "_Page" + to_string(pageIndex);
    uint maxRowCount;
    if (d == TABLE) {
        Table table = *tableCatalogue.getTable(tableName);
        this->columnCount = table.columnCount;
        this->rowCount = table.rowsPerBlockCount[pageIndex];
        maxRowCount = table.maxRowsPerBlock;
    }
    else {
        Matrix matrix = *tableCatalogue.getMatrix(tableName);
        tie(this->rowCount, this->columnCount) = matrix.dimsPerBlock[pageIndex];
        maxRowCount = this->rowCount;
    }
    vector<int> row(columnCount, 0);
    this->rows.assign(maxRowCount, row);
    ifstream fin(pageName, ios::in);
    int number;
    for (uint rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        for (int columnCounter = 0; columnCounter < columnCount; columnCounter++)
        {
            fin >> number;
            this->rows[rowCounter][columnCounter] = number;
        }
    }
    fin.close();
}

/**
 * @brief Get row from page indexed by rowIndex
 * 
 * @param rowIndex 
 * @return vector<int> 
 */
vector<int> Page::getRow(int rowIndex)
{
    logger.log("Page::getRow");
    vector<int> result;
    result.clear();
    if (rowIndex >= this->rowCount)
        return result;
    return this->rows[rowIndex];
}

/**
 * @param row
 * @param col
 * @return Returns value at cell specified by parameters
 */
int Page::getCell(int row, int col) {
    logger.log("Page::getCell");
    assert(row < this->rowCount && col < this->columnCount);
    return this->rows[row][col];
}

/**
 * @brief Sets tableName of the page to a new table name, recreates the new pageName for the page.
 * @param newName
 */
void Page::setPageName(string newName) {
    this->tableName = newName;
    this->pageName = "../data/temp/"+this->tableName + "_Page" + to_string(this->pageIndex);
}

Page::Page(string tableName, int pageIndex, vector<vector<int>> rows, int rowCount, int colCount)
{
    logger.log("Page::Page");
    this->pageIndex = pageIndex;
    this->rows = rows;
    this->rowCount = rowCount;
    this->columnCount = colCount;
    setPageName(tableName);
}

/**
 * @brief writes current page contents to file.
 * 
 */
void Page::writePage()
{
    logger.log("Page::writePage");
    ofstream fout(this->pageName, ios::trunc);
    for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
        {
            if (columnCounter != 0)
                fout << " ";
            fout << this->rows[rowCounter][columnCounter];
        }
        fout << endl;
    }
    fout.close();
    this->dirty = 0;
}

/**
 * @brief transposes the submatrix stored in the page
 */
void Page::transpose(Page* p) {
    for (int i = 0; i < this->rowCount; i++) {
        for (int j = 0; j < this->columnCount; j++) {
            swap(this->rows[i][j], p->rows[j][i]);
        }
    }
    this->dirty = 1, p->dirty = 1;
}

/**
 * @brief Flips submatrix in place
 */
void Page::transpose() {
    for (int i = 0; i < this->rowCount; i++) {
        for (int j = i + 1; j < this->columnCount; j++) {
            swap(this->rows[i][j], this->rows[j][i]);
        }
    }
    this->dirty = 1;
}

/**
 * @brief Should only be used between corresponding blocks in the matrix. Swaps the values between them to
 * perform a transpose
 */
void Page::subtractTranspose(Page* p) {
    for (int i = 0; i < this->rowCount; i++) {
        for (int j = 0; j < this->columnCount; j++) {
            this->rows[i][j] -= p->rows[j][i];
            p->rows[j][i] = -this->rows[i][j];
        }
    }
    this->dirty = 1, p->dirty = 1;
}

/**
 * @brief Should only be used between corresponding blocks int he matrix. Performs a transpose and subtracts
 * the value to get the resultant.
 */
void Page::subtractTranspose() {
    for (int i = 0; i < this->rowCount; i++) {
        for (int j = i; j < this->columnCount; j++) {
            this->rows[i][j] -= this->rows[j][i];
            this->rows[j][i] = -this->rows[i][j];
        }
    }
    this->dirty = 1;
}

/**
 * @brief returns if a page is Dirty or not (has been changed and needs to be
 * written to disk).
 */
bool Page::isDirty() {
    return this->dirty;
}
/**
 * @brief returns the table name the page belongs to.
 */
string Page::getTableName() {
    return this->tableName;
}
