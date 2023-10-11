#include "global.h"


/**
 * @brief Construct a new Table:: Table object
 *
 */
Table::Table() {
    logger.log("Table::Table");
}

/**
 * @brief Construct a new Table:: Table object used in the case where the data
 * file is available and LOAD command has been called. This command should be
 * followed by calling the load function;
 *
 * @param tableName 
 */
Table::Table(string tableName) {
    logger.log("Table::Table");
    this->sourceFileName = "../data/" + tableName + ".csv";
    this->tableName = tableName;
}

/**
 * @brief Construct a new Table:: Table object used when an assignment command
 * is encountered. To create the table object both the table name and the
 * columns the table holds should be specified.
 *
 * @param tableName 
 * @param columns 
 */
Table::Table(string tableName, vector<string> columns) {
    logger.log("Table::Table");
    this->sourceFileName = "../data/temp/" + tableName + ".csv";
    this->tableName = tableName;
    this->columns = columns;
    this->columnCount = columns.size();
    for (int i = 0; i < columns.size(); i++) this->colNameToIdx[columns[i]] = i;
    this->maxRowsPerBlock = (uint) ((BLOCK_SIZE * 1000) / (sizeof(int) * columnCount));
    this->writeRow<string>(columns);
}

/**
 * @brief Construct a new Table::Table object using the originalTable provided
 *
 * @param tableName
 * @param originalTable
 */
Table::Table(string tableName, Table *originalTable) {
    this->sourceFileName = "../data" + tableName + ".csv";
    this->tableName = tableName;
    this->columns = originalTable->columns;
    this->distinctValuesPerColumnCount = originalTable->distinctValuesPerColumnCount;
    this->columnCount = originalTable->columnCount;
    this->rowCount = originalTable->rowCount;
    this->blockCount = originalTable->blockCount;
    this->maxRowsPerBlock = originalTable->maxRowsPerBlock;
    this->rowsPerBlockCount = originalTable->rowsPerBlockCount;
    this->indexed = originalTable->indexed;
    this->indexedColumn = originalTable->indexedColumn;
    this->indexingStrategy = originalTable->indexingStrategy;
    this->colNameToIdx = originalTable->colNameToIdx;
}

/**
 * @brief The load function is used when the LOAD command is encountered. It
 * reads data from the source file, splits it into blocks and updates table
 * statistics.
 *
 * @return true if the table has been successfully loaded 
 * @return false if an error occurred 
 */
bool Table::load() {
    logger.log("Table::load");
    fstream fin(this->sourceFileName, ios::in);
    string line;
    if (getline(fin, line)) {
        fin.close();
        if (this->extractColumnNames(line))
            if (this->blockify())
                return true;
    }
    fin.close();
    return false;
}

/**
 * @brief Function extracts column names from the header line of the .csv data
 * file. 
 *
 * @param line 
 * @return true if column names successfully extracted (i.e. no column name
 * repeats)
 * @return false otherwise
 */
bool Table::extractColumnNames(string firstLine) {
    logger.log("Table::extractColumnNames");
    string word;
    stringstream s(firstLine);
    int idx = 0;
    while (getline(s, word, ',')) {
        word.erase(std::remove_if(word.begin(), word.end(), ::isspace), word.end());
        if (colNameToIdx.find(word) != colNameToIdx.end())
            return false;
        colNameToIdx[word] = idx++;
        this->columns.emplace_back(word);
    }
    this->columnCount = this->columns.size();
    this->maxRowsPerBlock = (uint) ((BLOCK_SIZE * 1000) / (sizeof(int) * this->columnCount));
    return true;
}

/**
 * @brief This function splits all the rows and stores them in multiple files of
 * one block size. 
 *
 * @return true if successfully blockified
 * @return false otherwise
 */
bool Table::blockify() {
    logger.log("Table::blockify");
    ifstream fin(this->sourceFileName, ios::in);
    string line, word;
    vector<int> row(this->columnCount, 0);
    vector<vector<int>> rowsInPage(this->maxRowsPerBlock, row);
    int pageCounter = 0;
    unordered_set<int> dummy;
    dummy.clear();
    this->distinctValuesInColumns.assign(this->columnCount, dummy);
    this->distinctValuesPerColumnCount.assign(this->columnCount, 0);
    getline(fin, line);
    while (getline(fin, line)) {
        stringstream s(line);
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++) {
            if (!getline(s, word, ','))
                return false;
            row[columnCounter] = stoi(word);
            rowsInPage[pageCounter][columnCounter] = row[columnCounter];
        }
        pageCounter++;
        this->updateStatistics(row);
        if (pageCounter == this->maxRowsPerBlock) {
            bufferManager.writePage(this->tableName, this->blockCount, rowsInPage, pageCounter, rowsInPage[0].size());
            this->blockCount++;
            this->rowsPerBlockCount.emplace_back(pageCounter);
            pageCounter = 0;
        }
    }
    if (pageCounter) {
        bufferManager.writePage(this->tableName, this->blockCount, rowsInPage, pageCounter, rowsInPage.size());
        this->blockCount++;
        this->rowsPerBlockCount.emplace_back(pageCounter);
        pageCounter = 0;
    }

    if (this->rowCount == 0)
        return false;
    this->distinctValuesInColumns.clear();
    return true;
}

/**
 * @brief Given a row of values, this function will update the statistics it
 * stores i.e. it updates the number of rows that are present in the column and
 * the number of distinct values present in each column. These statistics are to
 * be used during optimisation.
 *
 * @param row 
 */
void Table::updateStatistics(vector<int> row) {
    this->rowCount++;
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++) {
        if (!this->distinctValuesInColumns[columnCounter].count(row[columnCounter])) {
            this->distinctValuesInColumns[columnCounter].insert(row[columnCounter]);
            this->distinctValuesPerColumnCount[columnCounter]++;
        }
    }
}

/**
 * @brief Checks if the given column is present in this table.
 *
 * @param columnName 
 * @return true 
 * @return false 
 */
bool Table::isColumn(string columnName) {
    logger.log("Table::isColumn");

    return (colNameToIdx.find(columnName) != colNameToIdx.end());
}

/**
 * @brief Renames the column indicated by fromColumnName to toColumnName. It is
 * assumed that checks such as the existence of fromColumnName and the non prior
 * existence of toColumnName are done.
 *
 * @param fromColumnName 
 * @param toColumnName 
 */
void Table::renameColumn(string fromColumnName, string toColumnName) {
    logger.log("Table::renameColumn");

    if (colNameToIdx.find(fromColumnName) == colNameToIdx.end()) return; // Should never occur. Sanity check

    int target = colNameToIdx[fromColumnName];
    columns[target] = toColumnName;
    auto nodeHandler = colNameToIdx.extract(fromColumnName);
    nodeHandler.key() = toColumnName;
    colNameToIdx.insert(std::move(nodeHandler));
}

/**
 * @brief Function prints the first few rows of the table. If the table contains
 * more rows than PRINT_COUNT, exactly PRINT_COUNT rows are printed, else all
 * the rows are printed.
 *
 */
void Table::print() {
    logger.log("Table::print");
    uint count = min((long long) PRINT_COUNT, this->rowCount);

    //print headings
    this->writeRow(this->columns, cout);

    Cursor cursor(this->tableName, 0, TABLE);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < count; rowCounter++) {
        row = cursor.getNext();
        this->writeRow(row, cout);
    }
    printRowCount(this->rowCount);
}


/**
 * @brief This function returns one row of the table using the cursor object. It
 * returns an empty row is all rows have been read.
 *
 * @param cursor 
 * @return vector<int> 
 */
void Table::getNextPage(Cursor *cursor) {
    logger.log("Table::getNext");

    if (cursor->pageIndex < this->blockCount - 1) {
        cursor->nextPage(cursor->pageIndex + 1);
    }
}


/**
 * @brief called when EXPORT command is invoked to move source file to "data"
 * folder.
 *
 */
void Table::makePermanent() {
    logger.log("Table::makePermanent");
    if (!this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    string newSourceFile = "../data/" + this->tableName + ".csv";
    ofstream fout(newSourceFile, ios::out);

    //print headings
    this->writeRow(this->columns, fout);

    Cursor cursor(this->tableName, 0, TABLE);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++) {
        row = cursor.getNext();
        this->writeRow(row, fout);
    }
    fout.close();
}

/**
 * @brief Function to check if table is already exported
 *
 * @return true if exported
 * @return false otherwise
 */
bool Table::isPermanent() {
    logger.log("Table::isPermanent");
    if (this->sourceFileName == "../data/" + this->tableName + ".csv")
        return true;
    return false;
}

/**
 * @brief The unload function removes the table from the database by deleting
 * all temporary files created as part of this table
 *
 */
void Table::unload() {
    logger.log("Table::~unload");
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->tableName, pageCounter);
    if (!isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
}

/**
 * @brief Function that returns a cursor that reads rows from this table
 * 
 * @return Cursor 
 */
Cursor Table::getCursor() {
    logger.log("Table::getCursor");
    Cursor cursor(this->tableName, 0, TABLE);
    return cursor;
}

/**
 * @brief Function that returns the index of column indicated by columnName
 * 
 * @param columnName 
 * @return int 
 */
int Table::getColumnIndex(string columnName) {
    logger.log("Table::getColumnIndex");

    if (colNameToIdx.find(columnName) == colNameToIdx.end()) return -1; //Should never occur. Sanity check
    return colNameToIdx[columnName];
}

/**
 * @brief Function that returns the indices of the columns indicated by columnNames
 *
 * @param columnNames
 * @return vector<int> containing the indices of the columnNames in order of input
 */
vector<int> Table::getColumnIndex(const vector<string> &columnNames) {
    logger.log("Table::getColumnIndex");

    vector<int> ret;
    for (const auto &i: columnNames) ret.emplace_back(getColumnIndex(i));
    return ret;
}

/**
 * @brief Function that sorts the table based on the subset of columns provided
 *
 * @param colNames Names of columns to sort the table on
 * @param colMultipliers Specifies the multipliers for each of the columns
 */
void Table::sort(const vector<std::string> &colNames, const vector<int> &colMultipliers) {
    logger.log("Table::sort");
    auto colIndices = getColumnIndex(colNames);
    sortingPhase(colIndices, colMultipliers);
    mergingPhase(colIndices, colMultipliers);
}

/**
 * @brief Performs the sorting phase of the external sort algorithm
 *
 * @param colIndices Indices of the columns to perform the sort on
 * @param colMultipliers Specifies the ordering for each column via multipliers (1 or -1)
 */
void Table::sortingPhase(const vector<int> &colIndices, const vector<int> &colMultipliers) {
    logger.log("Table::sortingPhase");

    const auto nb = BLOCK_COUNT - 1; //size of the buffer in blocks
    const auto b = blockCount; //size of the file in blocks
    const auto nr = (b + nb - 1) / nb; //Number of initial runs: ceil(B/Nb)

    Cursor cursor = getCursor();
    vector<vector<int>> rows(maxRowsPerBlock * nb, vector<int>(columnCount));
    vector<vector<int>> writeRows(maxRowsPerBlock, vector<int>(columnCount));
    auto cmp = [&colIndices, &colMultipliers](const vector<int> &A, const vector<int> &B) {
        for (int k = 0; k < colIndices.size() - 1; k++) {
            if (A[colIndices[k]] != B[colIndices[k]])
                return (A[colIndices[k]] * colMultipliers[k] < B[colIndices[k]] * colMultipliers[k]);
        }
        return (A[colIndices.back()] * colMultipliers.back() < B[colIndices.back()] * colMultipliers.back());
    };
    auto remBlocksToRead = b, remBlocksToWrite = b;
    auto blocksRead = 0, blocksWritten = 0;
    for (int runIdx = 0; runIdx < nr; runIdx++) {
        int rowReadCounter = 0;
        for (int blkIdx = 0; blkIdx < min(nb, remBlocksToRead); blkIdx++) {
            for (int r = 0; r < rowsPerBlockCount[blocksRead]; r++)
                rows[rowReadCounter++] = cursor.getNext();
            blocksRead++;
        }
        remBlocksToRead = b - blocksRead;
        std::sort(rows.begin(), rows.begin() + rowReadCounter, cmp);
        int rowWrittenCounter = 0;
        for (int blkIdx = 0; blkIdx < min(nb, remBlocksToWrite); blkIdx++) {
            for (int r = 0; r < rowsPerBlockCount[blocksWritten]; r++)
                writeRows[r] = rows[rowWrittenCounter++];
            bufferManager.writePage(tableName, blocksWritten, writeRows, rowsPerBlockCount[blocksWritten], columnCount);
            blocksWritten++;
        }
        remBlocksToWrite = b - blocksWritten;
    }
}

/**
 * @brief Performs the merging phase of the external sort algorithm
 *
 * @param colIndices
 * @param colMultipliers
 */
void Table::mergingPhase(const vector<int> &colIndices, const vector<int> &colMultipliers) {
    logger.log("Table::mergingPhase");

    const auto nb = BLOCK_COUNT - 1; //size of the buffer in blocks
    const auto b = blockCount; //size of the file in blocks
    auto nr = (b + nb - 1) / nb; //Number of initial runs: ceil(B/Nb)
    vector<vector<int>> writeRows(maxRowsPerBlock, vector<int>(columnCount));
    auto runSize = nb;
    vector<uint> remRows(nb, 0);
    vector<Cursor> currCursors(nb);
    auto cmp = [&colIndices, &colMultipliers](const pair<vector<int>, int> &A, const pair<vector<int>, int> &B) {
        for (int k = 0; k < colIndices.size() - 1; k++) {
            if (A.first[colIndices[k]] != B.first[colIndices[k]])
                return (A.first[colIndices[k]] * colMultipliers[k] > B.first[colIndices[k]] * colMultipliers[k]);
        }
        return (A.first[colIndices.back()] * colMultipliers.back() >
                B.first[colIndices.back()] * colMultipliers.back());
    };
    //TODO: Make cmp util?
    priority_queue<pair<vector<int>, int>, vector<pair<vector<int>, int>>, decltype(cmp)> pq(cmp);
    string readTableName = tableName, writeTableName = "sort_buffer_" + tableName;
    while (tableCatalogue.isTable(writeTableName)) writeTableName += "_"; //TODO: Random?
    auto writeTable = new Table(writeTableName, this);
    tableCatalogue.insertTable(writeTable);
    while (nr > 1) {
        logger.log("Table:MergePhaseStage");
        logger.log(to_string(nr) + "," + to_string(runSize));
        auto curr = (nr + nb - 1) / nb; //Number of subfiles to write in this pass: ceil(nr / nb)
        int writeBlockCounter = 0;
        for (uint runIdx = 0; runIdx < curr; runIdx++) {
            int i = 0;
            for (auto blkIdx = runIdx * nb * runSize; blkIdx < min((runIdx + 1) * nb * runSize, b); blkIdx += runSize) {
                currCursors[i] = Cursor(readTableName, blkIdx, TABLE);
                logger.log(to_string(runIdx) + "," + to_string(blkIdx));
                for (int j = 0; j < runSize and blkIdx + j < b; j++)
                    remRows[i] += rowsPerBlockCount[blkIdx + j]; //TODO: use maxRowsPerBlock instead?
                assert(remRows[i]); //Should never happen. Sanity check
                vector<int> row = currCursors[i].getNext();
                remRows[i]--;
                pq.emplace(row, i);
                i++;
            }
            for (auto &j: remRows) logger.log(to_string(j) + ":");
            auto writeRowCounter = 0;
            while (!pq.empty()) {
                if (writeRowCounter == maxRowsPerBlock) {
                    bufferManager.writePage(writeTableName, writeBlockCounter++, writeRows, writeRowCounter,
                                            columnCount);
                    writeRowCounter = 0;
                }
                auto [row, idx] = pq.top();
                pq.pop();
                writeRows[writeRowCounter++] = row;
                if (remRows[idx]) {
                    remRows[idx]--;
                    row = currCursors[idx].getNext();
                    pq.emplace(row, idx);
                }
            }
            if (writeRowCounter) {
                bufferManager.writePage(writeTableName, writeBlockCounter++, writeRows, writeRowCounter, columnCount);
                writeRowCounter = 0;
            }
        }
        nr = curr, runSize *= nb;
        readTableName.swap(writeTableName);
    }
    readTableName.swap(writeTableName);
    if (writeTableName != tableName) {
        writeTable->rename(tableName);
        tableCatalogue.eraseTable(writeTableName);
    } else tableCatalogue.eraseTable(readTableName);
}

/**
 * @brief Renames the table and all its associated pages
 * @param newName
 */
void Table::rename(const string &newName) {
    logger.log("Table::rename");
    bufferManager.renamePagesInMemory(tableName, newName);

    for (int pageCounter = 0; pageCounter < blockCount; pageCounter++)
        bufferManager.renameFile(tableName, newName, pageCounter);
    tableName = newName;
}
