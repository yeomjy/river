#include <stdio.h>
#include "Abstract.Loader.h"
#include "Abstract.Importer.h"
#include "Inproc.Mapper.h"
#include "Inproc.Native.Importer.h"
#include "LoaderAPI.h"

typedef void* (*handler)(unsigned long);

int main() {
	//ldr::AbstractBinary *fElf = ldr::LoadBinary("tested");
	//if (!fElf)
	//	return -1;

	//if (fElf->IsValid()) {
	//	printf("All				is good!\n\n");
	//}

	ldr::DWORD entry = GetEntryPoint("tested");
	printf("Got entry point %lx\n", entry);

	//void *func = (void*)imp.FindImport("librevtracerwrapper.so", "CallAllocateMemoryHandler");

	//void *addr = ((handler)func) (1024);
	//printf("Allocated %d bytes @address %p\n", 1024, addr);

	//delete fElf;

	return 0;
}
