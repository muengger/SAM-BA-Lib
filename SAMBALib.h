/*
 * BarcodeRFIDInftLib.h
 *
 *  Created on: Jul 15, 2022
 *      Author: sedo
 */

#ifndef SAMBALIB_H_
#define SAMBALIB_H_
#include <string>
#include <semaphore.h>
#include <vector>
#include "Helpers.h"

namespace std {


	class SAM_BA_Functions
	{
		public:
			SAM_BA_Functions(bool * Inited);
			virtual ~SAM_BA_Functions();
			bool ProgrammUC(string pathtofile);
			int  getProgress();

		private:

			bool InitSAMBA();
			bool Close();
			bool RunLowLevelInit();
			bool RunLowLevelInit(MailboxInit * InitMailb,bool CheckAnswer);
			bool WriteBinary(string pathtofile, int OffsetAddr);
	        bool SetGPNVMBitToFlashBoot();
	        bool RestartDevice();
	        LinuxSerialPort * OpenComport(string PortName);
	        bool LoadInitFlashWriteApplet();
	        bool RunFlashWriteApplet(MailboxWrite * MailbWr);
	        bool RunFlashReadApplet(MailboxRead * MailbRd);
	        bool RunFlashWriteGPNVM(MailboxGPNVM * MailbGP);
	        bool WriteSector(vector<uint8_t>  Data, uint32_t Address);
	        vector<uint8_t> * ReadSector(uint32_t Address);
	        bool loadFlashApplet(const uint8_t * Data, uint32_t Size);
	        bool ReadDataFromDevice(uint32_t Address, uint32_t leng,uint8_t * buffer);
	        bool WriteDataToDevice(uint32_t Address, uint32_t leng,const uint8_t * buffer);
	        bool WriteJumpAndEx(uint32_t Address);


			sem_t NoParalelSem;
			bool CInited;
			LinuxSerialPort * serPort;
		    double WriteProgress;
			int FlashStartAddr;
			int AppletAddr ;
			int ApplateMailbox ;
			const uint PageSize;
			const uint SectorSize;
	};



} /* namespace std */
#endif /* SAMBALIB_H_ */
