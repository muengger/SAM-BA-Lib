/*
 * Helpers.cpp
 *
 *  Created on: Jan 8, 2024
 *      Author: sedo
 */
#include "Helpers.h"
#include <cstdlib>


#include <sys/ioctl.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <dirent.h>

//GLOBAL Funtions **************************************************************************************************************************
	uint32_t bytes_to_uint(uint8_t * Data){
		uint32_t temp = (((uint32_t)Data[0])<<0)|(((uint32_t)Data[1])<<8)|(((uint32_t)Data[2])<<16)|(((uint32_t)Data[3])<<24);
		return temp;

	}
	void uint_to_byteArr(uint32_t Number,uint8_t * Data){
		*Data = (uint8_t)((Number>>0) & 0xFF);
		Data++;
		*Data = (uint8_t)((Number>>8) & 0xFF);
		Data++;
		*Data = (uint8_t)((Number>>16) & 0xFF);
		Data++;
		*Data = (uint8_t)((Number>>24) & 0xFF);
		Data++;
		return ;
	}

//MailboxWrite Funtions **************************************************************************************************************************
	MailboxWrite::MailboxWrite():length(24){
		 command =0;
		 status=0;
		 bufferAddr=0;
		 bufferSize=0;
		 memoryOffset=0;
		 bytesWritten=0;
	}
	MailboxWrite::MailboxWrite(uint8_t * bytes):length(24)
	{

		if(bytes == NULL)
		{
			return ;
		}
		command = bytes_to_uint(&bytes[0]);
		status = bytes_to_uint(&bytes[4]);
		bufferAddr = bytes_to_uint(&bytes[8]);
		bufferSize = bytes_to_uint(&bytes[12]);
		memoryOffset = bytes_to_uint(&bytes[16]);
		bytesWritten = bytes_to_uint(&bytes[20]);
		return ;

	}
	 void MailboxWrite::fromBytes(uint8_t * bytes){
		command = bytes_to_uint(&bytes[0]);
		status = bytes_to_uint(&bytes[4]);
		bufferAddr = bytes_to_uint(&bytes[8]);
		bufferSize = bytes_to_uint(&bytes[12]);
		memoryOffset = bytes_to_uint(&bytes[16]);
		bytesWritten = bytes_to_uint(&bytes[20]);
	 }
	void MailboxWrite::getBytes(uint8_t * Buffer)
	{
		if(Buffer == NULL)
		{
			return ;
		}
		uint_to_byteArr(command,&Buffer[0]);
		uint_to_byteArr(status,&Buffer[4]);
		uint_to_byteArr(bufferAddr,&Buffer[8]);
		uint_to_byteArr(bufferSize,&Buffer[12]);
		uint_to_byteArr(memoryOffset,&Buffer[16]);
		uint_to_byteArr(bytesWritten,&Buffer[20]);

	}

	//MailboxInit Funtions **************************************************************************************************************************
	MailboxInit::MailboxInit():length(28){
		command=0;
		status=0;
		comType=0;
		traceLev=0;
		mode=0;
		crystalFreq=0;
		extCLK=0;
	}
	MailboxInit::MailboxInit(uint8_t* bytes):length(28)
	{   if(bytes == NULL)
		{
			return ;
		}

		command    	= bytes_to_uint(&bytes[0]);
		status     	= bytes_to_uint(&bytes[4]);
		comType    	= bytes_to_uint(&bytes[8]);
		traceLev   	= bytes_to_uint(&bytes[12]);
		mode 		= bytes_to_uint(&bytes[16]);
		crystalFreq = bytes_to_uint(&bytes[20]);
		extCLK	 	= bytes_to_uint(&bytes[24]);

	}
	void MailboxInit::fromBytes(uint8_t * bytes){
		command    	= bytes_to_uint(&bytes[0]);
		status     	= bytes_to_uint(&bytes[4]);
		comType    	= bytes_to_uint(&bytes[8]);
		traceLev   	= bytes_to_uint(&bytes[12]);
		mode 		= bytes_to_uint(&bytes[16]);
		crystalFreq = bytes_to_uint(&bytes[20]);
		extCLK	 	= bytes_to_uint(&bytes[24]);
	}
	void MailboxInit::getBytes(uint8_t * Buffer)
	{

		uint_to_byteArr(command,&Buffer[0]);
		uint_to_byteArr(status,&Buffer[4]);
		uint_to_byteArr(comType,&Buffer[8]);
		uint_to_byteArr(traceLev,&Buffer[12]);
		uint_to_byteArr(mode,&Buffer[16]);
		uint_to_byteArr(crystalFreq,&Buffer[20]);
		uint_to_byteArr(extCLK,&Buffer[20]);
	}

	//MailboxRead Funtions **************************************************************************************************************************

	MailboxRead::MailboxRead():length(24){
		 command=0;
		 status=0;
		 bufferAddr=0;
		 bufferSize=0;
		 memoryOffset=0;
		 bytesRead=0;
	}
	MailboxRead::MailboxRead(uint8_t* bytes):length(24)
	{
		if (bytes == NULL)
		{
			return ;
		}

		command    	= bytes_to_uint(&bytes[0]);
		status     	= bytes_to_uint(&bytes[4]);
		bufferAddr  = bytes_to_uint(&bytes[8]);
		bufferSize 	= bytes_to_uint(&bytes[12]);
		memoryOffset= bytes_to_uint(&bytes[16]);
		bytesRead 	= bytes_to_uint(&bytes[20]);

	}
	void MailboxRead::fromBytes(uint8_t * bytes){
		command    	= bytes_to_uint(&bytes[0]);
		status     	= bytes_to_uint(&bytes[4]);
		bufferAddr  = bytes_to_uint(&bytes[8]);
		bufferSize 	= bytes_to_uint(&bytes[12]);
		memoryOffset= bytes_to_uint(&bytes[16]);
		bytesRead 	= bytes_to_uint(&bytes[20]);
	}
	void MailboxRead::getBytes(uint8_t * Buffer)
	{

		uint_to_byteArr(command,&Buffer[0]);
		uint_to_byteArr(status,&Buffer[4]);
		uint_to_byteArr(bufferAddr,&Buffer[8]);
		uint_to_byteArr(bufferSize,&Buffer[12]);
		uint_to_byteArr(memoryOffset,&Buffer[16]);
		uint_to_byteArr(bytesRead,&Buffer[20]);
	}

    //MailboxGPNVM Funtions **************************************************************************************************************************

	MailboxGPNVM::MailboxGPNVM():length(16){
		 command=0;
		 status=0;
		 action=0;
		 bitsOfNVM=0;
	}

	MailboxGPNVM::MailboxGPNVM(uint8_t* bytes):length(16)
	{
		if (bytes == NULL)
		{
			return;
		}

		command = bytes_to_uint(&bytes[0]);
		status  = bytes_to_uint(&bytes[4]);
		action 	= bytes_to_uint(&bytes[8]);
		bitsOfNVM= bytes_to_uint(&bytes[12]);

	}
	void MailboxGPNVM::fromBytes(uint8_t * bytes){
		command = bytes_to_uint(&bytes[0]);
		status  = bytes_to_uint(&bytes[4]);
		action 	= bytes_to_uint(&bytes[8]);
		bitsOfNVM= bytes_to_uint(&bytes[12]);
	}
	void MailboxGPNVM::getBytes(uint8_t * Buffer)
	{
		uint_to_byteArr(command,&Buffer[0]);
		uint_to_byteArr(status,&Buffer[4]);
		uint_to_byteArr(action,&Buffer[8]);
		uint_to_byteArr(bitsOfNVM,&Buffer[12]);
	}


    //LinuxSerialPort Funtions **************************************************************************************************************************

    LinuxSerialPort::LinuxSerialPort(const std::string& port, speed_t baud) : portName(port), baudRate(baud),ReadBuffSize(0) {
    	fd = -1;
    }


    std::list<std::string> LinuxSerialPort::FindComPort() {
        std::list<std::string> portNames;
        DIR *dir;
        struct dirent *ent;

        if ((dir = opendir("/dev")) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                std::string filename = ent->d_name;
                if (filename.find("ttyGateBoot6124") != std::string::npos) {
                	portNames.push_back("/dev/"+filename);
                }
            }
            closedir(dir);
        } else {
            std::cerr << "Error opening directory" << std::endl;
        }

        //portNames.push_back("/dev/ttyV1");
        return portNames;
    }


    bool LinuxSerialPort::Open() {
        fd = open(portName.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
        if (fd == -1) {
            perror("Fehler beim Öffnen der seriellen Schnittstelle");
            return false;
        }

        //fcntl(fd, F_SETFL, 0); // Nicht blockierend

        struct termios tty;

		memset(&tty, 0, sizeof(tty));

		if (tcgetattr(fd, &tty) != 0) {
			perror("error from tcgetattr: ");
			return false;
		}


			cfsetospeed(&tty, baudRate);
			cfsetispeed(&tty, baudRate);


		tty.c_cflag &= ~(CSIZE | PARENB | PARODD | CSTOPB | CRTSCTS);
		tty.c_cflag |= CS8 | CLOCAL | CREAD;
		tty.c_lflag = 0;
		tty.c_oflag = 0;
		tty.c_cc[VMIN] = 1;
		tty.c_cc[VTIME] = 5;
		tty.c_iflag &= ~(ICRNL | IGNBRK | IXON | IXOFF | IXANY);

		if (tcsetattr(fd, TCSANOW, &tty) != 0) {
			perror("error from tcsetattr: ");
			return false;
		}

        return true;
    }

    void LinuxSerialPort::Close() {
        close(fd);
        fd = -1;
    }

    bool LinuxSerialPort::IsOpen() {
    	if(fd != -1){
    		return true;
    	}
    	return false;
    }

    ssize_t LinuxSerialPort::Write(const std::vector<uint8_t>& data) {
        ssize_t bytesWritten = write(fd, data.data(), data.size());
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        if (bytesWritten < 0) {
            perror("Fehler beim Schreiben in die serielle Schnittstelle");
        }
        return bytesWritten;
    }
    ssize_t LinuxSerialPort::Write(const uint8_t * data,int offset,int size) {

        ssize_t bytesWritten = write(fd, data + offset, size);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        if (bytesWritten < 0) {
            perror("Fehler beim Schreiben in die serielle Schnittstelle");
        }
        return bytesWritten;
    }
    ssize_t LinuxSerialPort::WriteLine(std::string Line){
    	std::vector<uint8_t> data(Line.begin(), Line.end());
    	return Write(data);
    }
    ssize_t LinuxSerialPort::Read(std::vector<uint8_t>& buffer, size_t bytesToRead) {
    	uint8_t Buff[bytesToRead];
    	size_t bytesRead = 0;

		bytesRead = read(fd, Buff, bytesToRead);
		if (bytesRead < 0) {
			perror("Fehler beim Lesen von der seriellen Schnittstelle");
		}
		buffer.insert(buffer.end(),Buff,Buff + bytesRead);

        return bytesRead;
    }
    bool LinuxSerialPort::SetReadBufferSize(int size) {
        struct termios options;
        if (tcgetattr(fd, &options) == -1) {
            perror("Fehler beim Lesen der seriellen Schnittstellenattribute");
            return false;
        }
        options.c_cc[VMIN] = size; // Minimale Anzahl von Zeichen zum Lesen
        if (tcsetattr(fd, TCSANOW, &options) == -1) {
            perror("Fehler beim Setzen der seriellen Schnittstellenattribute");
            return false;
        }
        return true;
    }

    bool LinuxSerialPort::SetWriteBufferSize(int size) {
        // Dieser Ansatz setzt das Schreibverhalten nicht direkt, da es in der seriellen Kommunikation
        // keine Puffergröße im Sinne einer Datei oder eines Sockets gibt.
        // Aber Sie können es für die Zukunft implementieren, falls erforderlich.
        return true;
    }
    std::string LinuxSerialPort::ReadExisting() {

    	size_t sizeinbuff = BytesToRead();
    	char Buff[sizeinbuff+1];
    	memset(Buff,0,sizeinbuff+1);
    	size_t bytesRead = 0;

    	while(sizeinbuff != bytesRead){
			bytesRead += read(fd, Buff + bytesRead, sizeinbuff);

    	}

    	std::string Res(Buff);
        return Res;
    }
    ssize_t LinuxSerialPort::BytesToRead(){
        int bytes_avail = 0;
        if(ioctl(fd, FIONREAD, &bytes_avail)<0){
        	return 0;
        }
        return bytes_avail;
    }



