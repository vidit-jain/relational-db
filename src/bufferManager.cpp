#include "global.h"

BufferManager::BufferManager() {
    logger.log("BufferManager::BufferManager");
    this->blocksWritten = this->blocksRead = 0;
}

/**
 * @brief Function called to read a page from the buffer manager. If the page is
 * not present in the pool, the page is read and then inserted into the pool.
 *
 * @param tableName 
 * @param pageIndex 
 * @return Page*
 */
Page *BufferManager::getPage(string tableName, int pageIndex, datatype d) {
    logger.log("BufferManager::getPage");
    string pageName = "../data/temp/" + tableName + "_Page" + to_string(pageIndex);
    if (this->inPool(pageName))
        return this->getFromPool(pageName);
    else
        return this->insertIntoPool(tableName, pageIndex, d);
}

/**
 * @brief Checks to see if a page exists in the pool
 *
 * @param pageName 
 * @return true 
 * @return false 
 */
bool BufferManager::inPool(string pageName) {
    logger.log("BufferManager::inPool");
    for (auto page: this->pages) {
        if (pageName == page.pageName and !page.isDeleted())
            return true;
    }
    return false;
}

/**
 * @brief If the page is present in the pool, then this function returns the
 * page. Note that this function will fail if the page is not present in the
 * pool.
 *
 * @param pageName 
 * @return Page*
 */
Page *BufferManager::getFromPool(string pageName) {
    logger.log("BufferManager::getFromPool");
    for (auto &page: this->pages)
        if (pageName == page.pageName)
            return &page;
}

/**
 * @brief Inserts page indicated by tableName and pageIndex into pool. If the
 * pool is full, the pool ejects the oldest inserted page from the pool and adds
 * the current page at the end. It naturally follows a queue data structure. 
 *
 * @param tableName 
 * @param pageIndex 
 * @return Page*
 */
Page *BufferManager::insertIntoPool(string tableName, int pageIndex, datatype d) {
    logger.log("BufferManager::insertIntoPool");
    this->blocksRead++;
    Page page(tableName, pageIndex, d);
    if (this->pages.size() >= BLOCK_COUNT) {
        if (!pages.front().isDeleted() and pages.front().isDirty()) {
            pages.front().writePage();
            this->blocksWritten++;
        }
        pages.pop_front();
    }
    pages.push_back(page);
    return &pages.back();
}

/**
 * @brief The buffer manager is also responsible for writing pages. This is
 * called when new tables are created using assignment statements.
 *
 * @param tableName 
 * @param pageIndex 
 * @param rows 
 * @param rowCount 
 */
void BufferManager::writePage(string tableName, int pageIndex, vector<vector<int>> rows, int rowCount, int colCount) {
    logger.log("BufferManager::writePage");

    string pageName = "../data/temp/" + tableName + "_Page" + to_string(pageIndex);
    if (!inPool(pageName)) {
        this->blocksWritten++;
        Page page(tableName, pageIndex, rows, rowCount, colCount);
        page.writePage();
    } else {
        auto page = getFromPool(pageName);
        page->modifyPage(rows, rowCount, colCount);
    }
}

/**
 * @brief Deletes file named fileName
 *
 * @param fileName 
 */
void BufferManager::deleteFile(string fileName) {

    if (remove(fileName.c_str()))
        logger.log("BufferManager::deleteFile: Err");
    else logger.log("BufferManager::deleteFile: Success");
}

/**
 * @brief Renames file from oldName to newName
 *
 * @param oldName
 * @param newName
 */
void BufferManager::renameFile(string oldName, string newName) {

    if (rename(oldName.c_str(), newName.c_str()))
        logger.log("BufferManager::renameFile: Err");
    else logger.log("BufferManager::renameFile: Success");
}

/**
 * @brief Goes through pages in the deque and renames pages from oldName to newName
 *
 * @param oldName
 * @param newName
 */

void BufferManager::renamePagesInMemory(string oldName, string newName) {
    assert(oldName != newName); //Should never occur. Sanity check
    for (auto &page: this->pages) {
        if (page.getTableName() == newName)
            page.setDeleted();
        else if (page.getTableName() == oldName)
            page.setPageName(newName);
    }
}

/**
 * @brief Overloaded function that calls deleteFile(fileName) by constructing
 * the fileName from the tableName and pageIndex.
 *
 * @param tableName 
 * @param pageIndex 
 */
void BufferManager::deleteFile(string tableName, int pageIndex) {
    logger.log("BufferManager::deleteFile");
    string fileName = "../data/temp/" + tableName + "_Page" + to_string(pageIndex);
    this->deleteFile(fileName);
}

/**
 * @brief Overloaded function that calls renameFile(oldName, newName) by constructing
 * the fileName from the file names and pageIndex.
 *
 * @param oldName
 * @param newName
 * @param pageIndex
 */
void BufferManager::renameFile(string oldName, string newName, int pageIndex) {
    logger.log("BufferManager::deleteFile");
    string oldFileName = "../data/temp/" + oldName + "_Page" + to_string(pageIndex);
    string newFileName = "../data/temp/" + newName + "_Page" + to_string(pageIndex);
    this->renameFile(oldFileName, newFileName);
}