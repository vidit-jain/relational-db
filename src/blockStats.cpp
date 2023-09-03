#include "global.h"

void BlockStats::ReadBlock() { this->blocksRead++; }
void BlockStats::WriteBlock() { this->blocksWritten++; }

/**
 * @brief Logs the stats of the number of blocks read, written and accessed. Doesn't
 * count reads from memory as a read.
 */
void BlockStats::log() {
    cout << "\nNumber of blocks read: " << this->blocksRead << endl;
    cout << "Number of blocks written: " << this->blocksWritten << endl;
    cout << "Number of blocks accessed: " << this->blocksRead + this->blocksWritten << endl;
    clearStats();
}

/**
 * @brief Sets the block statistics to 0
 */
void BlockStats::clearStats() {
    this->blocksRead = 0;
    this->blocksWritten = 0;
}
