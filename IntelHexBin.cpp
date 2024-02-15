#include "IntelHexBin.h"
#include <iostream>
#include <fstream>
#include <string>



namespace std
{


    std::vector<uint8_t> IntelHexBin::MemoryMap() {
        return _memoryMap;
    }

    std::vector<uint8_t> IntelHexBin::GetBinaryData(uint32_t& StartAddr) {
        std::vector<uint8_t> arr(_endOffset - _startOffset);
        StartAddr = static_cast<uint32_t>(_startOffset);
        std::copy(_memoryMap.begin() + StartAddr, _memoryMap.begin() + StartAddr + arr.size(), arr.begin());
        return arr;
    }

    int IntelHexBin::UsedMemory() {
        return _endOffset - _startOffset + 1;
    }

    bool IntelHexBin::HasValidData() {
        return _hasValidData;
    }

    IntelHexBin::IntelHexBin(int bufferSize) {
        if (bufferSize <= 0) throw std::invalid_argument("Invalid HEX buffer size!");
        _memoryMap.resize(bufferSize);
        ResetMemoryMap();
    }

    void IntelHexBin::ResetMemoryMap(uint8_t value) {
        std::fill(_memoryMap.begin(), _memoryMap.end(), value);
        _startOffset = 0;
        _endOffset = 0;
        _hasValidData = false;
    }

    int IntelHexBin::Hex2Dec(std::string hexString) {
        int value = 0;
        if (hexString.length() == 0) throw std::invalid_argument("Unable to convert zero-length hex string to number!");
        if (hexString.length() > 8) throw std::out_of_range("hexString");
        for (uint32_t i = 0; i < hexString.length(); i++) {
            std::string hexDigit = hexString.substr(i, 1);
            value = (value << 4) | std::stoi(hexDigit, nullptr, 16);
        }
        return value;
    }

    std::string IntelHexBin::Dec2Hex(int value, uint32_t minWidth) {
        if (minWidth < 0 || minWidth > 8) throw std::out_of_range("minWidth");
        std::string hexString = std::to_string(value);
        while (hexString.length() < minWidth) {
            hexString = "0" + hexString;
        }
        return hexString;
    }

    EIntelHexResult IntelHexBin::ParseRecord(std::string hexLine, HexRecord& record) {
        if (hexLine.length() < 11)
            return EIntelHexResult::MissingFieldsInLine;
        if (hexLine[0] != ':')
            return EIntelHexResult::ColonExpected;
        try {
            record.Length = static_cast<uint8_t>(Hex2Dec(hexLine.substr(1, 2)));
            record.Offset = Hex2Dec(hexLine.substr(3, 4));
            record.Type = static_cast<uint8_t>(Hex2Dec(hexLine.substr(7, 2)));
        }
        catch (...) {
            return EIntelHexResult::HexConversionFailed;
        }
        if (hexLine.length() < 11 + ((uint32_t)record.Length) * 2)
            return EIntelHexResult::MissingFieldsInLine;
        int checksum = record.Length;
        checksum += static_cast<uint8_t>((record.Offset >> 8) & 0xFF);
        checksum += static_cast<uint8_t>(record.Offset & 0xFF);
        checksum += record.Type;
        if (record.Length > 0) {
            record.Data.resize(record.Length);
            for (int recordPos = 0; recordPos < record.Length; recordPos++) {
                record.Data[recordPos] = static_cast<uint8_t>(Hex2Dec(hexLine.substr(9 + recordPos * 2, 2)));
                checksum += record.Data[recordPos];
            }
        }
        checksum += static_cast<uint8_t>(Hex2Dec(hexLine.substr(9 + record.Length * 2, 2)));
        return ((uint8_t)checksum) != 0 ? EIntelHexResult::ChecksumError : EIntelHexResult::Success;
    }


    void IntelHexBin::WriteRecord(std::ofstream& streamWriter, HexRecord& record)
    {
        int recordPos;
        int checksum = record.Length;
        checksum += static_cast<int>((record.Offset >> 8) & 0xFF);
        checksum += static_cast<int>(record.Offset & 0xFF);
        checksum += record.Type;
        // write initial line
        streamWriter << ":" << Dec2Hex(record.Length, 2) << Dec2Hex(record.Offset, 4) << Dec2Hex(record.Type, 2);
        for (recordPos = 0; recordPos < record.Length; recordPos++)
        {
            checksum += record.Data[recordPos];
            streamWriter << Dec2Hex(record.Data[recordPos], 2);
        }
        // make two's complement of 256-modulo-checksum
        checksum = static_cast<int>(checksum != 0 ? static_cast<int>(256 - checksum) : 0);
        // and finally write the hexline
        streamWriter << Dec2Hex(checksum, 2) << std::endl;
    }


    EIntelHexResult IntelHexBin::LoadHexStream(std::ifstream& hexStream) {
        if (!hexStream) throw std::invalid_argument("hexStream");
        HexRecord rec;
        int32_t baseAddress = 0;
        ResetMemoryMap();
        _startOffset = static_cast<int>(_memoryMap.size());
        std::string hexline;
        while (std::getline(hexStream, hexline)) {
            EIntelHexResult parseResult = ParseRecord(hexline, rec);
            if (parseResult != EIntelHexResult::Success) {
                hexStream.close();
                return parseResult;
            }
            switch (rec.Type) {
                case 0x00:
                    if ((uint32_t)baseAddress + rec.Offset + rec.Length > _memoryMap.size()) {
                        hexStream.close();
                        return EIntelHexResult::DataOutsideMemoryMapDefined;
                    }
                    for (int dataPos = 0; dataPos < rec.Length; dataPos++) {
                        _memoryMap[baseAddress + rec.Offset + dataPos] = rec.Data[dataPos];
                    }
                    if (baseAddress + rec.Offset < _startOffset) {
                        _startOffset = baseAddress + rec.Offset;
                    }
                    if (baseAddress + rec.Offset + rec.Length - 1 > _endOffset) {
                        _endOffset = baseAddress + rec.Offset + rec.Length - 1;
                    }
                    break;
                case 0x02:
                    baseAddress = static_cast<int>(rec.Data[0] << 8) | rec.Data[1];
                    baseAddress <<= 4;
                    break;
                case 0x03:
                    break;
                case 0x04:
                    baseAddress = static_cast<int>(rec.Data[0] << 8) | rec.Data[1];
                    baseAddress <<= 16;
                    break;
                case 0x05:
                    break;
                case 0x01:
                    hexStream.close();
                    return EIntelHexResult::Success;
                default:
                    hexStream.close();
                    return EIntelHexResult::UnknownHexFormat;
            }
        }
        hexStream.close();
        return EIntelHexResult::UnknownHexFormat;
    }

    EIntelHexResult IntelHexBin::LoadHexFile(std::string path) {
        try {
            std::ifstream readFile(path);
            EIntelHexResult loadResult = LoadHexStream(readFile);
            if (loadResult == EIntelHexResult::Success) {
                _hasValidData = true;
            }
            return loadResult;
        }
        catch (...) {
            return EIntelHexResult::FileError;
        }
    }

    EIntelHexResult IntelHexBin::LoadHexFile(std::ifstream& stream) {
        try {
            EIntelHexResult loadResult = LoadHexStream(stream);
            if (loadResult == EIntelHexResult::Success) {
                _hasValidData = true;
            }
            return loadResult;
        }
        catch (...) {
            return EIntelHexResult::FileError;
        }
    }

    EIntelHexResult IntelHexBin::SaveHexFile(std::string path) {
        std::ofstream hexfile(path, std::ios::out | std::ios::binary);
        try {
            HexRecord rec;
            rec.Data.resize(16);
            int baseAddress = _startOffset;
            baseAddress &= ~0xFFFF;
            int offset = _startOffset & 0xFFFF;
            int dataPos = 0;
            rec.Length = 2;
            rec.Offset = 0;
            rec.Type = 0x02;
            rec.Data[1] = 0x00;
            rec.Data[0] = static_cast<uint8_t>((baseAddress >> 12));
            WriteRecord(hexfile, rec);
            do {
                if (offset + dataPos >= 0x10000) {
                    offset -= 0x10000;
                    baseAddress += 0x10000;
                    rec.Length = 2;
                    rec.Offset = 0;
                    rec.Type = 0x02;
                    rec.Data[0] = static_cast<uint8_t>(baseAddress >> 12);
                    rec.Data[1] = 0x00;
                    WriteRecord(hexfile, rec);
                }
                rec.Data[dataPos] = _memoryMap[baseAddress + offset + dataPos];
                dataPos++;
                if (offset + dataPos > 0x10000 || dataPos >= 16 || baseAddress + offset + dataPos > _endOffset) {
                    rec.Length = static_cast<uint8_t>(dataPos);
                    rec.Offset = offset;
                    rec.Type = 0x00;
                    WriteRecord(hexfile, rec);
                    offset += dataPos;
                    dataPos = 0;
                }
            } while (baseAddress + offset + dataPos <= _endOffset);
            rec.Length = 0;
            rec.Offset = 0;
            rec.Type = 0x01;
            WriteRecord(hexfile, rec);
            hexfile.close();
            return EIntelHexResult::Success;
        }
        catch (...) {
            return EIntelHexResult::UnknownHexFormat;
        }
    }

    EIntelHexResult IntelHexBin::LoadBinStream(std::ifstream& binaryReader) {
        try {
            ResetMemoryMap();
            binaryReader.read(reinterpret_cast<char*>(_memoryMap.data()), _memoryMap.size());
            _endOffset = static_cast<int>(_memoryMap.size() - 1);
            _hasValidData = true;
        }
        catch (...) {
            return EIntelHexResult::DataOutsideMemoryMapDefined;
        }
        return EIntelHexResult::Success;
    }

    EIntelHexResult IntelHexBin::LoadBinFile(std::string path) {
        try {
            std::ifstream readFile(path, std::ios::in | std::ios::binary);
            if (readFile.tellg() > _memoryMap.size()) {
                return EIntelHexResult::BinaryFileIsTooBig;
            }
            EIntelHexResult loadResult = LoadBinStream(readFile);
            if (loadResult == EIntelHexResult::Success) {
                _hasValidData = true;
            }
            return loadResult;
        }
        catch (...) {
            return EIntelHexResult::FileError;
        }
    }

    EIntelHexResult IntelHexBin::SaveBinFile(const std::string& path, bool writeUnusedMemory)
        {
            std::ofstream saveFile(path, std::ios::binary);
            if (!saveFile)
                return EIntelHexResult::FileError;
            if (saveFile.tellp() > static_cast<int>(_memoryMap.size()))
                return EIntelHexResult::BinaryFileIsTooBig;
            if (writeUnusedMemory)
                saveFile.write(reinterpret_cast<const char*>(_memoryMap.data()), _memoryMap.size());
            else
                saveFile.write(reinterpret_cast<const char*>(_memoryMap.data() + _startOffset), _endOffset - _startOffset + 1);
            return EIntelHexResult::Success;
        }
};


