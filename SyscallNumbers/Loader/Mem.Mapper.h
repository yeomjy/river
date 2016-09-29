#ifndef _MEM_MAPPER_H_
#define _MEM_MAPPER_H_

#include "Abstract.Mapper.h"

class MemMapper : public AbstractPEMapper {
public :
	virtual void *CreateSection(void *lpAddress, size_t dwSize, DWORD flProtect);
	virtual bool ChangeProtect(void *lpAddress, size_t dwSize, DWORD flProtect);
	virtual bool WriteBytes(void *lpAddress, void *lpBuffer, size_t nSize);

	virtual DWORD FindImport(const char *moduleName, const char *funcName);
	virtual DWORD FindImport(const char *moduleName, const unsigned int funcOrdinal);
};

#endif