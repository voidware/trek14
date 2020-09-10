//
// loader.cpp -- load most TRS-80 file formats.
//
// My primary purpose is to bring all the file format interpretation and
// parsing routines into one spot.  And to add high-speed cassette file
// loading.
//
// Then we go to the trouble of getting trs80gp, zidis and the odd utilities
// to use them.
//
// The basic design principle is that we have more than enough memory to
// load the file several times over.  No need for streaming.
//
// I intend to expose the layers of the data.  First the entire file is
// loaded as a single data chunk.  Cassette files may then see the data
// transformed into a byte stream and broken into several chunks.  Then
// those chunks are broken down into more chunks of data corresponding
// to individual load records, execute addresses and file names.  Finally,
// the last layer will copy data into a memory space.
//
// Errors in the data and in our processing will be recorded as special
// data chunks.
//
// I'm hoping to preserve information between layers so that a higher layer
// can identify the data in the previous layer that generated it.  This
// is angling towards some kind of visual environment where a user may
// interactively apply data processing steps to decypher unknown or
// damaged data.
//
// Scripting may be a nice way to extend what data formats are supported.
// Especially those programs with custom tape loaders.  But they're very
// little harm in building those in for now.
//
// Wacky (but useful) directions:
//
//      Load disk images.
//      Load .wav cassette dumps
//      Disassembly
//      Basic listing
//      Generation (say zmac creating a list of load records that can be
//          output in several formats)
//
//
// Level II ROM Reference Manual.pdf has some handy documentation of
// tape formats.
//

#include <stdio.h>
#include <stdlib.h> // for the old BASIC tokenization code.
#include <ctype.h>  // for the old BASIC tokenization code.
#include <string.h>
#include <assert.h>

#include "loader.h"

Block::Block()
{
    m_payload = 0;
    m_bitSize = 0;
    m_type = valid; // yeah, kinda
    m_source = 0;
}

Block::Block(const char *filename)
{
    m_payload = 0;
    m_bitSize = 0;
    m_type = valid; // yeah, kinda
    m_source = 0;

    LoadFile(filename);
}

void Block::LoadFile(const char *filename)
{
    delete m_source;
    m_source = new Source(filename);
    delete [] m_payload;
    m_payload = 0;

    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        m_type = fileError;
        m_source->GetLastError();
    }
    else
    {
        fseek(fp, 0, SEEK_END);
        int size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        m_payload = new unsigned char[size];
        fread(m_payload, size, 1, fp);
        fclose(fp);

        m_type = valid;

        m_bitSize = size * 8;
    }
}

Block::Block(const Block *base, int firstBitAddress, int lastBitAddress)
{
    delete m_source;
    m_source = 0;   // should point to the original block

    m_bitSize = (lastBitAddress - firstBitAddress) + 1;
    m_payload = new unsigned char[(m_bitSize + 7) / 8];
    m_type = valid;
        int i;
    for (i = 0; i < m_bitSize / 8; i++)
    {
        m_payload[i] = base->GetByte(i);
    }
    int rembits = m_bitSize % 8;
    if (rembits)
    {
        int bits = base->GetBits(m_bitSize - rembits, rembits);
        m_payload[m_bitSize / 8 + 1] = bits << (8 - rembits);
    }
}

Block::~Block()
{
    delete [] m_payload;
}

int Block::GetSize() const
{
    assert(m_type == valid);

    return m_bitSize;
}

void Block::SetSize(int bitSize)
{
    assert(bitSize >= 0);

    delete [] m_payload; m_payload = 0;
    if (bitSize > 0)
    {
        int byteSize = (bitSize + 7) / 8;
        m_payload = new unsigned char[byteSize];
        memset(m_payload, 0, byteSize);
    }
    m_bitSize = bitSize;
}

int Block::AvailableBits(int bitAddress) const
{
    if (bitAddress < 0 || bitAddress >= m_bitSize)
    {
        return 0;
    }
    else
    {
        return m_bitSize - bitAddress;
    }
}

int Block::AvailableBytes(int byteAddress) const
{
    return AvailableBits(byteAddress * 8) / 8;
}

int Block::GetBits(int bitAddress, int length) const
{
    assert(length <= 9);    // should really allow 32 or at least 16
    assert(bitAddress >= 0);
    assert(bitAddress < m_bitSize);
    assert(bitAddress + length >= 0);
    assert(bitAddress + length <= m_bitSize);

    const int bitMask = (1 << length) - 1;

    int byteAddress = bitAddress / 8;
    int bitPos = bitAddress % 8;

    int bitChunk = (m_payload[byteAddress] << 8) | m_payload[byteAddress + 1];
    return (bitChunk >> (16 - length - bitPos)) & bitMask;
}

int Block::GetByte(int byteAddress) const
{
    assert(byteAddress >= 0);
    assert(byteAddress < (m_bitSize - 8) / 8);

    return m_payload[byteAddress];
}

void Block::PutBits(int bitAddress, int length, int bits)
{
    assert(length <= 9);    // should really allow 32 or at least 16
    assert(bitAddress >= 0);
    assert(bitAddress < m_bitSize);
    assert(bitAddress + length >= 0);
    assert(bitAddress + length <= m_bitSize);

    const int bitMask = (1 << length) - 1;

    int byteAddress = bitAddress / 8;
    int bitPos = bitAddress % 8;

    bits &= bitMask;
    int bitChunk = (m_payload[byteAddress] << 8) | m_payload[byteAddress + 1];

    bitChunk &= ~(bitMask << (16 - length - bitPos));
    bitChunk |= bits << (16 - length - bitPos);

    m_payload[byteAddress] = bitChunk >> 8;
    m_payload[byteAddress + 1] = bitChunk;
}

void Block::PutByte(int byteAddress, int byte)
{
    assert(byteAddress >= 0);
    assert(byteAddress < (m_bitSize - 8) / 8);

    m_payload[byteAddress] = byte;
}

Source::Source(const char *name)
{
    int bytesNeeded = strlen(name) + 1;
    m_name = new char[bytesNeeded];
    strcpy(m_name, name);
    m_errorMessage = 0;
    m_type = file;
}

Source::~Source()
{
    delete [] m_name;
    delete [] m_errorMessage;
}

TRS80Record::TRS80Record(Action action, int addr, int length)
{
    m_action = action;
    m_addr = addr;
    m_size = length;
    m_data = new unsigned char[length];
    m_next = 0;
}

TRS80Record::~TRS80Record()
{
    delete [] m_data;
}

void TRS80Record::LoadAll(unsigned char *mem, int *pc, int *hl, unsigned char *loaded)
{
    for (TRS80Record *rec = this; rec; rec = rec->m_next)
    {
        if (rec->m_action == loadmem)
        {
            for (int i = 0; i < rec->m_size; i++)
            {
                int addr = (rec->m_addr + i) & 0xffff;
                mem[addr] = rec->m_data[i];
                if (loaded)
                {
                    loaded[addr]++;
                    if (loaded[addr] == 0)
                        loaded[addr]--;
                }
            }
        }
        else if (rec->m_action == execute)
        {
            if (pc)
            {
                *pc = rec->m_addr;
            }
        }
        else if (rec->m_action == loadHL)
        {
            if (hl)
            {
                *hl = rec->m_addr;
            }
        }
    }
}

void TRS80Record::SetWord(int byteOffset, int value)
{
    assert(byteOffset >= 0);
    assert(byteOffset < m_size - 1);

    m_data[byteOffset] = value & 255;
    m_data[byteOffset + 1] = value >> 8;
}

TRS80Loader::TRS80Loader()
{
    m_file = 0;
    m_errorMessage = 0;
}

TRS80Loader::~TRS80Loader()
{
    DisposeFile();
}

char *TRS80Loader::GetErrorMessage()
{
    const char *msg = 0;
    // Could use some accessor help from Block and Source, but whatever.
    if (m_errorMessage)
    {
        msg = m_errorMessage;
    }
    else if (m_data.m_source && m_data.m_source->m_type == Source::error)
    {
        msg = m_data.m_source->m_errorMessage;
    }

    if (msg)
    {
        int bytesNeeded = strlen(msg) + 1;
        char *err = new char[bytesNeeded];
        strcpy(err, msg);
        return err;
    }

    return 0;
}

void TRS80Loader::DisposeFile()
{
    TRS80Record *next;

    for (TRS80Record *t = m_file; t; t = next)
    {
        next = t->m_next;
        delete t;
    }

    m_file = 0;
}

bool TRS80Loader::SetsPC() const
{
    for (TRS80Record *rec = m_file; rec; rec = rec->m_next)
    {
        if (rec->m_action == TRS80Record::execute)
        {
            return true;
        }
    }

    return false;
}

void TRS80Loader::AddRecord(TRS80Record *rec)
{
    TRS80Record **prec = &m_file;
    while (*prec)
    {
        prec = &(*prec)->m_next;
    }

    *prec = rec;
}

void TRS80Loader::SaveFile(const char *filename)
{
    m_errorMessage = 0;

    // Gets a little cheesy here borrowing m_data.m_source and all.
    // In some sense "Source" should be more sensible.

    m_data.m_source = new Source(filename);

    FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
        m_data.m_source->GetLastError();
        m_errorMessage = m_data.m_source->m_errorMessage;
        return;
    }

    for (int bitPos = 0; bitPos < m_data.GetSize(); bitPos += 8)
    {
        int bitLen = m_data.GetSize() - bitPos;
        if (bitLen > 8)
        {
            bitLen = 8;
        }

        int byte = m_data.GetBits(bitPos, bitLen) << (8 - bitLen);
        if (fputc(byte, fp) == EOF)
        {
            m_data.m_source->GetLastError();
            m_errorMessage = m_data.m_source->m_errorMessage;
            fclose(fp);
            return;
        }
    }

    if (fclose(fp) == EOF)
    {
        m_data.m_source->GetLastError();
        m_errorMessage = m_data.m_source->m_errorMessage;
    }
}

void TRS80Loader::LoadFile(const char *filename, int basicLoadAddress)
{
    m_errorMessage = 0;

    DisposeFile();

    m_data.LoadFile(filename);

    // Some kind of low-level error?
    if (m_data.m_type != Block::valid)
    {
        return;
    }

    m_bitPos = 0;

    // Determine the type of file we have.

    // First look at the media type.
    int leaderLength = DetectMedia();

    if (leaderLength > 0)
    {
        TRS80Record *leader = new TRS80Record(TRS80Record::leader, 0, 0);
        AddRecord(leader);
    }

    // Many messy decisions to make based on media type and subsequent detection
    // of format.

    int byte0 = GetMediaByte(0);

    if (byte0 == 0x55)
    {
        ParseSystemTape();
    }
    else if (m_curMedia == cassette250)
    {
        ParseLevel1Tape();
    }
    else if (byte0 == 0xD3)
    {
        ParseTokenizedBasicTape(basicLoadAddress);
    }
    else if (m_curMedia == justBytes && byte0 == ':')
    {
        ParseHexFile();
    }
    else if (m_curMedia == justBytes && byte0 <= 0x1f)
    {
        ParseCmdFile();
    }
    else if (m_curMedia == justBytes && byte0 == 0xff)
    {
        ParseTokenizedBasic(basicLoadAddress, 1);
    }
    else if (m_curMedia == justBytes && (byte0 >= '1' && byte0 <= '9'))
    {
        ParseBasic(basicLoadAddress);
    }
    else
    {
        m_errorMessage = "Unrecognized format.";
    }
}

void TRS80Loader::LoadMemory(const unsigned char *memory, const unsigned char *loaded, int pc)
{
    DisposeFile();

    int low = 0;
    while (low < 65536)
    {
        if (!loaded[low])
        {
            low++;
            continue;
        }

        int high = low;
        while (high < 65536 && loaded[high])
        {
            high++;
        }
        TRS80Record *load = new TRS80Record(TRS80Record::loadmem, low, high - low);
        for (int i = low; i < high; i++)
        {
            load->m_data[i - low] = memory[i];
        }
        AddRecord(load);

        low = high;
    }

    if (pc >= 0)
    {
        TRS80Record *exec = new TRS80Record(TRS80Record::execute, pc, 0);
        AddRecord(exec);
    }
}

void TRS80Loader::Encode(const char *name, Format format, Media media)
{
    for (int pass = 0; pass < 2; pass++)
    {
        if (pass == 0)
        {
            // First pass we output nothing but compute size as side effect.
            m_data.SetSize(0);
            m_bitPos = 0;
        }
        else
        {
            // Second pass we output the data.
            m_data.SetSize(m_bitPos);
            m_bitPos = 0;
        }

        m_curMedia = media;

        // Output leader as required by media.

        if (media == cassette1500)
        {
            for (int i = 0; i < 255; i++)
            {
                PutRawBits(8, 0x55);
            }
            PutRawBits(8, 0x7f);
        }
        else if (media == cassette250 || media == cassette500)
        {
            for (int i = 0; i < 255; i++)
            {
                PutRawBits(8, 0);
            }
            PutRawBits(8, 0xa5);
        }

        // Output header byte and name as required
        if (format == system)
        {
            PutMediaByte(0x55);
            bool nameEnded = false;
            for (int i = 0; i < 6; i++)
            {
                if (!nameEnded)
                {
                    nameEnded = name[i] == '\0';
                }
                PutMediaByte(nameEnded ? ' ': name[i]);
            }
        }
        else if (format == cmd)
        {
            PutMediaByte(5);    // module name
            int len = strlen(name);
            if (len > 255)
            {
                len = 255;
            }
            PutMediaByte(len);
            for (int i = 0; i < len; i++)
            {
                PutMediaByte(name[i]);
            }
        }

        const int maxRecordSize = format == hex ? 16 : 256;

        // Output records for loaded data, fragmenting as necessary.
        // By necessity, an execute record will end the output.

        // Cheap shot here.  If no execute address assume BASIC Ready.
        // Even more cute would be BASIC RUN if possible.
        int pc = 0x1a19;
        for (TRS80Record *rec = m_file; rec; rec = rec->m_next)
        {
            if (rec->m_action == TRS80Record::loadmem)
            {
                for (int off = 0; off < rec->m_size; )
                {
                    int addr = rec->m_addr + off;
                    int len = rec->m_size - off;
                    if (len > maxRecordSize)
                    {
                        len = maxRecordSize;
                    }
                    int checksum = addr + (addr >> 8);

                    switch (format)
                    {
                    case system:
                        PutMediaByte(0x3c);
                        PutMediaByte(len & 255);
                        PutMediaWord(addr);
                        break;
                    case cmd:
                        PutMediaByte(1);
                        PutMediaByte((len + 2) & 255);
                        PutMediaWord(addr);
                        break;
                    case hex:
                        PutRawBits(8, ':');
                        PutHexByte(len);
                        checksum += len;
                        PutHexByte(addr >> 8);
                        PutHexByte(addr & 255);
                        PutHexByte(0);
                        checksum += 0; // checksum all hex digits
                        break;
                    default:
                        break;
                    }

                    for (int i = 0; i < len; i++)
                    {
                        int byte = rec->m_data[off + i];
                        if (format == hex)
                        {
                            PutHexByte(byte);
                        }
                        else
                        {
                            PutMediaByte(byte);
                        }
                        checksum += byte;
                    }

                    switch (format)
                    {
                    case system:
                        PutMediaByte(checksum);
                        break;
                    case cmd:
                        break;
                    case hex:
                        PutHexByte(-checksum);
                        PutRawBits(8, '\n');
                        break;
                    default:
                        break;
                    }

                    off += len;
                }
            }
            else if (rec->m_action == TRS80Record::execute)
            {
                pc = rec->m_addr;
                break;
            }
            // Ignore other records as irrelevant.
        }

        switch (format)
        {
        case system:
            PutMediaByte(0x78);
            PutMediaWord(pc);
            break;
        case cmd:
            PutMediaByte(2); // execution address
            PutMediaByte(2); // record length
            PutMediaWord(pc);
            break;
        case hex:
            PutRawBits(8, ':');
            PutHexByte(0);
            PutHexByte(pc >> 8);
            PutHexByte(pc & 255);
            PutHexByte(1);
            PutHexByte(-(1 + (pc >> 8) + (pc & 255)));
            PutRawBits(8, '\n');
            break;
        default:
            break;
        }

        // Cassette seems to require a few zeros at the end when I use "Play Cas".
        // So add them because at least they'll be ignored.  Another possible
        // control option.

        if (media == cassette250 || media == cassette500 || media == cassette1500)
        {
            for (int i = 0; i < 8; i++)
            {
                PutMediaByte(0);
            }
        }
    }
}

void NameFromFilename(TRS80Loader::Format format, const char *filename, char *name)
{
    int len;
    if (*filename == '\0')
    {
        *name = '\0';
        len = 0;
    }
    else
    {
        // Find base name.
        const char *basename = strchr(filename, '\0');
        for (; basename >= filename; basename--)
        {
            if (*basename == '/' || *basename == '\\')
            {
                break;
            }
        }
        basename++;

        // Copy up to 6 and/or stop at dot.
        for (len = 0; len < 6; len++)
        {
            int ch = basename[len];
            if (ch == '\0' || ch == '.')
            {
                break;
            }
            if (islower(ch))
            {
                ch = toupper(ch);
            }
            name[len] = ch;
        }
        name[len] = '\0';
    }

    // Padding may be necessary depending on the format.
    if (format == TRS80Loader::system)
    {
        while (len < 6)
        {
            name[len++] = ' ';
        }
        name[len] = '\0';
    }
}

void TRS80Loader::ParseSystemTape()
{
    // Assume we already have seen the 0x55 name header code.

    int bpos = 1;

    TRS80Record *name = new TRS80Record(TRS80Record::name, 0, 6);
    for (int i = 0; i < 6; i++)
    {
        name->m_data[i] = GetMediaByte(bpos++);
    }
    if (m_errorMessage)
    {
        delete name;
        return;
    }
    AddRecord(name);

    while (GetMediaByte(bpos) == 0x3c)
    {
        bpos++;
        int length = GetMediaByte(bpos++);
        if (m_errorMessage) return;
        if (length == 0)
        {
            length = 256;
        }
        int addr = GetMediaWord(bpos);
        if (m_errorMessage) return;
        int sum  = (addr & 255) + (addr >> 8);
        bpos += 2;
        TRS80Record *load = new TRS80Record(TRS80Record::loadmem, addr, length);

        for (int i = 0; i < length; i++)
        {
            load->m_data[i] = GetMediaByte(bpos++);
            sum += load->m_data[i];
            if (m_errorMessage)
            {
                delete load;
                return;
            }
        }
        int checksum = GetMediaByte(bpos++);
        if (m_errorMessage)
        {
            delete load;
            return;
        }

        if (checksum != (sum & 255))
        {
            delete load;
            m_errorMessage = "Checksum error.";
            return;
        }
        AddRecord(load);
    }

    if (GetMediaByte(bpos) == 0x78)
    {
        int execAddr = GetMediaWord(bpos + 1);
        if (m_errorMessage) return;
        bpos += 3;
        TRS80Record *exec = new TRS80Record(TRS80Record::execute, execAddr, 0);
        AddRecord(exec);
        SeekMediaByte(bpos); // advance to next file
    }
    else
    {
        m_errorMessage = "Cassette system file with no execute address.";
    }
}

void TRS80Loader::ParseLevel1Tape()
{
    // Level I begins with a big endian load address.  Must be $4200 for BASIC.
    // If you want it to execute, machine language must load over $41fe.
    // $4091 is the most reasonable low point, but lower is theoretically possible.
    //
    // But we're not going to care here; just simulate things best we can.

    int startAddress = GetMediaWordBigEndian(0);
    int endAddress = GetMediaWordBigEndian(2);
    if (m_errorMessage) return;
    int length = endAddress - startAddress + 1;

    TRS80Record *load = new TRS80Record(TRS80Record::loadmem, startAddress, length);
    // Note that the checksum is loaded into memory as per the real system.
    int sum =  0;
    for (int i = 0; i < length; i++)
    {
        load->m_data[i] = GetMediaByte(i + 4);
        sum += load->m_data[i];
        if (m_errorMessage)
        {
            delete load;
            return;
        }
    }

    if ((sum & 255) != 0)
    {
        delete load;
        m_errorMessage = "Checksum error.";
        return;
    }

    AddRecord(load);
    AddRecord(new TRS80Record(TRS80Record::loadHL, endAddress, 0));

    // Level 1 BASIC programs load at $4200 and do not automatically execute.
    // Level 1 machine langauge programs load lower than $4200 and overwrite
    // a stack address so they run automatically.  We check for that possibility
    // and set the PC if so.  We don't set the stack pointer, though nor
    // generally perform all the possible side-effects.
    // Happily, the $eec return address is same for Model 1 and Model 3 level 1 ROM.

    // Excessive loop here to get the PC, but no sense fooling around with a
    // bunch of range checks and the like especially if we wish to catch the
    // wraparound cases.

    int addr = startAddress;
    int newPC = 0xeec;
    for (int j = 0; j < length; j++)
    {
        if (addr == 0x41fe)
        {
            newPC &= ~0xff;
            newPC |= load->m_data[j];
        }
        else if (addr == 0x41ff)
        {
            newPC &= ~0xff00;
            newPC |= load->m_data[j] << 8;
        }

        addr++;
        addr &= 0xffff;
    }

    if (newPC != 0xeec)
    {
        AddRecord(new TRS80Record(TRS80Record::execute, newPC, 0));
    }
}

void TRS80Loader::ParseTokenizedBasicTape(int basicLoadAddress)
{
    int d0 = GetMediaByte(0);
    int d1 = GetMediaByte(1);
    int d2 = GetMediaByte(2);
    int nameByte = GetMediaByte(3);

    if (m_errorMessage) return;

    if (d0 != 0xD3 || d1 != 0xD3 || d2 != 0xD3)
    {
        m_errorMessage = "Missing BASIC tape header 0xD3 bytes.";
        return;
    }

    TRS80Record *name = new TRS80Record(TRS80Record::name, 0, 1);
    name->m_data[0] = nameByte;
    AddRecord(name);

    ParseTokenizedBasic(basicLoadAddress, 4);
}

void TRS80Loader::ParseTokenizedBasic(int basicLoadAddress, int pos)
{
    for (;;)
    {
        int nextLine = GetMediaWord(pos);
        pos += 2;
        if (m_errorMessage) return;
        AddRecord(new TRS80Record(TRS80Record::originalAddr, nextLine, 0));
        if (nextLine == 0)
        {
            break;
        }
        int lineLength = 2;
        int b = 0;
        do
        {
            b = GetMediaByte(pos + lineLength);
            if (m_errorMessage) return;
            lineLength++;
        } while (b != 0);
        TRS80Record *line = new TRS80Record(TRS80Record::loadmem, basicLoadAddress, lineLength + 2);
        int nextAddr = basicLoadAddress + 2 + lineLength;
        line->SetWord(0, nextAddr);
        for (int i = 0; i < lineLength; i++)
        {
            line->m_data[2 + i] = GetMediaByte(pos++);
        }
        AddRecord(line);

        basicLoadAddress = nextAddr;
    }

    AddBasicPointers(basicLoadAddress);
}

void TRS80Loader::AddBasicPointers(int basicEndAddress)
{
    TRS80Record *ptr;

    // Null pointer to terminate BASIC program.
    ptr = new TRS80Record(TRS80Record::loadmem, basicEndAddress, 2);
    ptr->SetWord(0, 0);
    AddRecord(ptr);
    basicEndAddress += 2;

    // Now add records to set up BASIC's program pointers.

    // end of BASIC program pointer
    ptr = new TRS80Record(TRS80Record::loadmem, 0x40f9, 2);
    ptr->SetWord(0, basicEndAddress);
    AddRecord(ptr);

    // start of array variables pointer
    ptr = new TRS80Record(TRS80Record::loadmem, 0x40fb, 2);
    ptr->SetWord(0, basicEndAddress);
    AddRecord(ptr);

    // start of free memory pointer
    ptr = new TRS80Record(TRS80Record::loadmem, 0x40fd, 2);
    ptr->SetWord(0, basicEndAddress);
    AddRecord(ptr);
}

static int tokenize(const char *line, unsigned char *tok);

void TRS80Loader::ParseBasic(int basicLoadAddress)
{
    int pos = 0;

    while (m_data.AvailableBytes(pos) > 0)
    {
        char line[1024];
        // Read a line.
        char *p = line;
        for (;;)
        {
            int ch = GetMediaByte(pos++);
            if (ch == '\n' || ch == '\r' || ch < 0)
            {
                *p++ = '\n';
                *p = '\0';
                break;
            }
            *p++ = ch;
        }

        unsigned char tok[1024];
        int tokenizedLength = tokenize(line, tok);
        if (tokenizedLength <= 0)
        {
            m_errorMessage = "Syntax error.";
            return;
        }

        TRS80Record *lineRec = new TRS80Record(TRS80Record::loadmem, basicLoadAddress, tokenizedLength + 2);
        int nextAddr = basicLoadAddress + 2 + tokenizedLength;
        lineRec->SetWord(0, nextAddr);
        for (int i = 0; i < tokenizedLength; i++)
        {
            lineRec->m_data[2 + i] = tok[i];
        }

        AddRecord(lineRec);
        basicLoadAddress = nextAddr;

        while (m_data.AvailableBytes(pos) > 0)
        {
            int ch = GetMediaByte(pos);
            if (ch == '\n' || ch == '\r')
            {
                pos++;
            }
            else
            {
                break;
            }
        }
    }

    AddBasicPointers(basicLoadAddress);
}

static int HexDigitValue(int digit)
{
    if (digit >= '0' && digit <= '9')
        return digit - '0';

    if (digit >= 'A' && digit <= 'F')
        return 10 + digit - 'A';

    if (digit >= 'a' && digit <= 'f')
        return 10 + digit - 'a';

    return -1;
}

static int HexPairValue(int highDigit, int lowDigit)
{
    int high = HexDigitValue(highDigit);
    int low = HexDigitValue(lowDigit);
    if (high >= 0 && low >= 0)
    {
        return high * 16 + low;
    }

    return -1;
}

int TRS80Loader::GetHexByte(int *pos)
{
    int hexByte = HexPairValue(GetMediaByte(*pos), GetMediaByte(*pos + 1));
    if (hexByte < 0)
    {
        if (!m_errorMessage) m_errorMessage = "Bad hexadecimal number.";
    }

    *pos += 2;

    return hexByte;
}

void TRS80Loader::ParseHexFile()
{
    int pos = -1;
    for (;;)
    {
        do
        {
            pos++;
            if (m_errorMessage || m_data.AvailableBytes(pos) < 8)
            {
                return;
            }
        }
        while (HexDigitValue(GetMediaByte(pos)) < 0);
        
        int hexRecordLength = GetHexByte(&pos);
        int address = GetHexByte(&pos) << 8;
        address += GetHexByte(&pos);

        // Skip record type byte; we don't need it.  Here 0 == data, 1 == execute
        int type = GetHexByte(&pos);

        if (m_errorMessage) return;

        if (hexRecordLength == 0)
        {
            // I assume this is a checksum; not bothering with it for now.
            int checksum = GetHexByte(&pos);
            if (m_errorMessage) return;

            AddRecord(new TRS80Record(TRS80Record::execute, address, 0));
        }
        else
        {
            TRS80Record *load = new TRS80Record(TRS80Record::loadmem, address, hexRecordLength);
            for (int i = 0; i < hexRecordLength; i++)
            {
                load->m_data[i] = GetHexByte(&pos);
            }
            // I assume this is a checksum; not bothering with it for now.
            int checksum = GetHexByte(&pos);
            if (m_errorMessage)
            {
                delete load;
                return;
            }
            AddRecord(load);
        }
    }
}

void TRS80Loader::ParseCmdFile()
{
    int pos = 0;

    while (m_data.AvailableBytes(pos) > 0)
    {
        int cmdRecordType = GetMediaByte(pos++);
        if (m_errorMessage) return;
        if (cmdRecordType > 0x1f)
        {
            m_errorMessage = "Invalid /CMD record type.";
            return;
        }

        int length = GetMediaByte(pos++);
        if (m_errorMessage) return;

        TRS80Record::Action recordType;
        int addr = cmdRecordType;
        switch (cmdRecordType)
        {
        case 0:
            AddRecord(new TRS80Record(TRS80Record::eof, 0, 0));
            // TODO - could add crap past end as part of true fidelity.
            return;
        case 1:
        case 2:
            addr = GetMediaWord(pos);
            pos += 2;
            if (cmdRecordType == 1)
            {
                length -= 2;
                length &= 255;
                recordType = TRS80Record::loadmem;
            }
            else
            {
                recordType = TRS80Record::execute;
                length = -1;
            }
            break;
        case 5:
            recordType = TRS80Record::name;
            break;
        default:
            recordType = TRS80Record::cmdRecord;
            break;
        }

        if (length == 0)
            length = 256;

        if (length == -1)
            length = 0;

        TRS80Record *rec = new TRS80Record(recordType, addr, length);

        for (int i = 0; i < length; i++)
        {
            rec->m_data[i] = GetMediaByte(pos + i);
        }
        pos += length;

        if (m_errorMessage)
        {
            delete rec;
            return;
        }
        AddRecord(rec);

        // It would seem that crap after the execution address is OK.  I'd like to generate
        // a warning but have no warning mechanism as of yet.
        if (recordType == TRS80Record::execute)
        {
            return;
        }
    }
}

int TRS80Loader::DetectMedia()
{
    m_curMedia = justBytes;

    // Try high speed cassette.  Header of $55 bytes followed by $7F.
    // Or {01}* 0111 1111.  Transferred tapes will have the $7F sync byte
    // byte aligned, but we'll do things at a bit level.  But won't go
    // into the machinations of skipping bad bits.
    int leaderLength = 0;
    int bitWanted = 0;
        int i;
    for (i = m_bitPos; i < m_data.GetSize() - 8; i++)
    {
        if (leaderLength > 32 && m_data.GetBits(i, 8) == 0x7f)
        {
            leaderLength += 8;
            m_curMedia = cassette1500;
            m_bitPos += leaderLength;
            return leaderLength;
        }
        if (leaderLength == 0)
        {
            bitWanted = !m_data.GetBits(i, 1);
            leaderLength++;
        }
        else
        {
            int bit = m_data.GetBits(i, 1);
            if (bit != bitWanted)
            {
                break;
            }
            else
            {
                leaderLength++;
                bitWanted = !bitWanted;
            }
        }
    }

    // Now look at low-speed cassette.  Generally $00 followed by $A5.
    // Or 0* 1010 0101.  But then we'll have to figure out if it was
    // really 250 baud.

    leaderLength = 0;
    for (i = m_bitPos; i < m_data.GetSize() - 8; i++)
    {
        if (leaderLength > 32 && m_data.GetBits(i, 8) == 0xa5)
        {
            leaderLength += 8;
            m_curMedia = cassette500;
            m_bitPos += leaderLength;
            break;
        }
        else if (m_data.GetBits(i, 1) == 0)
        {
            leaderLength++;
        }
        else
        {
            break;
        }
    }

    // $40, $41 or $42 will mean Level I.  There are more robust
    // checks we could do but this will suffice.
    if (m_curMedia == cassette500)
    {
        int loadHi = m_data.GetBits(m_bitPos, 8);
        if (loadHi == 0x40 || loadHi == 0x41 || loadHi == 0x42)
        {
            m_curMedia = cassette250;
        }

        return leaderLength;
    }

    // We don't really know; leave it to the default we set originally.

    return 0;
}

int TRS80Loader::GetMediaByte(int n)
{
    int dataByte = -1;
    switch (m_curMedia)
    {
    case justBytes:
    case cassette250:
    case cassette500:
        if (m_data.AvailableBits(m_bitPos + n * 8) >= 8)
        {
            dataByte = m_data.GetBits(m_bitPos + n * 8, 8);
        }
        break;
    case cassette1500:
        if (m_data.AvailableBits(m_bitPos + 1 + n * 9) >= 8)
        {
            // Don't care if we have 0 or 1 as a start bit
            dataByte = m_data.GetBits(m_bitPos + 1 + n * 9, 8);
        }
        break;
    }

    if (dataByte < 0)
    {
        m_errorMessage = "File too short.";
    }

    return dataByte;
}

int TRS80Loader::GetMediaWord(int byteOffset)
{
    return GetMediaByte(byteOffset) + (GetMediaByte(byteOffset + 1) << 8);
}

int TRS80Loader::GetMediaWordBigEndian(int byteOffset)
{
    return (GetMediaByte(byteOffset) << 8) + GetMediaByte(byteOffset + 1);
}

void TRS80Loader::SeekMediaByte(int n)
{
    int bitsPerByte = m_curMedia == cassette1500 ? 9 : 8;
    m_bitPos += n * bitsPerByte;
}

void TRS80Loader::PutRawBits(int length, int bits)
{
    if (m_data.AvailableBits(m_bitPos) >= length)
    {
        m_data.PutBits(m_bitPos, length, bits);
    }
    m_bitPos += length;
}

void TRS80Loader::PutMediaByte(int data) 
{
    if (m_curMedia == cassette1500)
    {
        PutRawBits(1, 0); // 0 start bit
    }
    PutRawBits(8, data);
}

void TRS80Loader::PutMediaWord(int data)
{
    PutMediaByte(data & 255);
    PutMediaByte(data >> 8);
}

void TRS80Loader::PutHexByte(int data)
{
    char buf[3];
    sprintf(buf, "%02X", data & 255);
    PutMediaByte(buf[0]);
    PutMediaByte(buf[1]);
}

// Just putting this here out of everyone's hair.

static char* token[] = {
    "END",
    "FOR", "RESET", "SET", "CLS", "CMD", "RANDOM", "NEXT", "DATA", "INPUT",
    "DIM", "READ", "LET", "GOTO", "RUN", "IF", "RESTORE", "GOSUB", "RETURN",
    "REM", "STOP", "ELSE", "TRON", "TROFF", "DEFSTR", "DEFINT", "DEFSNG",
    "DEFDBL", "LINE", "EDIT", "ERROR", "RESUME", "OUT", "ON", "OPEN",
    "FIELD", "GET", "PUT", "CLOSE", "LOAD", "MERGE", "NAME", "KILL", "LSET",
    "RSET", "SAVE", "SYSTEM", "LPRINT", "DEF", "POKE", "PRINT", "CONT",
    "LIST", "LLIST", "DELETE", "AUTO", "CLEAR", "CLOAD", "CSAVE", "NEW",
    "TAB(", "TO", "FN", "USING", "VARPTR", "USR", "ERL", "ERR", "STRING$",
    "INSTR", "POINT", "TIME$", "MEM", "INKEY$", "THEN", "NOT", "STEP",
    "+", "-", "*", "/", "[", "AND", "OR", ">", "=", "<", "SGN", "INT",
    "ABS", "FRE", "INP", "POS", "SQR", "RND", "LOG", "EXP", "COS", "SIN",
    "TAN", "ATN", "PEEK", "CVI", "CVS", "CVD", "EOF", "LOC", "LOF", "MKI$",
    "MKS$", "MKD$", "CINT", "CSNG", "CDBL", "FIX", "LEN", "STR$", "VAL",
    "ASC", "CHR$", "LEFT$", "RIGHT$", "MID$" };

struct tok {
    char*       fullname;
    int         len;
    int         token;
    struct tok* next;
};

static bool made_key_lookup = 0;
static struct tok* char_to_tok[256];

//
// Create a table which will give us each reserved word that starts with
// each character.  This should make the tokenizing quite quick.
//
static int make_key_lookup()
{
    int i;
    int n;
    struct tok* t;
    struct tok** tpp;
    char first;

    if (made_key_lookup)
        return 1;

    made_key_lookup = true;

    n = sizeof(token) / sizeof(token[0]);

    if ((t = (struct tok *)calloc(n, sizeof(struct tok))) == NULL) {
        return 0;
    }

    for (i = 0; i < 256; i++)
        char_to_tok[i] = NULL;

    for (i = 0; i < n; i++) {
        first = token[i][0];
        t[i].fullname = token[i];
        t[i].len = strlen(token[i]);
        t[i].token = i + 128;
        t[i].next = NULL;
        for (tpp = &(char_to_tok[first]); *tpp != NULL; tpp = &((*tpp)->next))
            ;
        *tpp = t + i;
    }

    return 1;
}

#define REM (147)
#define DATA (136)
#define REMQUOT (251)
#define ELSE (149)

static int tokenize(const char *line, unsigned char *tok)
{
    int lnum;
    int ch;
    unsigned char*  tp;
    struct tok* t;

    make_key_lookup();

    tp = tok;

    while (isspace(*line))
        line++;

    lnum = atoi(line);

    if (!isdigit(*line))
        return 0;

    while (isdigit(*line))
        line++;

    while (isspace(*line))
        line++;

    if (!*line)
        return 0;

    *tp++ = lnum & 255;
    *tp++ = (lnum >> 8) & 255;

/*  printf("%d ", lnum); */

    while (*line) {
        switch (*line) {
        case '\n':
            line++;
            break;
        case '\'':
            *tp++ = ':';
            *tp++ = REM;
            *tp++ = REMQUOT;
            line++;
            while (*tp++ = *line++)
                ;
            if (tp[-2] == '\n') {
                tp[-2] = '\0';
                tp--;
            }
            return tp - tok;
        case '"':
            *tp++ = *line++;
            while (*line && *line != '\n') {
                *tp++ = *line++;
                if (line[-1] == '"') {
                    break;
                }
            }
            break;
        default:
            ch = *line & 255;
            if (islower(ch))
                ch = toupper(ch);

            *tp = ch;
            for (t = char_to_tok[ch]; t != NULL; t = t->next) {
                if (!strnicmp(t->fullname, line, t->len)) {
                    if (t->token == REM || t->token == DATA) {
                        *tp++ = t->token;
                        line += t->len;
                        while (*tp++ = *line++)
                            ;
                        if (tp[-2] == '\n') {
                            tp[-2] = '\0';
                            tp--;
                        }
                        return tp - tok;
                    }
                    else if (t->token == ELSE && tp[-1] != ':')
                        *tp++ = ':';

                    *tp = t->token;
                    line += t->len - 1;
                    break;
                }
            }
            tp++;
            line++;
        }
    }
    *tp++ = '\0';
    return tp - tok;
}

// Ugly, platform-specific stuff beyond this point.
// Should be seperate file, etc.

#include <windows.h>

void Source::GetLastError()
{ 
    // Retrieve the system error message for the last-error code

    char *msgbuf;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        ::GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &msgbuf,
        0, NULL );

    // I'll use the C++ allocator, thank you.
    delete [] m_errorMessage;
    int bytesNeeded = strlen(msgbuf) + 1;
    m_errorMessage = new char[bytesNeeded];
    strcpy(m_errorMessage, msgbuf);

    LocalFree(msgbuf);

    m_type = error;
}
