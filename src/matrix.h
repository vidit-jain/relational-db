#include "cursor.h"

/**
 * @brief The Matrix class holds all information related to a loaded matrix.
 * It also implements methods that interact with the parsers, executors, cursors
 * and buffer manager. A matrix is usually created by a LOAD command or by the
 * COMPUTE command.
 */
class Matrix
{

public:
    string sourceFileName = "";
    string matrixName = "";
    long long int dimension = 0;
    uint blockCount = 0;
    int m, concurrentBlocks;
    vector<pair<int,int>> dimsPerBlock;
    bool blockify();
    Matrix();
    Matrix(string matrixName);
    Matrix(string matrixName, long long int dimension);
    bool extractDimension(string fileName);
    bool load();
    void print();
    void makePermanent();
    bool isPermanent();
    bool blockDimensions();
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
    void writeRow(vector<T> row, ostream &fout)
    {
        logger.log("Table::printRow");
        for (int columnCounter = 0; columnCounter < row.size(); columnCounter++)
        {
            if (columnCounter != 0)
                fout << ", ";
            fout << row[columnCounter];
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
        logger.log("Table::printRow");
        ofstream fout(this->sourceFileName, ios::app);
        this->writeRow(row, fout);
        fout.close();
    }

};
