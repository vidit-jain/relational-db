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
    int symmetric = -1; // -1 - not computed, 0 - not symmetric, 1 - symmetric
    int m, concurrentBlocks;
    vector<pair<int,int>> dimsPerBlock;
    bool blockify();
    Matrix();
    Matrix(string matrixName);
    Matrix(string matrixName, Matrix* originalMatrix);
    bool extractDimension(string fileName);
    bool load();
    void print();
    void makePermanent();
    bool isPermanent();
    void transpose();
    bool blockDimensions();
    void getNextPage(Cursor *cursor);
    bool symmetry();
    Cursor getCursor();
    void unload();
    void compute(string originalMatrix);
    void rename(string newName);

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

    template<typename T>
    void writeRows(vector<vector<T>> rows, int rowCount, ostream& fout) {
        for (int i = 0; i < rowCount; i++) writeRow(rows[i], fout);
    }
};
