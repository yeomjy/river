#include "SymbopTranslator.h"

#include "CodeGen.h"

void SymbopTranslator::CopyInstruction(RiverInstruction &rOut, const RiverInstruction &rIn) {
	memcpy(&rOut, &rIn, sizeof(rOut));

	if (RIVER_OPTYPE_MEM == RIVER_OPTYPE(rIn.opTypes[0])) {
		rOut.operands[0].asAddress = codegen->CloneAddress(*rIn.operands[0].asAddress, rIn.modifiers);
	}

	if (RIVER_OPTYPE_MEM == RIVER_OPTYPE(rIn.opTypes[1])) {
		rOut.operands[1].asAddress = codegen->CloneAddress(*rIn.operands[1].asAddress, rIn.modifiers);
	}
}

DWORD SymbopTranslator::GetMemRepr(const RiverAddress &mem) {
	return 0;
}

bool SymbopTranslator::Init(RiverCodeGen *cg) {
	codegen = cg;
	return true;
}

bool SymbopTranslator::Translate(const RiverInstruction &rIn, RiverInstruction *rMainOut, DWORD &instrCount, RiverInstruction *rTrackOut, DWORD &trackCount) {
	if (RIVER_FAMILY(rIn.family) == RIVER_FAMILY_RIVER) {
		/* do not track river operations */
		CopyInstruction(*rMainOut, rIn);
		rMainOut++;
		instrCount++;
		return true;
	}

	DWORD dwTable = (RIVER_MODIFIER_EXT & rIn.modifiers) ? 1 : 0;
	(this->*translateOpcodes[dwTable][rIn.opCode])(rIn, rMainOut, instrCount, rTrackOut, trackCount);

	return true;
}

void SymbopTranslator::MakeInitTrack(RiverInstruction *&rTrackOut, DWORD &trackCount) {
	trackedValues = 0;

	rTrackOut->opCode = 0xB8; // mov eax, 0
	rTrackOut->subOpCode = 0;
	rTrackOut->modifiers = 0;
	rTrackOut->family = RIVER_FAMILY_TRACK;

	rTrackOut->opTypes[0] = RIVER_OPTYPE_REG;
	rTrackOut->operands[0].asRegister.versioned = RIVER_REG_xDI;

	rTrackOut->opTypes[1] = RIVER_OPTYPE_IMM; 
	rTrackOut->operands[1].asImm32 = 0;

	rTrackOut->opTypes[2] = rTrackOut->opTypes[3] = RIVER_OPTYPE_NONE;

	rTrackOut++;
	trackCount++;
}

void SymbopTranslator::MakeCleanTrack(RiverInstruction *&rTrackOut, DWORD &trackCount) {
	rTrackOut->opCode = 0xC3; // mov eax, 0
	rTrackOut->subOpCode = 0;
	rTrackOut->modifiers = 0;
	rTrackOut->family = RIVER_FAMILY_TRACK;

	rTrackOut->opTypes[0] = RIVER_OPTYPE_IMM | RIVER_OPSIZE_8;
	rTrackOut->operands[0].asImm8 = trackedValues;

	rTrackOut->opTypes[1] = rTrackOut->opTypes[2] = rTrackOut->opTypes[3] = RIVER_OPTYPE_NONE;

	rTrackOut++;
	trackCount++;

}

void SymbopTranslator::MakeTrackFlg(RiverInstruction *&rMainOut, DWORD &instrCount, RiverInstruction *&rTrackOut, DWORD &trackCount) {
	trackedValues += 1;

	rMainOut->opCode = 0x9C;
	rMainOut->opTypes[0] = rMainOut->opTypes[1] = rMainOut->opTypes[2] = rMainOut->opTypes[3] = RIVER_OPTYPE_NONE;
	rMainOut->modifiers = 0;
	rMainOut->specifiers = 0;
	rMainOut->family = RIVER_FAMILY_PRETRACK;
	rMainOut++;
	instrCount++;


	rTrackOut->opCode = 0x9C; // pushf
	rTrackOut->opTypes[0] = rTrackOut->opTypes[1] = rTrackOut->opTypes[2] = rTrackOut->opTypes[3] = RIVER_OPTYPE_NONE;
	rTrackOut->subOpCode = 0;
	rTrackOut->modifiers = 0;
	rTrackOut->specifiers = 0;
	rTrackOut->family = RIVER_FAMILY_TRACK;

	rTrackOut++;
	trackCount++;
}

void SymbopTranslator::MakeMarkFlg(RiverInstruction *&rMainOut, DWORD &instrCount, RiverInstruction *&rTrackOut, DWORD &trackCount) {
	rTrackOut->opCode = 0x9D; // popf
	rTrackOut->opTypes[0] = rTrackOut->opTypes[1] = rTrackOut->opTypes[2] = rTrackOut->opTypes[3] = RIVER_OPTYPE_NONE;
	rTrackOut->subOpCode = 0;
	rTrackOut->modifiers = 0;
	rTrackOut->family = RIVER_FAMILY_TRACK;

	rTrackOut++;
	trackCount++;
}

void SymbopTranslator::MakeTrackReg(const RiverRegister &reg, RiverInstruction *&rMainOut, DWORD &instrCount, RiverInstruction *&rTrackOut, DWORD &trackCount) {
	trackedValues += 1;

	rMainOut->opCode = 0x50; // lea eax, [mem]
	rMainOut->modifiers = 0;
	rMainOut->family = RIVER_FAMILY_PRETRACK | ((RIVER_REG_xSP == (reg.name & 0x07)) ? RIVER_FAMILY_FLAG_ORIG_xSP : 0);
	rMainOut->opTypes[0] = RIVER_OPTYPE_REG;
	rMainOut->operands[0].asRegister.versioned = reg.versioned;
	rMainOut->opTypes[1] = rMainOut->opTypes[2] = rMainOut->opTypes[3] = RIVER_OPTYPE_NONE;
	rMainOut->TrackEspAsParameter();
	rMainOut->TrackUnusedRegisters();
	rMainOut++;
	instrCount++;

	rTrackOut->opCode = 0x50; // push + r
	rTrackOut->opTypes[0] = RIVER_OPTYPE_REG;
	rTrackOut->operands[0].asRegister.versioned = reg.versioned;
	rTrackOut->opTypes[1] = rTrackOut->opTypes[2] = rTrackOut->opTypes[3] = RIVER_OPTYPE_NONE;
	rTrackOut->modifiers = 0;
	rTrackOut->family = RIVER_FAMILY_TRACK | ((RIVER_REG_xSP == (reg.name & 0x07)) ? RIVER_FAMILY_FLAG_ORIG_xSP : 0);
	rTrackOut->TrackEspAsParameter();
	rTrackOut->TrackUnusedRegisters();
	rTrackOut++;
	trackCount++;
}

void SymbopTranslator::MakeMarkReg(const RiverRegister &reg, RiverInstruction *&rMainOut, DWORD &instrCount, RiverInstruction *&rTrackOut, DWORD &trackCount) {
	rTrackOut->opCode = 0x58; // pop + r

	rTrackOut->opTypes[0] = RIVER_OPTYPE_REG;
	rTrackOut->operands[0].asRegister.versioned = reg.versioned;

	rTrackOut->opTypes[1] = rTrackOut->opTypes[2] = rTrackOut->opTypes[3] = RIVER_OPTYPE_NONE;

	rTrackOut->modifiers = 0;
	rTrackOut->family = RIVER_FAMILY_TRACK | ((RIVER_REG_xSP == (reg.name & 0x07)) ? RIVER_FAMILY_FLAG_ORIG_xSP : 0);

	rTrackOut->TrackEspAsParameter();
	rTrackOut->TrackUnusedRegisters();
	
	rTrackOut++;
	trackCount++;
}


void SymbopTranslator::MakeTrackMem(const RiverAddress &mem, WORD specifiers, RiverInstruction *&rMainOut, DWORD &instrCount, RiverInstruction *&rTrackOut, DWORD &trackCount) {
	if (0 == mem.type) {
		MakeTrackReg(mem.base, rMainOut, instrCount, rTrackOut, trackCount);
		return;
	}

	if (0 == (specifiers & RIVER_SPEC_IGNORES_MEMORY)) {
		trackedValues += 1;

		rMainOut->opCode = 0xFF; // lea eax, [mem]
		rMainOut->specifiers = 0;
		rMainOut->subOpCode = 6;
		rMainOut->modifiers = 0;
		rMainOut->family = RIVER_FAMILY_PRETRACK;
		rMainOut->opTypes[0] = RIVER_OPTYPE_MEM;
		rMainOut->operands[0].asAddress = codegen->CloneAddress(mem, 0);
		rMainOut->opTypes[1] = rMainOut->opTypes[2] = rMainOut->opTypes[3] = RIVER_OPTYPE_NONE;
		rMainOut->PromoteModifiers();
		rMainOut->TrackEspAsParameter();
		rMainOut->TrackUnusedRegisters();
		rMainOut++;
		instrCount++;
	}

	trackedValues += 1;
	rMainOut->opCode = 0x8D;
	rMainOut->specifiers = 0;
	rMainOut->modifiers = 0;
	rMainOut->family = RIVER_FAMILY_PRETRACK;
	rMainOut->opTypes[0] = RIVER_OPTYPE_MEM;
	rMainOut->operands[0].asAddress = codegen->CloneAddress(mem, 0);
	rMainOut->opTypes[1] = rMainOut->opTypes[2] = rMainOut->opTypes[3] = RIVER_OPTYPE_NONE;
	rMainOut->PromoteModifiers();
	rMainOut->TrackEspAsParameter();
	rMainOut->TrackUnusedRegisters();
	rMainOut++;
	instrCount++;

	rTrackOut->opCode = 0xFF;
	rTrackOut->specifiers = 0;
	rTrackOut->subOpCode = 0x06;
	rTrackOut->modifiers = 0;
	rTrackOut->family = RIVER_FAMILY_TRACK;
	rTrackOut->opTypes[0] = RIVER_OPTYPE_MEM;
	rTrackOut->operands[0].asAddress = codegen->CloneAddress(mem, 0);
	rTrackOut->opTypes[1] = rTrackOut->opTypes[2] = rTrackOut->opTypes[3] = RIVER_OPTYPE_NONE;
	rTrackOut->TrackEspAsParameter();
	rTrackOut->TrackUnusedRegisters();
	rTrackOut++;
	trackCount++;
}

void SymbopTranslator::MakeMarkMem(const RiverAddress &mem, RiverInstruction *&rMainOut, DWORD &instrCount, RiverInstruction *&rTrackOut, DWORD &trackCount) {
	rTrackOut->opCode = 0x8F;
	rTrackOut->subOpCode = 0x00;
	rTrackOut->modifiers = 0;
	rTrackOut->family = RIVER_FAMILY_TRACK;
	rTrackOut->opTypes[0] = RIVER_OPTYPE_MEM;
	rTrackOut->operands[0].asAddress = codegen->CloneAddress(mem, 0);
	rTrackOut->opTypes[1] = rTrackOut->opTypes[2] = rTrackOut->opTypes[3] = RIVER_OPTYPE_NONE;
	rTrackOut->TrackEspAsParameter();
	rTrackOut->TrackUnusedRegisters();

	rTrackOut++;
	trackCount++;
}

void SymbopTranslator::MakeSkipMem(const RiverAddress &mem, RiverInstruction *&rMainOut, DWORD &instrCount, RiverInstruction *&rTrackOut, DWORD &trackCount) {
	rTrackOut->opCode = 0x8F;
	rTrackOut->subOpCode = 0x07;
	rTrackOut->modifiers = 0;
	rTrackOut->family = RIVER_FAMILY_TRACK;
	rTrackOut->opTypes[0] = RIVER_OPTYPE_MEM;
	rTrackOut->operands[0].asAddress = codegen->CloneAddress(mem, 0);
	rTrackOut->opTypes[1] = rTrackOut->opTypes[2] = rTrackOut->opTypes[3] = RIVER_OPTYPE_NONE;
	rTrackOut->TrackEspAsParameter();
	rTrackOut->TrackUnusedRegisters();

	rTrackOut++;
	trackCount++;
}

void SymbopTranslator::MakeTrackOp(const BYTE type, const RiverOperand &op, WORD specifiers, RiverInstruction *&rMainOut, DWORD &instrCount, RiverInstruction *&rTrackOut, DWORD &trackCount) {
	switch (RIVER_OPTYPE(type)) {
	case RIVER_OPTYPE_IMM :
		break;
	case RIVER_OPTYPE_REG :
		MakeTrackReg(op.asRegister, rMainOut, instrCount, rTrackOut, trackCount);
		break;
	case RIVER_OPTYPE_MEM:
		MakeTrackMem(*op.asAddress, specifiers, rMainOut, instrCount, rTrackOut, trackCount);
		break;
	}
}

void SymbopTranslator::MakeMarkOp(const BYTE type, const RiverOperand &op, RiverInstruction *&rMainOut, DWORD &instrCount, RiverInstruction *&rTrackOut, DWORD &trackCount) {
	switch (RIVER_OPTYPE(type)) {
	case RIVER_OPTYPE_IMM:
		break;
	case RIVER_OPTYPE_REG:
		MakeMarkReg(op.asRegister, rMainOut, instrCount, rTrackOut, trackCount);
		break;
	case RIVER_OPTYPE_MEM:
		MakeMarkMem(*op.asAddress, rMainOut, instrCount, rTrackOut, trackCount);
		break;
	}
}

void SymbopTranslator::TranslateUnk(const RiverInstruction &rIn, RiverInstruction *&rMainOut, DWORD &instrCount, RiverInstruction *&rTrackOut, DWORD &trackCount) {
	__asm int 3;
}

void SymbopTranslator::TranslateDefault(const RiverInstruction &rIn, RiverInstruction *&rMainOut, DWORD &instrCount, RiverInstruction *&rTrackOut, DWORD &trackCount) {
	MakeInitTrack(rTrackOut, trackCount);

	if (0 == (RIVER_SPEC_IGNORES_FLG & rIn.specifiers)) {
		MakeTrackFlg(rMainOut, instrCount, rTrackOut, trackCount);
	}

	for (int i = 3; i >= 0; --i) {
		if ((RIVER_OPTYPE_NONE != rIn.opTypes[i]) && (0 == (RIVER_SPEC_IGNORES_OP(i) & rIn.specifiers))) {
			MakeTrackOp(rIn.opTypes[i], rIn.operands[i], rIn.specifiers, rMainOut, instrCount, rTrackOut, trackCount);
		}
	}

	// make opcode
	CopyInstruction(*rMainOut, rIn);
	rMainOut++;
	instrCount++;
	

	if (RIVER_SPEC_MODIFIES_FLG & rIn.specifiers) {
		MakeMarkFlg(rMainOut, instrCount, rTrackOut, trackCount);
	}

	for (int i = 3; i >= 0; --i) {
		if ((RIVER_OPTYPE_NONE != rIn.opTypes[i]) && (RIVER_SPEC_MODIFIES_OP(i) & rIn.specifiers)) {
			MakeMarkOp(rIn.opTypes[i], rIn.operands[i], rMainOut, instrCount, rTrackOut, trackCount);
		}
	}

	MakeCleanTrack(rTrackOut, trackCount);
}






SymbopTranslator::TranslateOpcodeFunc SymbopTranslator::translateOpcodes[2][0x100] = {
	{
		/* 0x00 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x04 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x08 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x0C */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,

		/* 0x10 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x14 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x18 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x1C */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,

		/* 0x20 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x24 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x28 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x2C */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,

		/* 0x30 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x34 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x38 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x3C */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,

		/* 0x40 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x44 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x48 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x4C */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,

		/* not really default */
		/* 0x50 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x54 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x58 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x5C */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,

		/* 0x60 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x64 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x68 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x6C */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,

		/* 0x70 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x74 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x78 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x7C */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,

		/* 0x80 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x84 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x88 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x8C */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,

		/* 0x90 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x94 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x98 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x9C */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,

		/* 0xA0 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0xA4 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0xA8 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0xAC */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,

		/* 0xB0 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0xB4 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0xB8 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0xBC */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,

		/* 0xC0 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0xC4 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0xC8 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0xCC */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,

		/* 0xD0 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0xD4 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0xD8 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0xDC */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,

		/* 0xE0 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0xE4 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0xE8 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0xEC */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,

		/* 0xF0 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0xF4 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0xF8 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0xFC */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault
	}, {
		/* 0x00 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x04 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x08 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x0C */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,

		/* 0x10 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x14 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x18 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x1C */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,

		/* 0x20 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x24 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x28 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x2C */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,

		/* 0x30 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x34 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x38 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x3C */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,

		/* 0x40 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x44 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x48 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x4C */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,

		/* 0x50 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x54 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x58 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x5C */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,

		/* 0x60 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x64 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x68 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x6C */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,

		/* 0x70 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x74 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x78 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0x7C */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,

		/* 0x80 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x84 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x88 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x8C */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,

		/* 0x90 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x94 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x98 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0x9C */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,

		/* 0xA0 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateUnk,
		/* 0xA4 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0xA8 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0xAC */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateDefault,

		/* 0xB0 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0xB4 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,
		/* 0xB8 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateUnk,
		/* 0xBC */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault,

		/* 0xC0 */ &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateDefault, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0xC4 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateDefault,
		/* 0xC8 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0xCC */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,

		/* 0xD0 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0xD4 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0xD8 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0xDC */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,

		/* 0xE0 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0xE4 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0xE8 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0xEC */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,

		/* 0xF0 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0xF4 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0xF8 */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk,
		/* 0xFC */ &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk, &SymbopTranslator::TranslateUnk
	}
};