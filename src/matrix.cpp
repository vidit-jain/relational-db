#include "global.h"

/**
 * @brief Construct a new Matrix::Matrix object
 *
 */
Matrix::Matrix()
{
    logger.log("Matrix::Matrix");
}

/**
 * @brief Construct a new Matrix::Matrix object used in the case where the data
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
}

/**
 * @brief Construct a new Matrix::Matrix object used when an assignment command
 * is encountered. To create the Matrix object both the matrix name and the
 * dimensions of the matrix should be specified
 *
 * @param matrixName
 * @param dimension
 */
Matrix::Matrix(string matrixName, long long int dimension)
{
    logger.log("Matrix::Matrix");
    this->sourceFileName = "../data/temp/" + matrixName + ".csv";
    this->matrixName = matrixName;
    this->dimension = dimension;
    //this->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * columnCount));
}

/**
 * @brief The load function is used when the LOAD command is encountered. It
 * reads data from the source file, splits it into blocks.
 *
 * @return true if the matrix has been successfully loaded
 * @return false if an error occurred
 */
bool Matrix::load()
{
    logger.log("Matrix::load");
    if (this->extractDimension(this->sourceFileName)) {
        if (this->blockify())
            return true;
    }
    return false;
}

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

bool Matrix::blockDimensions() {
    int totalIntegers = (BLOCK_SIZE * 1000) / (sizeof(int));
    int c = sqrt(totalIntegers);
    // avoiding precision issues
    while ((c + 1) * (c + 1) <= totalIntegers) c++;
    while (c * c > totalIntegers) c--;
    if (c == 0) return false;
    this->m = c;
    this->concurrentBlocks = (this->dimension + this->m - 1) / this->m;
    return true;
}
/**
 * @brief This function splits all the rows and stores them in multiple files of
 * one block size.
 *
 * @return true if successfully blockified
 * @return false otherwise
 */
bool Matrix::blockify() {
    logger.log("Matrix::blockify");
    ifstream fin(this->sourceFileName, ios::in);
    if (!blockDimensions()) return false;

    vector<vector<vector<int>>> grids(concurrentBlocks, \
                              vector<vector<int>>(m, \
                                      vector<int>(m)));
    string line, word;
    int rowIndex = 0, rowsRead = 0;
    function<void()> writeToBuffer = [&] () {
        for (int i = 0; i < concurrentBlocks; i++) {
            int colSize = (i == concurrentBlocks - 1 && this->dimension % m) ? (this->dimension % m) : m;
            bufferManager.writePage(this->matrixName, this->blockCount, grids[i], rowIndex, colSize);
            this->blockCount++;
            this->dimsPerBlock.emplace_back(rowIndex, colSize);
        }
        rowIndex = 0;
    };
    while (getline(fin, line))
    {
        stringstream s(line);
        for (int columnCounter = 0; columnCounter < this->dimension; columnCounter++) {
            if (!getline(s, word, ',')) return false;
            grids[columnCounter / m][rowIndex][columnCounter % m] = stoi(word);
        }
        rowIndex++, rowsRead++;
        if (rowIndex == m) writeToBuffer();
    }
    if (rowIndex) writeToBuffer();
    if (rowsRead != 0) return true;
    return false;
}

/**
 * @brief Function prints the first few rows and columns of the matrix. If the matrix contains
 * more rows than PRINT_COUNT, exactly PRINT_COUNT rows and columns are printed, else all
 * the rows and columns are printed.
 */
void Matrix::print()
{
    uint count = min((long long)PRINT_COUNT, this->dimension);
    vector<vector<int>> mat(count, vector<int>(count));

    Cursor cursor(this->matrixName, 0, MATRIX);
    int rows = (count + m - 1) / m;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < rows; j++) {
            cursor.nextPage(i * concurrentBlocks + j);

            for (int k = 0; k < min(m, (int)count - i * m); k++) {
                vector<int> row = cursor.getNext();
                for (int l = 0; l < min(m, (int)count - j * m); l++) {
                    mat[i * m + k][j * m + l] = row[l];
                }
            }
        }
    }
    writeRows(mat, count, cout);
    printRowCount(this->dimension);
}

/**
 * @brief This function returns one row of the matrix using the cursor object. It
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

    vector<vector<int>> mat(m, vector<int>(this->dimension));
    Cursor cursor(this->matrixName, 0, MATRIX);
    for (int i = 0; i < concurrentBlocks; i++) {
        for (int j = 0; j < concurrentBlocks; j++) {
            cursor.nextPage(i * concurrentBlocks + j);

            for (int k = 0; k < min(m, (int)this->dimension - i * m); k++) {
                vector<int> row = cursor.getNext();
                for (int l = 0; l < min(m, (int)this->dimension - j * m); l++) {
                    mat[k][j * m + l] = row[l];
                }
            }
        }
        int rowCount = min(m, (int)this->dimension - i * m);
        this->writeRows(mat, rowCount, fout);
    }
    fout.close();
}

/**
 * @brief Function to check if matrix is already exported
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
 * @brief The unload function removes the matrix from the database by deleting
 * all temporary files created as part of this matrix
 *
 */
void Matrix::unload(){
    logger.log("Matrix::~unload");
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->matrixName, pageCounter);
    if (!isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
}

bool Matrix::symmetry() {
    if (symmetric != -1) return symmetric;
    logger.log("Concurrent: " + to_string(concurrentBlocks));
    for (int i = 0; i < concurrentBlocks; i++) {
        for (int j = i; j < concurrentBlocks; j++) {
            if (i == j) {
                Cursor a(this->matrixName, i * concurrentBlocks + j, MATRIX);
                int lim = min((long long) this->m, this->dimension - i * m);
                for (int k = 0; k < lim; k++)
                    for (int l = k + 1; l < lim; l++)
                        if (a.getCell(k, l) != a.getCell(l, k)) return symmetric = false;
            }
            else {
                Cursor a(this->matrixName, i * concurrentBlocks + j, MATRIX);
                Cursor b(this->matrixName, j * concurrentBlocks + i, MATRIX);
                int limrow = min((long long)this->m, this->dimension - i * m);
                int limcol = min((long long)this->m, this->dimension - j * m);
                logger.log(to_string(limrow) + " " + to_string(limcol) + " " + to_string(i * m + j) +  " " + to_string(j * m + i));
                for (int k = 0; k < limrow; k++)
                    for (int l = k + 1; l < limcol; l++)
                        if (a.getCell(k, l) != b.getCell(l, k)) return symmetric = false;
            }
        }
    }
    return symmetric = true;
}

/**
 * @brief Function that returns a cursor that reads rows from this matrix
 *
 * @return Cursor
 */
Cursor Matrix::getCursor()
{
    logger.log("Matrix::getCursor");
    Cursor cursor(this->matrixName, 0, MATRIX);
    return cursor;
}
