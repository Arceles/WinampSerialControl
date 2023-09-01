#include "pch.h"
#include "windows.h" 
#include "Winamp/wa_ipc.h"
#include "Winamp/GEN.H"
#include "Winamp/WinampDefs.h"
#include <process.h>
#include <iostream>

#include <string.h>
#include <sys/timeb.h>
#include <time.h>

#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <vector>

#define SAMPLERATE 0
#define BITRATE 1
#define CHANNELS 2
#define SECONDS 1
#define MSECONDS 0

int init();
void config();
void quit();
static void Reader(void*);
int play(void);
void volumeUp(void);
void volumeDown(void);
void stop(void);
void pause(void);
int getStatus(void);
int getInfo(int);
void nextTrack(void);
void prevTrack(void);
void fforward(void);
void rewind(void);
int stringSize(char*);
int getVersion(void);
int getPlayTime(void);
int getPlaylistIndex(void);
int getPlaylistLength(void);
char* getPlaylist(void);
char* remove_nulls(char[], int);

void WriteToSerial(HANDLE inputHandle, const char* textToWrite, const DWORD bytesToWrite);


/*****************************************************************/
/************winamp plugin stuff**********************************/
/*****************************************************************/

winampGeneralPurposePlugin plugin =
{
	GPPHDR_VER,
	"Winamp Remote v1.0",
	init,
	config,
	quit,
};
//required function
int init()
{
	//spawn new thread for main servicing function
	_beginthread(Reader, 0, NULL);
	return 0;
}
//required function
void config()
{
}
//required function
void quit()
{
}

extern "C" __declspec(dllexport) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin()
{
	return &plugin;
}

/*****************************************************************/
/************end winamp plugin stuff******************************/
/*****************************************************************/


//function that Rx, Tx, and communicates with winamp
static void Reader(void*)
{
	HWND mainwinampwin = plugin.hwndParent;
	int status;
	char choice;
	int version;
	char aData = 'h';
	double x = clock();
	//CString trackFilename;
	struct _timeb timebuffer;
	char* timeline;
	char* title;
	int pos;
	int current_pos = 0, song_pos;
	char buffer[10];
	char plbuffer[100];
	int i;
	char clear_flag = 0;
	bool exitProgram = FALSE;

	/**************************************************/
	/******find folders******************************/
	/**************************************************/
	std::vector<std::string> folderVector;
	int iteratorFolder = 0;
	std::size_t currentFolderNumber = 0;
	WIN32_FIND_DATAA findData;
	std::string full_path = "C:\\Music\\*";
	std::string currentFolder;
	HANDLE hFind = FindFirstFileA(full_path.c_str(), &findData);

	while (FindNextFileA(hFind, &findData) != 0)
	{
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			folderVector.push_back(std::string(findData.cFileName));
		}
	} 
	FindClose(hFind);

	std::string replaceStringTemplate = "C:\\Progra~2\\Winamp\\Winamp.exe /REPLACE \"C:\\Music\\";
	std::string addStringTemplate = "C:\\Progra~2\\Winamp\\Winamp.exe /ADD \"C:\\Music\\";
	std::string finalStringCMD;
	/**************************************************/
	/******setup com port******************************/
	/**************************************************/
	DWORD dwEventMask;                     // Event mask to trigger
	bool statusSetRead;
	char  TempChar;                        // Temporary Character
	char  SerialBuffer[256];               // Buffer Containing Rxed Data
	DWORD NoBytesRead;                     // Bytes read by ReadFile()
	int charIterator = 0;
	int charIterator2 = 0;


	HANDLE aPort;
	DCB dcbSerialParams = { 0 };  // Initializing DCB structure
	COMMTIMEOUTS timeouts = { 0 };  //Initializing timeouts structure

	LPCSTR pcCommPort = "COM3";
	aPort = CreateFileA(pcCommPort,//port name
		GENERIC_READ | GENERIC_WRITE, //Read/Write
		0,                            // No Sharing
		NULL,                         // No Security
		OPEN_EXISTING,// Open existing port only
		0,            // Non Overlapped I/O
		NULL);        // Null for Comm Devices

	if (aPort == INVALID_HANDLE_VALUE)
		printf("Error in opening serial port");
	else
		printf("opening serial port successful");

	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	dcbSerialParams.BaudRate = CBR_9600;      //BaudRate = 9600
	dcbSerialParams.ByteSize = 8;             //ByteSize = 8
	dcbSerialParams.StopBits = ONESTOPBIT;    //StopBits = 1
	dcbSerialParams.Parity = NOPARITY;      //Parity = None
	
	if (SetCommState(aPort, &dcbSerialParams) == FALSE)
	{
		printf("\nError to Setting DCB Structure\n\n");
	}
	else
	{
		printf("parameters set succesfully");
	}

	//Setting Timeouts
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	if (SetCommTimeouts(aPort, &timeouts) == FALSE)
	{
		printf("\nError to Setting Time outs");
	}
	else
	{
		printf("timeouts set succesfully");
	}

	statusSetRead = SetCommMask(aPort, EV_RXCHAR); //Configure Windows to Monitor the serial device for Character Reception

	if (statusSetRead == FALSE)
		printf("\n\n    Error! in Setting CommMask");
	else
		printf("\n\n    Setting CommMask successfull");
	/***************************************************/
	/******end setup com port***************************/
	/***************************************************/

	while (exitProgram == FALSE)
	{
		//block until remote sends a single byte of data
		statusSetRead = WaitCommEvent(aPort, &dwEventMask, NULL); //Wait for the character to be received

		if (statusSetRead == FALSE)
		{
			printf("\n    Error! in Setting WaitCommEvent()");
		}
		else //If  WaitCommEvent()==True Read the RXed data using ReadFile();
		{
			charIterator = 0;
			charIterator2 = 0;

			printf("\n\n    Characters Received");
			do
			{
				statusSetRead = ReadFile(aPort, &TempChar, sizeof(TempChar), &NoBytesRead, NULL);
				SerialBuffer[charIterator] = TempChar;
				charIterator++;
			} while (NoBytesRead > 0);	
			/*------------Printing the RXed String to Console----------------------*/

			printf("\n\n    ");
			for (charIterator2 = 0; charIterator2 < charIterator - 1; charIterator2++)		// j < i-1 to remove the dupliated last character
				printf("%c", SerialBuffer[charIterator2]);
		}

		choice = SerialBuffer[0];
		//all data that is transmitted ends with a vertical tab '\v'
		//I figure this is not used in any text so it makes a good terminator
		switch (choice)
		{
			//current time
		case '0':
			//********************************
			//   get time
			_ftime(&timebuffer);
			timeline = ctime(&(timebuffer.time));
			WriteToSerial(aPort, timeline, stringSize(timeline));
			WriteToSerial(aPort, "\v", strlen("\v"));
			//**************************************
			break;
			//play
		case '1':
			play();
			break;
			//stop
		case '2':
			stop();
			break;
			//pause
		case '3':
			pause();
			break;
			//increase volume
		case '4':
			volumeUp();
			break;
			//decrease volume
		case '5':
			volumeDown();
			break;
			//nothing
		case '6':
			break;
			//get status:  play, pause, stop
		case '7':
			status = getStatus();
			if (status == 0)
				WriteToSerial(aPort, "stop\v", strlen("stop\v"));
			else if (status == 1)
				WriteToSerial(aPort, "play\v", strlen("play\v"));
			else if (status == 3)
				WriteToSerial(aPort, "pause\v", strlen("pause\v"));
			else
				WriteToSerial(aPort, "unknown\v", strlen("unknown\v"));
			break;
			//samplerate
		case '8':
			//format the output
			sprintf(buffer, "%d\v", getInfo(SAMPLERATE));
			WriteToSerial(aPort, buffer, stringSize(buffer));
			break;
			//bitrate
		case '9':
			//format the output
			sprintf(buffer, "%d\v", getInfo(BITRATE));
			WriteToSerial(aPort, buffer, stringSize(buffer));
			break;
			//number of channels
		case 'a':
			//format the output
			sprintf(buffer, "%d\v", getInfo(CHANNELS));
			WriteToSerial(aPort, buffer, stringSize(buffer));
			break;
			//go to next track
		case 'b':
			nextTrack();
			break;
			//go to prev track
		case 'c':
			prevTrack();
			break;
			//fast forward
		case 'd':
			fforward();
			break;
			//rewind
		case 'e':
			rewind();
			break;
			//get version of winamp
		case 'f':
			version = getVersion();
			//format the output
			sprintf(buffer, "WinAmp v%x.%02x\v", version / 4096, version & 0xFFF);
			WriteToSerial(aPort, buffer, stringSize(buffer));
			break;
			//get current play time
		case 'g':
			//format the output
			sprintf(buffer, "%02d:%02d\v", getPlayTime() / 1000 / 60, (getPlayTime() / 1000) % 60);
			WriteToSerial(aPort, buffer, stringSize(buffer));
			break;
			//get title of current playing song
		case 'h':
			title = (char*)SendMessage(plugin.hwndParent, WM_WA_IPC, getPlaylistIndex(), IPC_GETPLAYLISTTITLE);
			//format the output
			sprintf(buffer, "%s\v", title);
			WriteToSerial(aPort, buffer, stringSize(buffer));
			break;
			//get entire playlist
		case 'i':
			sprintf(plbuffer, "");
			pos = 1;
			while (pos < getPlaylistLength())
			{
				title = (char*)SendMessage(plugin.hwndParent, WM_WA_IPC, pos, IPC_GETPLAYLISTTITLE);
				//format the output
				//put a \r between songs in the playlist
				sprintf(plbuffer, "%s%s\r", plbuffer, title);
				pos++;
			}
			//add the vertical tab as an end of data character
			sprintf(plbuffer, "%s\v", plbuffer);
			WriteToSerial(aPort, plbuffer, stringSize(plbuffer));
			break;
			//get only two songs to display in playlist(incrementing)
		case 'j':
			if (current_pos + 1 < getPlaylistLength())
			{
				current_pos++;
			}

			title = (char*)SendMessage(plugin.hwndParent, WM_WA_IPC, current_pos - 1, IPC_GETPLAYLISTTITLE);

			//fill first 16 characters of buffer with first song
			//add a space to beginning
			plbuffer[0] = ' ';
			for (i = 1; i < 16; i++)
			{
				plbuffer[i] = title[i - 1];
			}

			//remove the null terminating character and add blank spaces
			//after the null is found, replace it and all remaining with spaces
			for (i = 1; i < 16; i++)
			{
				if (plbuffer[i] == 0)
				{
					plbuffer[i] = ' ';
					clear_flag = 1;
				}
				else if (clear_flag == 1)
				{
					plbuffer[i] = ' ';
				}
			}
			clear_flag = 0;

			//get second song to send
			if (current_pos < getPlaylistLength())
			{
				title = (char*)SendMessage(plugin.hwndParent, WM_WA_IPC, current_pos, IPC_GETPLAYLISTTITLE);
				//fill first 16 characters of buffer with second song
				//add a star to beginning to signify current selected song in playlist
				plbuffer[16] = '*';
				for (i = 1; i < 16; i++)
				{
					plbuffer[i + 16] = title[i - 1];
				}
			}

			//remove the null terminating character and add blank spaces
			//after the null is found, replace it and all remaining with spaces
			for (i = 17; i < 32; i++)
			{
				if (plbuffer[i] == 0)
				{
					plbuffer[i] = ' ';
					clear_flag = 1;
				}
				else if (clear_flag == 1)
				{
					plbuffer[i] = ' ';
				}
			}
			clear_flag = 0;

			//add a vertical tab and null terminate it
			plbuffer[32] = '\v';
			plbuffer[33] = 0;

			//write to uart
			WriteToSerial(aPort, plbuffer, stringSize(plbuffer));
			break;
			//get only two songs to display in playlist(decrementing)
		case 'k':
			if (current_pos - 1 >= 0)
			{
				current_pos--;
			}

			title = (char*)SendMessage(plugin.hwndParent, WM_WA_IPC, current_pos, IPC_GETPLAYLISTTITLE);

			//fill first 16 characters of buffer with first song
			//add a star to beginning to signify current playlist selection
			plbuffer[0] = '*';
			for (i = 1; i < 16; i++)
			{
				plbuffer[i] = title[i - 1];
			}

			//remove the null terminating character and add blank spaces
			//after the null is found, replace it and all remaining with spaces
			for (i = 0; i < 16; i++)
			{
				if (plbuffer[i] == 0)
				{
					plbuffer[i] = ' ';
					clear_flag = 1;
				}
				else if (clear_flag == 1)
				{
					plbuffer[i] = ' ';
				}
			}
			clear_flag = 0;

			title = (char*)SendMessage(plugin.hwndParent, WM_WA_IPC, current_pos + 1, IPC_GETPLAYLISTTITLE);

			//fill first 16 characters of buffer with second song
			//add a space to beginning
			plbuffer[16] = ' ';
			for (i = 1; i < 16; i++)
			{
				plbuffer[i + 16] = title[i - 1];
			}

			//remove the null terminating character and add blank spaces
			//after the null is found, replace it and all remaining with spaces
			for (i = 17; i < 32; i++)
			{
				if (plbuffer[i] == 0)
				{
					plbuffer[i] = ' ';
					clear_flag = 1;
				}
				else if (clear_flag == 1)
				{
					plbuffer[i] = ' ';

				}

			}
			clear_flag = 0;

			//add a vertical tab and null terminate it
			plbuffer[32] = '\v';
			plbuffer[33] = 0;
			WriteToSerial(aPort, plbuffer, stringSize(plbuffer));
			break;
			//get the current two songs in playlist
		case 'l':

			if (current_pos == 0)
			{
				song_pos = current_pos + 1;
			}
			else song_pos = current_pos;
			title = (char*)SendMessage(plugin.hwndParent, WM_WA_IPC, song_pos - 1, IPC_GETPLAYLISTTITLE);

			//add star to beginning of song to signify current playlist selection
			if (song_pos - 1 == current_pos)
			{
				plbuffer[0] = '*';
			}
			//add space at beginning of song
			else plbuffer[0] = ' ';

			//fill buffer with first 16 chars of first song
			for (i = 1; i < 16; i++)
			{
				plbuffer[i] = title[i - 1];
			}

			//remove all null chars and fill remaining with spaces
			for (i = 1; i < 16; i++)
			{
				if (plbuffer[i] == 0)
				{
					plbuffer[i] = ' ';
					clear_flag = 1;
				}
				else if (clear_flag == 1)
				{
					plbuffer[i] = ' ';

				}

			}
			clear_flag = 0;


			if (song_pos < getPlaylistLength())
			{
				title = (char*)SendMessage(plugin.hwndParent, WM_WA_IPC, song_pos, IPC_GETPLAYLISTTITLE);
				if (song_pos == current_pos)
				{
					plbuffer[16] = '*';
				}
				else plbuffer[16] = ' ';
				for (i = 1; i < 16; i++)
				{
					plbuffer[i + 16] = title[i - 1];
				}
			}

			//fill with first 16 chars of second song
			for (i = 17; i < 32; i++)
			{
				if (plbuffer[i] == 0)
				{
					plbuffer[i] = ' ';
					clear_flag = 1;
				}
				else if (clear_flag == 1)
				{
					plbuffer[i] = ' ';
				}
			}
			clear_flag = 0;
			//add vertical tab and null char
			plbuffer[32] = '\v';
			plbuffer[33] = 0;
			WriteToSerial(aPort, plbuffer, stringSize(plbuffer));
			break;
			//play current playlist selection
		case 'm':
			//set playlist position
			SendMessage(plugin.hwndParent, WM_WA_IPC, current_pos, IPC_SETPLAYLISTPOS);
			play();
			break;
		case 'n':
			//Send folder list inside c:\\Music, increase iterator for next folder
			currentFolderNumber = iteratorFolder;
			currentFolder = folderVector[iteratorFolder] + "\v";
			WriteToSerial(aPort, currentFolder.c_str(), strlen(currentFolder.c_str()));
			iteratorFolder++;
			break;
		case 'o':
			//Send folder list inside c:\\Music, decrease iterator for next folder
			currentFolderNumber = iteratorFolder;
			currentFolder = folderVector[iteratorFolder] + "\v";
			WriteToSerial(aPort, currentFolder.c_str(), strlen(currentFolder.c_str()));
			iteratorFolder--;
			break;
		case 'p':
			//Replace winamp playlist and play
			finalStringCMD = replaceStringTemplate + folderVector[currentFolderNumber] + "\"";
			system(finalStringCMD.c_str());
			break;
		case 'q':
			//Enqueue to winamp playlist
			finalStringCMD = addStringTemplate + folderVector[currentFolderNumber] + "\"";
			system(finalStringCMD.c_str());
			break;
		case 'z':
			printf("Exiting program");
			WriteToSerial(aPort, "Exiting program\v", strlen("Exiting program\v"));
			exitProgram = TRUE;
			break;
		}

		//Set boundaries for vector access
		if (iteratorFolder >= (int)folderVector.size())
		{
			iteratorFolder = 0;
		}
		else if (iteratorFolder < 0)
		{
			iteratorFolder = (int)folderVector.size() - 1;
		}

		//Clear Serial buffer
		for (charIterator2 = 0; charIterator2 < charIterator - 1; charIterator2++)
			SerialBuffer[charIterator2] = 0;

		//Sleep(5); // wait 100 ms 
	}

	CloseHandle(aPort);//Closing the Serial Port
}


/****************************************************************/
/**************basic control functions***************************/
/****************************************************************/
void volumeUp(void)
{
	SendMessage(plugin.hwndParent, WM_COMMAND, WINAMP_VOLUMEUP, 0);
}

void volumeDown(void)
{
	SendMessage(plugin.hwndParent, WM_COMMAND, WINAMP_VOLUMEDOWN, 0);
}

int play(void)
{
	SendMessage(plugin.hwndParent, WM_COMMAND, WINAMP_PLAY, 0);
	return 0;
}

void stop(void)
{
	SendMessage(plugin.hwndParent, WM_COMMAND, WINAMP_STOP, 0);
}


void pause(void)
{
	SendMessage(plugin.hwndParent, WM_COMMAND, WINAMP_PAUSE, 0);
}

void nextTrack(void)
{
	SendMessage(plugin.hwndParent, WM_COMMAND, WINAMP_NEXT, 0);
}

void prevTrack(void)
{
	SendMessage(plugin.hwndParent, WM_COMMAND, WINAMP_PREV, 0);
}

void fforward(void)
{
	SendMessage(plugin.hwndParent, WM_COMMAND, WINAMP_FFWD5S, 0);
}

void rewind(void)
{
	SendMessage(plugin.hwndParent, WM_COMMAND, WINAMP_REW5S, 0);
}

/****************************************************************/
/**************end basic control functions***********************/
/****************************************************************/

//get play status:  play, pause, stop
int getStatus(void)
{
	return SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING);
}

//get basic info : bitrate, samplrate, channels
int getInfo(int mode)
{
	return SendMessage(plugin.hwndParent, WM_WA_IPC, mode, IPC_GETINFO);
}

//get winamp version
int getVersion(void)
{
	return SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETVERSION);
}

//current time elapsed of song
//time in seconds does not work.  Use MSECONDS.
int getPlayTime(void)
{
	return SendMessage(plugin.hwndParent, WM_WA_IPC, MSECONDS, IPC_GETOUTPUTTIME);
}

//returns the index of the song in the playlist that is currently playing
int getPlaylistIndex(void)
{
	return SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETLISTPOS);
}

//returns pointer to entire playlist
char* getPlaylist(void)
{
	char* song;
	int pos = 0;
	const char initChar[1] = "";
	char* buffer = const_cast<char*>(initChar);
	while (pos < getPlaylistLength())
	{
		song = (char*)SendMessage(plugin.hwndParent, WM_WA_IPC, getPlaylistIndex(), IPC_GETPLAYLISTTITLE);
		sprintf(buffer, "%s\r\n%s\r\n", buffer, song);
		pos++;
	}
	return buffer;

}

//return the number of songs in the playlist
int getPlaylistLength(void)
{
	return SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETLISTLENGTH);
}

//calculate the string size so i don't have to
int stringSize(char* inString)
{
	int x = 0;
	while (inString[x] != 0)
	{
		x++;
	}
	return x;
}

/*****************************************************************/
/************serial port stuff**********************************/
/*****************************************************************/
void WriteToSerial(HANDLE inputHandle, const char* textToWrite, const DWORD bytesToWrite)
{
	bool Status;
	DWORD bytesWritten = 0;          // No of bytes written to the port
	Status = WriteFile(inputHandle,				 // Handle to the Serial port
					   textToWrite,				 // Data to be written to the port
					   bytesToWrite,  //No of bytes to write
					   &bytesWritten, //Bytes written
					   NULL);
	
	if (Status == FALSE)
	{
		printf("Fail to Write");
	}
}