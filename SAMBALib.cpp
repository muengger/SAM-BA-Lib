/*
 * BarcodeRFIDInffLib.cpp
 *
 *  Created on: Jul 15, 2022
 *      Author: sedo
 */



#include "SAMBALib.h"
#include "IntelHexBin.h"
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions*/
#include <unistd.h>  /* Linuxstandard function definitions*/
#include <fcntl.h>   /* File control definitions*/
#include <errno.h>   /* Error number definitions*/
#include <termios.h> /* POSIX terminal control definitions*/
#include <fstream>
#include <chrono>
#include <thread>
#include "Applet.h"

namespace std {



        SAM_BA_Functions::SAM_BA_Functions( bool * Inited):PageSize(512),SectorSize(128 * PageSize )
        {
        	WriteProgress = 0;
			FlashStartAddr = 0x00400000;
			AppletAddr = 0x20000800;
			ApplateMailbox = 0x20000840;
        	if(sem_init(&NoParalelSem, 0, 1)<0){
        		return;
        	}
            *Inited = InitSAMBA();
            CInited = *Inited;

        }
        SAM_BA_Functions::~SAM_BA_Functions() {
        	Close();
        }
        bool SAM_BA_Functions::InitSAMBA()
        {
        	sem_wait(&NoParalelSem);
            if (CInited)
            {
                CInited = false;
                serPort->Close();
            }
            list<string> portName = LinuxSerialPort::FindComPort();
            if (portName.size()>0)
            {
            	auto it = portName.begin();
            	serPort = OpenComport(*it);
            	if(serPort != NULL){
            	//serPort = new LinuxSerialPort(*it,O_RDWR | O_NOCTTY);
					if (serPort->IsOpen())
					{

						sem_post(&NoParalelSem);
						CInited = true;
						return true;
					}
            	}

            }

            sem_post(&NoParalelSem);
            return false;
        }

        bool SAM_BA_Functions::Close()
        {
        	sem_wait(&NoParalelSem);
            if (CInited == false)
            {
            	sem_post(&NoParalelSem);
                return true;
            }
            serPort->Close();
            sem_post(&NoParalelSem);
            return true;
        }
        bool SAM_BA_Functions::ProgrammUC(string pathtofile){
        	bool res;
        	if(CInited == false){
        		return false;
        	}
        	res = RunLowLevelInit();
        	if(res == false){
        		printf("ProgrammUC Unable to RunLowLevelInit\n");
        		return false;
        	}
        	for(int i = 0;i<3;i++){
            	res = WriteBinary(pathtofile,0);
            	if(res == false){
            		printf("ProgrammUC Unable to WriteBinary Chance %d\n",i);
            	}else{
            		break;
            	}
        	}
        	if(res == false){
        		printf("ProgrammUC Unable to WriteBinary\n");
        	}
        	res = SetGPNVMBitToFlashBoot();
        	if(res == false){
        		printf("ProgrammUC Unable SetGPNVMBitToFlashBoot\n");
        		return false;
        	}
        	res = RestartDevice();
        	if(res == false){
        		printf("ProgrammUC Unable RestartDevice\n");
        		return false;
        	}
        	return true;
        }
        bool SAM_BA_Functions::RunLowLevelInit()
        {
            MailboxInit InitMailb;// = new MailboxInit();
            InitMailb.command = 0;
            InitMailb.mode = 0;
            return RunLowLevelInit(&InitMailb , true);

        }

        bool SAM_BA_Functions::RunLowLevelInit(MailboxInit * InitMailb,bool CheckAnswer)
        {
        	uint8_t  Buffer[InitMailb->length];
            uint32_t command = InitMailb->command;
            sem_wait(&NoParalelSem);
            if (loadFlashApplet(applet_lowlevelinit_sam4e8,sizeof(applet_lowlevelinit_sam4e8)))
            {
                //write to message box

            	InitMailb->getBytes(Buffer);
                if (WriteDataToDevice(0x20000840, InitMailb->length, Buffer ))
                {
                    if (WriteJumpAndEx(0x20000800))
                    {
                        while (CheckAnswer)
                        {

                            ReadDataFromDevice(0x20000840, InitMailb->length,Buffer);
                            InitMailb->fromBytes(Buffer);

                            if (InitMailb->command == ~command)
                            {
                                if (InitMailb->status == 0)
                                {
                                	sem_post(&NoParalelSem);
                                    return true;
                                }
                                else
                                {
                                	sem_post(&NoParalelSem);
                                    printf("RunLowLevelInit : command unnown");
                                    return false;
                                }
                            }
                        }
                    }
                    else
                    {
                    	printf("RunLowLevelInit : WriteJumpAndEx Fail");
                    }
                }
                else
                {
                	printf("RunLowLevelInit : Unable to write Data to device");
                }


            }
            else
            {
            	printf("RunLowLevelInit : Unable to load lowlevelinit applet");
            }
            sem_post(&NoParalelSem);
            return false;
        }

        bool SAM_BA_Functions::WriteBinary(string pathtofile, int OffsetAddr)
        {
            if(CInited == false)
            {
            	printf("WriteBinary : CInited == false Fail");
                return false;
            }

            sem_wait(&NoParalelSem);
            WriteProgress = 0;
            FILE * file =fopen(pathtofile.c_str(),"r");

            if (file == NULL)
            {
                sem_post(&NoParalelSem);
                printf("WriteBinary : fi.Exists == false Fail");
                return false;
            }
            //Here file exits now we parse the file to binary byte array
            IntelHexBin  intelHexBin(0x500000);// = new IntelHexBin(0x500000); //5Mb
            if (pathtofile.find(".bin")!=string::npos)
            {
                if (intelHexBin.LoadBinFile(pathtofile) != Success)
                {
                    sem_post(&NoParalelSem);
                    printf("WriteBinary : LoadBinFile Fail");
                    return false;
                }
            }
            else if(pathtofile.find(".hex")!=string::npos)
            {
                if (intelHexBin.LoadHexFile(pathtofile) != Success)
                {
                    sem_post(&NoParalelSem);
                    printf("WriteBinary : LoadHexFile Fail");
                    return false;
                }
            }else{
                sem_post(&NoParalelSem);
                printf("Wrong File ending");
                return false;
            }

            uint StartAddr = 0;

            vector<uint8_t> BinaryData = intelHexBin.GetBinaryData(StartAddr);
            uint Size = BinaryData.size();
            if (Size % SectorSize > 0)
            {
                Size += SectorSize - (Size % SectorSize);
            }


            uint AnzSector = Size / SectorSize;
            double stepperpage = (100 / (AnzSector-1)) / 2;
            uint writeBlockSize;

            writeBlockSize = SectorSize;

            vector<uint8_t> *SectorData  = new vector<uint8_t>(writeBlockSize);


            uint offset = 0;
            if (LoadInitFlashWriteApplet() == false)
            {
                sem_post(&NoParalelSem);
                printf("WriteBinary : LoadInitFlashWriteApplet() == false Fail");
                return false;
            }
            //Write Data to Device
            while (true)
            {


                if (offset + writeBlockSize <= BinaryData.size())
                {
                    //Array.Copy(BinaryData, offset, SectorData, 0, writeBlockSize);
                    copy(BinaryData.begin()+offset, BinaryData.begin()+offset + writeBlockSize, SectorData->begin());

                }
                else
                {
                    //Array.Clear(SectorData, 0, SectorData.Length);
                    fill(SectorData->begin(), SectorData->end(), 0);
                    copy(BinaryData.begin()+offset ,BinaryData.begin() +BinaryData.size(), SectorData->begin());
                    //Array.Copy(BinaryData, offset, SectorData, 0, BinaryData.Length - offset);
                }
                if (WriteSector(*SectorData, offset + OffsetAddr) == false)
                {
                    sem_post(&NoParalelSem);
                    printf("WriteBinary : WriteSector == false Fail");
                    return false;
                }
                offset += writeBlockSize;
                WriteProgress += stepperpage;
                if (offset > Size - writeBlockSize)
                {
                    break;
                }
            }
            //Read back to verify
            offset = 0;
            uint SectorToCheck = AnzSector;
            while (true)
            {
                uint res = writeBlockSize;
                if (offset + writeBlockSize <= BinaryData.size())
                {
                    //Array.Copy(BinaryData, offset, SectorData, 0, writeBlockSize);
                	//copy(von,bis,ziel)
                    copy(BinaryData.begin() + offset, BinaryData.begin() + offset + writeBlockSize, SectorData->begin());
                }
                else
                {
                    fill(SectorData->begin(), SectorData->end(), 0);
                    copy(BinaryData.begin() + offset,BinaryData.end(),SectorData->begin() );

                }
                vector<uint8_t> * rdata = ReadSector(offset + OffsetAddr);
                if (rdata->size() != writeBlockSize)
                {
                    sem_post(&NoParalelSem);
                    printf("WriteBinary : rdata.Length != System.Convert.Touint32_t(writeBlockSize) Fail");
                    return false;
                }
                WriteProgress += stepperpage;
                offset += writeBlockSize;


                for (uint32_t i = 0; i < res; i++)
                {
                    if ((*SectorData)[i] != (*rdata)[i])
                    {
                        sem_post(&NoParalelSem);
                        printf("WriteBinary : SectorData[%d]:=%d != rdata[%d]:=%d Fail",i,(*SectorData)[i],i,(*rdata)[i]);
                        return false;
                    }

                }
                delete(rdata);
                SectorToCheck--;
                if (SectorToCheck == 0)
                {
                    sem_post(&NoParalelSem);
                    return true;
                }


            }
            delete(SectorData);
            sem_post(&NoParalelSem);
            return false;
        }

        int SAM_BA_Functions::getProgress()
        {
            return WriteProgress;
        }

        bool SAM_BA_Functions::SetGPNVMBitToFlashBoot()
        {
        	sem_wait(&NoParalelSem);
            MailboxGPNVM mailb;// = new MailboxGPNVM();
            mailb.status = 0;
            mailb.action = 1; //0 clear bit 1 set bit
            mailb.bitsOfNVM = 1; //Bit index to set or reset
            mailb.command = 6; //command for GPNVM
            if(RunFlashWriteGPNVM(&mailb))
            {

                sem_post(&NoParalelSem);

                return true;
            }
            sem_post(&NoParalelSem);
            printf("SetGPNVMBitToFlashBoot : RunFlashWriteGPNVM Fail");
            return false;
        }

        bool SAM_BA_Functions::RestartDevice()
        {
            MailboxInit mailbInit;// = new MailboxInit();
            mailbInit.command = 0x15;
            RunLowLevelInit(&mailbInit,false);
            sleep(1);
            if (serPort->IsOpen())
            {
                try
                {
                    serPort->Close();

                }
                catch (...){ }
            }
            return true;
        }

        LinuxSerialPort * SAM_BA_Functions::OpenComport(string PortName)
        {
        	LinuxSerialPort* Comport;

            Comport = new LinuxSerialPort(PortName, 115200);

            try
            {
                Comport->Open();
                //Comport->SetReadBufferSize(SectorSize + 100);
                //Comport->SetWriteBufferSize(SectorSize + 100);

            }
            catch (...){
                printf("OpenComport :  Comport.Open() Fail");
                return NULL;
            }
            //Comport->SetReadBufferSize(SectorSize + 100);
            //Comport->SetWriteBufferSize(SectorSize + 100);
            vector<uint8_t> Command = { 'T', '#'};
            Comport->Write(Command);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            string res = Comport->ReadExisting();
            if (res.find("\n") !=string::npos)
            {
                return Comport;
            }
            else
            {
                Comport->Close();
                printf("OpenComport :  answer incorrect res:=%s\n",res.c_str());
                return NULL;
            }
        }


        bool SAM_BA_Functions::LoadInitFlashWriteApplet()
        {
            MailboxWrite MailBWr;// = new MailboxWrite();
            MailBWr.command = 0; //Init

            if (loadFlashApplet(applet_flash_sam4e8,sizeof(applet_flash_sam4e8)))
            {
            	uint8_t Buffer[MailBWr.length];
            	MailBWr.getBytes(Buffer);
                if (WriteDataToDevice(0x20000840, MailBWr.length, Buffer))
                {
                    if (WriteJumpAndEx(0x20000800))
                    {
                        while (true)
                        {
                            uint32_t command = MailBWr.command;
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));

                            ReadDataFromDevice(0x20000840, MailBWr.length,Buffer);
                            MailBWr.fromBytes(Buffer);
                            if (MailBWr.command == ~command)
                            {
                                if (MailBWr.status == 0)
                                {
                                    return true;
                                }
                                else
                                {
                                    printf("LoadInitFlashWriteApplet :  (MailBWr.status == 0) Fail");
                                    return false;
                                }
                            }
                            else
                            {
                                printf("LoadInitFlashWriteApplet :  (MailBWr.command == ~command Fail");
                            }
                        }
                    }
                    else
                    {
                        printf("LoadInitFlashWriteApplet :  WriteJumpAndEx Fail");
                    }
                }
                else
                {
                    printf("LoadInitFlashWriteApplet :  WriteDataToDevice Fail");
                }

            }
            else
            {
                printf("LoadInitFlashWriteApplet :  loadFlashApplet Fail");
            }
            return false;
        }

        bool SAM_BA_Functions::RunFlashWriteApplet(MailboxWrite * MailbWr)
        {
        	uint8_t Buffer[MailbWr->length];
            //write to message box
        	MailbWr->getBytes(Buffer);
            if (WriteDataToDevice(0x20000840, MailbWr->length, Buffer))
            {
                if (WriteJumpAndEx(0x20000800))
                {
                    while (true)
                    {
                        uint32_t command = MailbWr->command;
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        ReadDataFromDevice(0x20000840, MailbWr->length,Buffer);
                        MailbWr->fromBytes(Buffer);

                        if (MailbWr->command == ~command)
                        {
                            if (MailbWr->status == 0)
                            {
                                return true;
                            }
                            else
                            {
                                printf("RunFlashWriteApplet :  MailbWr.status == 0 Fail");
                                return false;
                            }
                        }
                        else
                        {
                            printf("RunFlashWriteApplet :  MailbWr.command == ~command Fail");
                            return false;
                        }
                    }
                }
                else
                {
                    printf("RunFlashWriteApplet :  WriteJumpAndEx Fail");
                }
            }
            else
            {
                printf("RunFlashWriteApplet :  WriteDataToDevice Fail");
            }
            return false;
        }

        bool SAM_BA_Functions::RunFlashReadApplet(MailboxRead * MailbRd)
        {
        	uint8_t Buffer[MailbRd->length];
        	MailbRd->getBytes(Buffer);
            //write to message box
            if (WriteDataToDevice(0x20000840, MailbRd->length, Buffer))
            {
                if (WriteJumpAndEx(0x20000800))
                {
                    while (true)
                    {
                        uint32_t command = MailbRd->command;
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        ReadDataFromDevice(0x20000840, MailbRd->length,Buffer);
                        MailbRd->fromBytes(Buffer);

                        if (MailbRd->command == ~command)
                        {
                            if (MailbRd->status == 0)
                            {
                                return true;
                            }
                            else
                            {
                                return false;
                            }
                        }
                        else
                        {
                            printf("RunFlashReadApplet :  MailbRd.command == ~command Fail");
                        }
                    }
                }
                else
                {
                    printf("RunFlashReadApplet :  WriteJumpAndEx Fail");
                }
            }
            else
            {
                printf("RunFlashReadApplet :  WriteDataToDevice Fail");
            }
            return false;
        }

        bool SAM_BA_Functions::RunFlashWriteGPNVM(MailboxGPNVM * MailbGP)
        {
        	uint8_t Buffer[MailbGP->length];
        	MailbGP->getBytes(Buffer);
            //write to message box
            if (WriteDataToDevice(0x20000840, MailbGP->length, Buffer))
            {
                if (WriteJumpAndEx(0x20000800))
                {
                    while (true)
                    {
                        uint32_t command = MailbGP->command;
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        ReadDataFromDevice(0x20000840, MailbGP->length,Buffer);
                        MailbGP->fromBytes(Buffer);

                        if (MailbGP->command == ~command)
                        {
                            if (MailbGP->status == 0)
                            {
                                return true;
                            }
                            else
                            {
                                printf("RunFlashWriteGPNVM :  MailbGP.status == 0 Fail");
                                return false;
                            }
                        }
                        else
                        {
                            printf("RunFlashWriteGPNVM :  MailbGP.command == ~command Fail");
                            return false;
                        }
                    }
                }
                else
                {
                    printf("RunFlashWriteGPNVM :  WriteJumpAndEx Fail");
                }
            }
            else
            {
                printf("RunFlashWriteGPNVM :  WriteDataToDevice Fail");
            }
            return false;
        }

        bool SAM_BA_Functions::WriteSector(vector<uint8_t>  Data, uint32_t Address)
        {
            uint32_t PageAddress = 0x2000E000;
            MailboxWrite MailBWr;// =MailBWr.w MailboxWrite();
            if(WriteDataToDevice(PageAddress, Data.size(),&Data[0])== false)
            {
                printf("WriteSector :  WriteDataToDevice Fail");
                return false;
            }
            MailBWr.memoryOffset = Address;
            MailBWr.bufferAddr = PageAddress;
            MailBWr.bufferSize = Data.size();
            MailBWr.command = 2;
            if (RunFlashWriteApplet(&MailBWr) == false)
            {
                printf("WriteSector :  RunFlashWriteApplet Fail");
                return false;
            }
            return true;
        }

        vector<uint8_t> * SAM_BA_Functions::ReadSector(uint32_t Address)
        {
            uint32_t tranche = 0x800;
            uint32_t PageAddress = 0x2000E000;
            MailboxRead  MailBRd;// MailBRd.ew MailboxRead();
            Address -= Address % SectorSize;
            uint32_t Anztransm = SectorSize / tranche;
            vector<uint8_t> * res = new vector<uint8_t>(0);
            for (uint32_t i = 0; i < Anztransm; i++)
            {
                MailBRd.bufferAddr = PageAddress;
                MailBRd.bufferSize = tranche;
                MailBRd.command = 3;
                MailBRd.memoryOffset = Address + (i*tranche);
                if (RunFlashReadApplet(&MailBRd) == false)
                {
                    printf("ReadSector :  RunFlashReadApplet Fail");
                    return NULL;
                }
                uint8_t data[tranche];
                ReadDataFromDevice(PageAddress, tranche,data);

                res->insert(res->end(), data,data + tranche);

            }
            return res;
        }

        bool SAM_BA_Functions::loadFlashApplet(const uint8_t * Data, uint32_t Size)
        {
            // Öffnen Sie die Datei im Binärmodus


            if (WriteDataToDevice(AppletAddr, Size, Data) == true)
            {

                return true;
            }
            printf("loadFlashApplet :  WriteDataToDevice Fail");
            return false;
        }

        bool SAM_BA_Functions::ReadDataFromDevice(uint32_t Address, uint32_t leng,uint8_t * buffer)
        {


            vector<uint8_t> byteres;
            uint32_t bytesReaded = 0;
            if (serPort == NULL)
            {
                printf("ReadDataFromDevice :  serPort == null Fail");
                return false;
            }
            if (serPort->IsOpen())
            {
            	char Buffer[200];
            	sprintf(Buffer,"R%08X,%08X#",Address,leng);
                string command = Buffer;
                try
                {
                    serPort->ReadExisting();
                    serPort->WriteLine(command);
                    int counter = 0;
                    while(bytesReaded < leng + 2){
                    	bytesReaded+= serPort->Read(byteres, 0x800);
                    	counter ++;
                    	if(counter > 10000){
                    		break;
                    	}
                    }


                }
                catch(...)
                {
                    printf("ReadDataFromDevice : serPort.ReadExisting();serPort.WriteLine(command); Fail");
                    return false;
                }
                if ((leng + 2) < bytesReaded) {
                    if ((byteres[0] == 0x0A) & (byteres[1] == 0x0D))
                    {
                    	memcpy(buffer,byteres.data()+2,leng);
                        //Array.Copy(byteres, 2, Data, 0, leng);
                        return true;
                    } else if ((byteres[0] == 0x3e) & (byteres[1] == 0x0A) & (byteres[2] == 0x0D)) {
                    	memcpy(buffer,byteres.data()+3,leng);
                        //Array.Copy(byteres, 3, Data, 0, leng);
                        return true;
                    }
                }
                else
                {
                    printf("ReadDataFromDevice : leng + 2 < bytesReaded Fail");
                }
                return false;


            }
            else
            {
                printf("ReadDataFromDevice :  serPort.IsOpen Fail");
            }
            return false;
        }

        bool SAM_BA_Functions::WriteDataToDevice(uint32_t Address, uint32_t leng,const uint8_t * buffer)
        {
            string res;
            if (serPort == NULL)
            {
                printf("WriteDataToDevice :  serPort == null Fail");
                return false;
            }
            if (serPort->IsOpen())
            {
            	char Buffer[200];
            	sprintf(Buffer,"S%08X,%08X#",Address,leng);
                string command = Buffer;

                serPort->ReadExisting();
                serPort->WriteLine(command);
                serPort->Write(buffer, 0, leng);
                //serPort->Write((uint8_t *)"\n",0,strlen("\n"));
                for (int i = 0; i < 10; i++)
                {
                    if (serPort->BytesToRead() > 2)
                    {
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                res = serPort->ReadExisting();
                if (res.find(">") !=string::npos)
                {
                    return true;
                }




                printf("WriteDataToDevice :  res !=  >\n\r >   Fail :res=%s",res.c_str());
                return false;
            }
            printf("WriteDataToDevice :  serPort.IsOpen Fail");
            return false;
        }

        bool SAM_BA_Functions::WriteJumpAndEx(uint32_t Address)
        {
            if (serPort == NULL)
            {
                printf("WriteJumpAndEx :  (serPort == null) Fail");
                return false;
            }
            if (serPort->IsOpen())
            {
            	char Buffer[200];
            	sprintf(Buffer,"G%08X#",Address);
                string command = Buffer;
                //string command = "G{Address:X8}#\n";

                serPort->WriteLine(command);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                if (serPort->IsOpen())
                {
                    serPort->ReadExisting();
                }
                return true;
            }
            printf("WriteJumpAndEx :  serPort.IsOpen Fail");
            return false;
        }




} /* namespace std */
