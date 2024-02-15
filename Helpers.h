/*
 * Helpers.h
 *
 *  Created on: Jan 8, 2024
 *      Author: sedo
 */

#ifndef HELPERS_H_
#define HELPERS_H_
#include <stdint.h>
#include <sys/types.h>
#include <vector>
#include <string>
#include <list>
#include <termios.h>



	class MailboxWrite
    {
    public:
		uint32_t command;
		uint32_t status;
		uint32_t bufferAddr;
		uint32_t bufferSize;
		uint32_t memoryOffset;
		uint32_t bytesWritten;
		const uint32_t length;

		MailboxWrite();
        MailboxWrite(uint8_t * bytes);
        void fromBytes(uint8_t * bytes);
        void getBytes(uint8_t * Buffer);
    };

    class MailboxInit
    {
    public:
    	uint32_t command;
    	uint32_t status;
    	uint32_t comType;
    	uint32_t traceLev;
    	uint32_t mode;
    	uint32_t crystalFreq;
    	uint32_t extCLK;
        const uint32_t length;

        MailboxInit();
        MailboxInit(uint8_t* bytes);
        void fromBytes(uint8_t * bytes);
        void getBytes(uint8_t * Buffer);
    };

    class MailboxRead
    {
    public:
    	uint32_t command;
    	uint32_t status;
    	uint32_t bufferAddr;
    	uint32_t bufferSize;
    	uint32_t memoryOffset;
    	uint32_t bytesRead;
        const uint32_t length;

        MailboxRead();
        MailboxRead(uint8_t* bytes);
        void fromBytes(uint8_t * bytes);
        void getBytes(uint8_t * Buffer);
    };

    class MailboxGPNVM
    {
    public:
    	uint32_t command;
    	uint32_t status;
    	uint32_t action;
    	uint32_t bitsOfNVM;
        const uint32_t length;

        MailboxGPNVM();
        MailboxGPNVM(uint8_t* bytes);
        void fromBytes(uint8_t * bytes);
        void getBytes(uint8_t * Buffer);
    };

	class LinuxSerialPort {
	public:
		static std::list<std::string> FindComPort();
		LinuxSerialPort(const std::string& port, speed_t baud);
		bool Open();
		void Close();
		bool IsOpen();
		ssize_t Write(const std::vector<uint8_t>& data);
		ssize_t Write(const uint8_t * data,int offset,int size);
		ssize_t WriteLine(std::string Line);
		ssize_t Read(std::vector<uint8_t>& buffer, size_t bytesToRead);
		bool SetReadBufferSize(int size);
		bool SetWriteBufferSize(int size) ;
		std::string ReadExisting();
		ssize_t BytesToRead();
	private:
		int fd; // File descriptor für die serielle Schnittstelle
		std::string portName;
		speed_t baudRate;
		int ReadBuffSize;

	};



#endif /* HELPERS_H_ */
