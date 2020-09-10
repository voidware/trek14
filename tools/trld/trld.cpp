//
// trld -- translate/load TRS-80 binaries
//
// General use is to translate format between .cas, .cmd, .hex, etc.  Can load
// more than one binary at once to concatenate them.
//
// Initially does not preserve the original structure and doesn't handle binary
// basic files in a sensible form.
//
// Suggestions: Control over BASIC load addresses.  Flag for low-speed cassette
// output. Tolerate checksum errors. Auto-load SYSTEM files.  Option to leave
// name out of .cmd output.
//
// Future possibilities include translations that preserve as much orignial
// structure as possible.  Simple dumps.  Module offsets.
//

const int basicLoadAddress = 17385;  // Model III cassette Basic

#include <stdio.h>
#include <string.h>

#include "loader.h"

static void usage();

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		usage();
		return 1;
	}

	TRS80Loader load;
	unsigned char memory[65536];
	unsigned char loaded[65536];
	int pc = -1;
        int i;

	memset(loaded, 0, sizeof(loaded));

	for (i = 1; i < argc - 1; i++)
	{
		load.LoadFile(argv[i], basicLoadAddress);
		char *loadErrorMessage = load.GetErrorMessage();
		if (loadErrorMessage)
		{
			printf("trld: error loading '%s': %s\n", argv[i], loadErrorMessage);
			delete loadErrorMessage;
			return 2;
		}

		int hl;
		load.m_file->LoadAll(memory, &pc, &hl, loaded);
	}

	load.LoadMemory(memory, loaded, pc);

	TRS80Loader::Format format = TRS80Loader::system;
	//TRS80Loader::Media media = TRS80Loader::cassette1500;
	TRS80Loader::Media media = TRS80Loader::cassette500;

	// Decide output format and media from output file name.
	const char *extension = strrchr(argv[argc - 1], '.');
	if (extension)
	{
		if (stricmp(extension, ".cmd") == 0)
		{
			format = TRS80Loader::cmd;
			media = TRS80Loader::justBytes;
		}
		else if (stricmp(extension, ".hex") == 0)
		{
			format = TRS80Loader::hex;
			media = TRS80Loader::justBytes;
		}
	}

	char name[32];
	NameFromFilename(format, argv[argc - 1], name);
	load.Encode(name, format, media);

	load.SaveFile(argv[argc - 1]);
	char *errorMessage = load.GetErrorMessage();
	if (errorMessage)
	{
		printf("trld: error saving '%s': %s\n", argv[i], errorMessage);
		delete errorMessage;
		return 3;
	}

	return 0;
}

void usage()
{
	printf("usage: trld file1.cmd {file2.cas, file3.hex} output.cmd\n");
}
