//
// loader.h -- load most TRS-80 file formats.
//
// Supports (or wants to support):
//
//	.3bn - my format for Model I and Model III system files
//	.hex - Intel hex records (in our case, as output by zmac)
//	.cmd - dumps of /cmd executables from TRS-80 DOSs.
//	.bas - tokenized BASIC files; slight variants between cassette and disk
//	.bba - my binary BASIC format obviously slightly different than .bas
//	.cas - various TRS-80 cassette file dumps:
//		Level I tapes (basic and system the same but sorta distinguishable)
//		Level II System
//		Level II BASIC
//		Model III System
//		Model III BASIC
//
// Customized cassette loaders, basic data records and edtasm source files
// also considered.


// Basic class is a block of data.
// This may have a list of sources for each range of bits in the data.
// A source is either another data block or a file.
// Records are basically lists of data blocks.
// An Interpreter takes a data block and produces records.
// A record may indicate an error in the data or a processing error.

class Source
{
public:
	Source(const char *name);
	~Source();
	void GetLastError();

	enum Type {
		file,
		block,
		error,
	};

	Type m_type;
	char *m_name;
	char *m_errorMessage;
};

class Block
{
public:
	Block();
	Block(const char *filename);
	Block(const Block *base, int firstBitAddress, int lastBitAddress);
	~Block();

	void LoadFile(const char *filename);

	Source *m_source;

	enum Type {
		valid,
		fileError,
		memoryError
	};

	Type m_type;

	int AvailableBits(int bitAddress) const;
	int AvailableBytes(int byteAddress) const;
	int GetBits(int bitAddress, int length) const;
	int GetByte(int byteAddress) const;

	int GetSize() const;

	void SetSize(int bitSize);
	void PutBits(int bitAddress, int length, int bits);
	void PutByte(int byteAddress, int byte);

private:
	unsigned char *m_payload;
	int m_bitSize;
};

class TRS80Record
{
public:
	enum Action {
		leader,
		name,
		loadmem,
		execute,
		loadHL,
		originalAddr,	// for tokenized basic programs
		eof,
		cmdRecord		// punting on /CMD stuff I don't care about
	};
	TRS80Record(Action action, int addr, int length);
	~TRS80Record();

	void LoadAll(unsigned char *mem, int *pc, int *hl, unsigned char *loaded = 0);

	void SetWord(int byteOffset, int value);

	Action m_action;
	int m_size;
	int m_addr;
	unsigned char *m_data;
	TRS80Record *m_next;
};

class TRS80Loader
{
public:
	TRS80Loader();
	~TRS80Loader();

	enum Media {
		justBytes,
		cassette250,
		cassette500,
		cassette1500
	};

	enum Format {
		system,
		cmd,
		hex
	};

	void LoadFile(const char *filename, int basicLoadAddress);
	void LoadMemory(const unsigned char *memory, const unsigned char *loaded, int pc = -1);
	void DisposeFile();
	char *GetErrorMessage();

	bool SetsPC() const;

	void Encode(const char *name, Format format, Media media = justBytes);
	void SaveFile(const char *filename);

	TRS80Record *m_file;

private:
	int DetectMedia();
	void ParseSystemTape();
	void ParseLevel1Tape();
	void ParseTokenizedBasicTape(int basicLoadAddress);
	void ParseTokenizedBasic(int basicLoadAddress, int offset);
	void ParseBasic(int basicLoadAddress);
	void ParseHexFile();
	void ParseCmdFile();
	void AddRecord(TRS80Record *rec);
	void AddBasicPointers(int basicEndAddress);
	int GetMediaByte(int n);
	int GetMediaWord(int n);
	int GetMediaWordBigEndian(int n);
	void SeekMediaByte(int n);
	int GetHexByte(int *pos);

	void PutRawBits(int length, int bits);
	void PutMediaByte(int data);
	void PutMediaWord(int data);
	void PutHexByte(int data);

	Block m_data;

	Media m_curMedia;

	int m_bitPos;

	const char *m_errorMessage;
};

void NameFromFilename(TRS80Loader::Format format, const char *filename, char *name);
