class BlockStats {
public:
    int blocksWritten, blocksRead;
    BlockStats(): blocksWritten(0), blocksRead(0) {}
    void ReadBlock();
    void WriteBlock();
    void log();
    void clearStats();
};