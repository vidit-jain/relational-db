#include"logger.h"
/**
 * @brief The Page object is the main memory representation of a physical page
 * (equivalent to a block). The page class and the page.h header file are at the
 * bottom of the dependency tree when compiling files. 
 *<p>
 * Do NOT modify the Page class. If you find that modifications
 * are necessary, you may do so by posting the change you want to make on Moodle
 * or Teams with justification and gaining approval from the TAs. 
 *</p>
 */
enum datatype {TABLE, MATRIX};
class Page{

    string tableName;
    int pageIndex;
    int columnCount;
    int rowCount;
    int dirty = 0;
    vector<vector<int>> rows;

    public:

    string pageName = "";
    Page();
    Page(string tableName, int pageIndex, datatype d);
    Page(string tableName, int pageIndex, vector<vector<int>> rows, int rowCount, int colCount);
    vector<int> getRow(int rowIndex);
    int getCell(int row, int col);
    void transpose(Page* p);
    void transpose();
    bool isDirty();
    void subtractTranspose(Page* p);
    void subtractTranspose();
    void setPageName(string newName);
    void writePage();
};