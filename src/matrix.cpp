#include "global.h"

/**
 * @brief Construct a new Matrix:: Matrix object
 *
 */
Matrix::Matrix()
{
    logger.log("Matrix::Matrix");
}

/**
 * @brief Construct a new Matrix:: Matrix object used in the case where the data
 * file is available and LOAD command has been called. This command should be
 * followed by calling the load function;
 *
 * @param matrixName
 */
Matrix::Matrix(string matrixName)
{
    logger.log("Matrix::Matrix");
    this->sourceFileName = "../data/" + matrixName + ".csv";
    this->matrixName = matrixName;
    this->maxValuesPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int)));
}

/**
 * @brief Construct a new Matrix::Matrix object used when an assignment command
 * is encountered. To create the matrix object both the matrix name and the
 * dimension of the matrix should be specified.
 *
 * @param matrixName
 * @param dimension
 */
Matrix::Matrix(string matrixName, int dimension)
{
    logger.log("Matrix::Matrix");
    this->sourceFileName = "../data/temp/" + matrixName + ".csv";
    this->matrixName = matrixName;
    this->dimension = dimension;
    this->maxValuesPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int)));
}

/**
 * @brief The load function is used when the LOAD command is encountered. It
 * reads data from the source file, splits it into blocks and updates table
 * statistics.
 *
 * @return true if the matrix has been successfully loaded
 * @return false if an error occurred
 */
bool Matrix::load()
{
    logger.log("Matrix::load");
    if (extractDimension(this->sourceFileName))
    {
        if (this->blockify())
            return true;
    }
    return false;
}

/**
 * @brief Function extracts dimension of matrix from given fileName
 *
 * @param fileName
 * @return true if dimension is successfully extracted
 * @return false otherwise
 */
bool Matrix::extractDimension(string fileName)
{
    logger.log("Matrix::extractDimension");
    fstream fin(fileName, ios::in);
    string line;
    if (getline(fin, line)) {
        this->dimension = std::count(line.begin(), line.end(), ',') + 1;
        fin.close();
        return true;
    }
    fin.close();
    return false;
}

/**
 * @brief This function splits all the values and stores them in multiple files of
 * one block size.
 *
 * @return true if successfully blockified
 * @return false otherwise
 */
bool Matrix::blockify()
{
    logger.log("Matrix::blockify");
    ifstream fin(this->sourceFileName, ios::in);
    string line, word;
    vector<int> row(this->maxValuesPerBlock, 0);
    vector<vector<int>> rowsInPage(1, vector<int>(this->maxValuesPerBlock, 0));
    int pageCounter = 0;
    while (getline(fin, line))
    {
        stringstream s(line);
        for (int columnCounter = 0; columnCounter < this->dimension; columnCounter++)
        {
            if (!getline(s, word, ','))
                return false;
            rowsInPage[0][pageCounter] = stoi(word);
            pageCounter++;
            if (pageCounter == this->maxValuesPerBlock)
            {
                bufferManager.writePage(this->matrixName, this->blockCount, rowsInPage, 1);
                this->blockCount++;
                this->valuesPerBlockCount.emplace_back(pageCounter);
                pageCounter = 0;
            }
        }
    }
    if (pageCounter)
    {
        rowsInPage[0].resize(pageCounter);
        bufferManager.writePage(this->matrixName, this->blockCount, rowsInPage, 1);
        this->blockCount++;
        this->valuesPerBlockCount.emplace_back(pageCounter);
        pageCounter = 0;
    }

    return true;
}

/**
 * @brief Function prints the first few rows of the table. If the table contains
 * more rows than PRINT_COUNT, exactly PRINT_COUNT rows are printed, else all
 * the rows are printed.
 *
 */
void Matrix::print()
{
    logger.log("Matrix::print");
    uint count = min((long long)PRINT_COUNT, this->dimension);

    Cursor cursor(this->matrixName, 0);
    int currPos = 0;
    vector<int> row;
    for (int rowCounter = 0; rowCounter < count; rowCounter++)
    {
        row = cursor.getNext();
        this->writeRow(row, cout, currPos);
    }
    printRowCount(this->dimension);
}



/**
 * @brief This function returns one row of the table using the cursor object. It
 * returns an empty row is all rows have been read.
 *
 * @param cursor
 * @return vector<int>
 */
void Matrix::getNextPage(Cursor *cursor)
{
    logger.log("Matrix::getNext");

    if (cursor->pageIndex < this->blockCount - 1)
    {
        cursor->nextPage(cursor->pageIndex+1);
    }
}



/**
 * @brief called when EXPORT command is invoked to move source file to "data"
 * folder.
 *
 */
void Matrix::makePermanent()
{
    logger.log("Matrix::makePermanent");
    if(!this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    string newSourceFile = "../data/" + this->matrixName + ".csv";
    ofstream fout(newSourceFile, ios::out);

    Cursor cursor(this->matrixName, 0);
    vector<int> row;
    int currPos = 0;
    for (int rowCounter = 0; rowCounter < this->blockCount; rowCounter++)
    {
        row = cursor.getNext();
        this->writeRow(row, fout, currPos);
    }
    fout.close();
}

/**
 * @brief Function to check if table is already exported
 *
 * @return true if exported
 * @return false otherwise
 */
bool Matrix::isPermanent()
{
    logger.log("Matrix::isPermanent");
    if (this->sourceFileName == "../data/" + this->matrixName + ".csv")
        return true;
    return false;
}

/**
 * @brief The unload function removes the table from the database by deleting
 * all temporary files created as part of this table
 *
 */
void Matrix::unload(){
    logger.log("Matrix::~unload");
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->matrixName, pageCounter);
    if (!isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
}

/**
 * @brief Function that returns a cursor that reads rows from this table
 *
 * @return Cursor
 */
Cursor Matrix::getCursor()
{
    logger.log("Matrix::getCursor");
    Cursor cursor(this->matrixName, 0);
    return cursor;
}
