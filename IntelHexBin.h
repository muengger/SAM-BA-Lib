/*
 * IntelHexBin.h
 *
 *  Created on: Jan 9, 2024
 *      Author: sedo
 */

#ifndef INTELHEXBIN_H_
#define INTELHEXBIN_H_

#include <stdint.h>
#include <string>
#include <vector>
namespace std
{

	enum EIntelHexResult {
		Success,
		MissingFieldsInLine,
		ColonExpected,
		HexConversionFailed,
		ChecksumError,
		DataOutsideMemoryMapDefined,
		UnknownHexFormat,
		FileError,
		BinaryFileIsTooBig
	};

	struct HexRecord {
		uint8_t Length;
		int Offset;
		uint8_t Type;
		vector<uint8_t> Data;
	};
	class IntelHexBin {
	private:
	    vector<uint8_t> _memoryMap;
	    int32_t _startOffset;
	    int32_t _endOffset;
	    bool _hasValidData;
	public:

		IntelHexBin(int bufferSize);

		vector<uint8_t> 	MemoryMap();
		vector<uint8_t> 	GetBinaryData(uint32_t& StartAddr);
		int 					UsedMemory();
		bool 					HasValidData();
		void 					ResetMemoryMap(uint8_t value = 0x00);
		static int 				Hex2Dec(std::string hexString);
		static std::string 		Dec2Hex(int value, uint32_t minWidth) ;
		static EIntelHexResult 	ParseRecord(std::string hexLine, HexRecord& record) ;
		void 					WriteRecord(std::ofstream& streamWriter, HexRecord& record);
		EIntelHexResult 		LoadHexStream(std::ifstream& hexStream);
		EIntelHexResult 		LoadHexFile(std::string path);
		EIntelHexResult 		LoadHexFile(std::ifstream& stream);
		EIntelHexResult 		SaveHexFile(std::string path);
		EIntelHexResult 		LoadBinStream(std::ifstream& binaryReader) ;
		EIntelHexResult 		LoadBinFile(std::string path);
		EIntelHexResult 		SaveBinFile(const std::string& path, bool writeUnusedMemory);

	};
}
#endif /* INTELHEXBIN_H_ */
