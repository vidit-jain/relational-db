#include "cursor.h"

/**
 * @brief The Matrix class holds all information related to a loaded table. It
 * also implements methods that interact with the parsers, executors, cursors
 * and the buffer manager. There are typically 2 ways a table object gets
 * created through the course of the workflow - the first is by using the LOAD
 * command and the second is to use assignment statements (SELECT, PROJECT,
 * JOIN, SORT, CROSS and DISTINCT).
 *
 */
class Matrix
{

public:
    string sourceFileName = "";
    string matrixName = "";
    long long int dimension = 0;
    bool symmetric = true;
    uint blockCount = 0;
    uint maxValuesPerBlock = 0;
    vector<uint> valuesPerBlockCount;

    bool extractDimension(string fileName);
    bool blockify();
    Matrix();
    Matrix(string tableName);
    Matrix(string tableName, int columnSize);
    bool load();
    void print();
    void makePermanent();
    bool isPermanent();
    void getNextPage(Cursor *cursor);
    Cursor getCursor();
    void unload();

    /**
 * @brief Static function that takes a vector of valued and prints them out in a
 * comma seperated format.
 *
 * @tparam T current usaages include int and string
 * @param row
 */
    template <typename T>
    void writeRow(vector<T> row, ostream &fout, int& currPos)
    {
        logger.log("Matrix::printRow");
        for (int columnCounter = 0; columnCounter < row.size(); columnCounter++)
        {
            if (columnCounter != 0)
                fout << ", ";
            fout << row[columnCounter];
            currPos = (currPos + 1) % this->dimension;
            if (currPos == 0) fout << "\n";
        }
        fout << endl;
    }

/**
 * @brief Static function that takes a vector of valued and prints them out in a
 * comma seperated format.
 *
 * @tparam T current usaages include int and string
 * @param row
 */
    template <typename T>
    void writeRow(vector<T> row)
    {
        logger.log("Matrix::printRow");
        ofstream fout(this->sourceFileName, ios::app);
        this->writeRow(row, fout);
        fout.close();
    }
};
