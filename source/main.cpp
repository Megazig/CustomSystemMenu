/*

	Wii System Menu Replacement

*/
#include "../GRRLIB/GRRLIB/GRRLIB.h"
#include "../GRRLIB/GRRLIB/GRRLIB_addon.h"

//#include "../ftp/ftpii.c"

#include <stdlib.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <gccore.h>
#include <stdio.h>
#include <math.h> 
#include <string.h>
#include <mxml.h>
#include <sys/dir.h>
#include <dirent.h>

#include <asndlib.h>
#include <mp3player.h>
#include <gcmodplay.h>

//#include "dol_launcher.h"

char Current_Directory[255], Skin_Directory[255];

GRRLIB_texImg 	*Spr_BlankChannel, 
				*Spr_SelectedChannel,
				*Spr_Font,
				*Spr_MaskChannel;
float view_xview, view_yview, view_hspeed, view_xview_to;
int Snd_MainMenu, Snd_Startup, FOCUS;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool file_exists(char *filename) {
	FILE *tmpFile;
	tmpFile = fopen(filename,"r");
	if (tmpFile == NULL) {
		fclose(tmpFile);
		return false;
	} else {
		fclose(tmpFile);
		return true;
	}
};

void Load_Configuaration() {

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Controls
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class PlayerPad {
	public:
		int X, Y, Channel, Rumble_Time;
		float Angle;
		u32 PadDown,PadHeld;
		ir_t IR;
		orient_t OR;
		GRRLIB_texImg *Spr_Pointer;
		bool WPAD_POWER;
		
		PlayerPad(int WPAD_CHANNEL, const char *Image) {
			Spr_Pointer = GRRLIB_LoadTextureFromFile(Image);
			Channel = WPAD_CHANNEL;
			Rumble_Time=0;
			WPAD_POWER = false;
		};
		~PlayerPad() {
			GRRLIB_FreeTexture(Spr_Pointer);
		};
		
		void Update() {
			WPAD_SetVRes(0, 640,480);
			WPAD_ScanPads();
			
			WPAD_IR(Channel, &IR); 
			WPAD_Orientation(Channel, &OR); 
			
			X=IR.sx-200;
			Y=IR.sy-250;
			Angle=OR.roll;
			
			PadDown = WPAD_ButtonsDown(0);
			PadHeld = WPAD_ButtonsHeld(0);
			
			GRRLIB_DrawImg(X-40, Y-40, Spr_Pointer, Angle, 1, 1, 0x10101040 );
			GRRLIB_DrawImg(X-48, Y-48, Spr_Pointer, Angle, 1, 1, 0xFFFFFFFF );
			
			
			if (Rumble_Time > 1) {
				WPAD_Rumble(Channel, 1);
				Rumble_Time -= 1;
			} else WPAD_Rumble(Channel, 0);

		};

		int GetX() {return X;}
		int GetY() {return Y;}
};
		PlayerPad *PlayerOne;
		PlayerPad *PlayerTwo;
		PlayerPad *PlayerThree;
		PlayerPad *PlayerFour;
void PowerBttn(s32 chan) {
	if (chan==0) PlayerOne->WPAD_POWER = true;
	if (chan==1) PlayerOne->WPAD_POWER = true;
	if (chan==2) PlayerOne->WPAD_POWER = true;
	if (chan==3) PlayerOne->WPAD_POWER = true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GUI System
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class GUI_System {
	public:
		GRRLIB_texImg 	*Spr_ButtonLeft, 
						*Spr_ButtonRight, 
						*Spr_ButtonCenter,
						*Spr_MessageBox,
						*Spr_WindowTopLeft,
						*Spr_WindowTopCenter,
						*Spr_WindowTopRight,
						*Spr_WindowCenterLeft,
						*Spr_WindowCenter,
						*Spr_WindowCenterRight,
						*Spr_WindowBottomLeft,
						*Spr_WindowBottomCenter,
						*Spr_WindowBottomRight;
		bool Debug;
		char DebugLog[255];
		int Line, Line_Position;
		
		FILE *DebugLogFile;
		GUI_System() {
			Spr_ButtonLeft = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/ButtonLeft.png");
			Spr_ButtonRight = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/ButtonRight.png");
			Spr_ButtonCenter = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/ButtonCenter.png");
			
			Spr_WindowTopLeft = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/WindowTopLeft.png");
			Spr_WindowTopRight = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/WindowTopRight.png");
			Spr_WindowTopCenter = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/WindowTopCenter.png");
			
			Spr_WindowCenterLeft = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/WindowCenterLeft.png");
			Spr_WindowCenterRight = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/WindowCenterRight.png");
			Spr_WindowCenter = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/WindowCenter.png");
			
			Spr_WindowBottomLeft = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/WindowBottomLeft.png");
			Spr_WindowBottomRight = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/WindowBottomRight.png");
			Spr_WindowBottomCenter = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/WindowBottomCenter.png");
			
			Spr_MessageBox = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/MessageBox.png");
			
			Spr_Font = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/FONT.png");
			
			GRRLIB_InitTileSet( Spr_Font, 8, 16, 32 );
			Line = 0;
			Line_Position = 0;
			
			PlayerOne->Update();
			
				Debug=true;
				DebugLogFile = fopen("SYS/ErrorLog.txt","w");
				fputs ("// Log File For Riiplacemy System Menu \r\n", DebugLogFile);
				
			
		};
		~GUI_System() {
			if (Debug == true) fclose(DebugLogFile);
			GRRLIB_FreeTexture(Spr_ButtonLeft);
			GRRLIB_FreeTexture(Spr_ButtonRight);
			GRRLIB_FreeTexture(Spr_ButtonCenter);
			GRRLIB_FreeTexture(Spr_MessageBox);
			GRRLIB_FreeTexture(Spr_WindowTopLeft);
			GRRLIB_FreeTexture(Spr_WindowTopCenter);
			GRRLIB_FreeTexture(Spr_WindowTopRight);
			GRRLIB_FreeTexture(Spr_WindowCenterLeft);
			GRRLIB_FreeTexture(Spr_WindowCenter);
			GRRLIB_FreeTexture(Spr_WindowCenterRight);
			GRRLIB_FreeTexture(Spr_WindowBottomLeft);
			GRRLIB_FreeTexture(Spr_WindowBottomCenter);
			GRRLIB_FreeTexture(Spr_WindowBottomRight);
		};
		void Step() {
		};
		bool ButtonRumble;
		bool DrawButton(int X, int Y, int Width) {
			unsigned int BX=X;
			if (PlayerOne->X > X && PlayerOne->Y > Y && PlayerOne->X < X+Width && PlayerOne->Y < (int)(Y+Spr_ButtonLeft->h)) {
				if (ButtonRumble == false) {
					ButtonRumble = true;
					PlayerOne->Rumble_Time = 10;
				};
				GRRLIB_DrawImg(X-(Spr_ButtonLeft->w*0.2), Y-(Spr_ButtonLeft->h*0.1), Spr_ButtonLeft, 0, 1.2, 1.2, 0xFFFFFFFF );
				for(BX=X+(Spr_ButtonLeft->w);
					BX<X+Width-(Spr_ButtonCenter->w*1.2);
					BX+=(Spr_ButtonCenter->w*1.2))
					GRRLIB_DrawImg(BX, Y-(Spr_ButtonCenter->h*0.1), Spr_ButtonCenter, 0, 1.2, 1.2, 0xFFFFFFFF );
				GRRLIB_DrawImg(BX, Y-(Spr_ButtonRight->h*0.1), Spr_ButtonRight, 0, 1.2, 1.2, 0xFFFFFFFF );
				
				if (PlayerOne->PadDown & WPAD_BUTTON_A) return true;
				else return false;
			} else {
				ButtonRumble = false;
				GRRLIB_DrawImg(X, Y, Spr_ButtonLeft, 0, 1, 1, 0xFFFFFFFF );
				for(BX=X+Spr_ButtonLeft->w;
					BX<X+Width-Spr_ButtonCenter->w;
					BX+=Spr_ButtonCenter->w)
					GRRLIB_DrawImg(BX, Y, Spr_ButtonCenter, 0, 1, 1, 0xFFFFFFFF );
				GRRLIB_DrawImg(BX, Y, Spr_ButtonRight, 0, 1, 1, 0xFFFFFFFF );

				return false;
			};
		};
		void Draw_Window(int X, int Y, int width, int height, u8 alpha) {
			u32 color = GRRLIB_GetColor(255,255,255,alpha);
			width = (width-(Spr_WindowTopRight->w-Spr_WindowTopLeft->w));
			width = floor(width/Spr_WindowTopCenter->w)*Spr_WindowTopCenter->w;
			width += (Spr_WindowTopRight->w+Spr_WindowTopLeft->w);
			
			height = (height-(Spr_WindowBottomLeft->h-Spr_WindowTopLeft->h));
			height = floor(height/Spr_WindowCenterLeft->h)*Spr_WindowCenterLeft->h;
			height += (Spr_WindowBottomLeft->h+Spr_WindowTopLeft->h);
			
			GRRLIB_DrawImg(X, Y, Spr_WindowTopLeft, 0, 1, 1, color );
			GRRLIB_DrawImg(X+width-Spr_WindowTopRight->w, 
							Y, 
							Spr_WindowTopRight, 0, 1, 1, color);
			GRRLIB_DrawImg(X, 
							Y+height-Spr_WindowBottomLeft->h, 
							Spr_WindowBottomLeft, 0, 1, 1, color );
			GRRLIB_DrawImg(X+width-Spr_WindowBottomRight->w,
							Y+height-Spr_WindowBottomRight->h, 
							Spr_WindowBottomRight, 0, 1, 1, color );
			unsigned int WX,WY;
			
			for(WX = X+Spr_WindowTopLeft->w;
				WX < X+width-Spr_WindowTopRight->w;
				WX += Spr_WindowTopCenter->w) {
					GRRLIB_DrawImg(WX, Y, Spr_WindowTopCenter, 0, 1, 1, color );
					GRRLIB_DrawImg(WX, Y+height-Spr_WindowBottomCenter->h, Spr_WindowBottomCenter, 0, 1, 1, color );
			}
			for(WY = Y+Spr_WindowTopLeft->h;
				WY < Y+height-Spr_WindowBottomLeft->h;
				WY += Spr_WindowCenterLeft->h) {
					GRRLIB_DrawImg(X, WY, Spr_WindowCenterLeft, 0, 1, 1, color );
					GRRLIB_DrawImg(X+width-Spr_WindowCenterRight->w, WY, Spr_WindowCenterRight, 0, 1, 1, color );
			}
			
		};
		void Draw_Tooltip(int X, int Y, int width, int height, u8 alpha, char *text) {
			u32 color = GRRLIB_GetColor(255,255,255,alpha);
			width -= (Spr_WindowTopRight->w+Spr_WindowTopLeft->w);
			width = floor(width/Spr_WindowTopCenter->w)*Spr_WindowTopCenter->w;
			width += (Spr_WindowTopRight->w+Spr_WindowTopLeft->w);
			
			height -=(Spr_WindowBottomLeft->h+Spr_WindowTopLeft->h);
			height = floor(height/Spr_WindowCenterLeft->h)*Spr_WindowCenterLeft->h;
			height += (Spr_WindowBottomLeft->h+Spr_WindowTopLeft->h);
			
			GRRLIB_DrawImg(X, Y, Spr_WindowTopLeft, 0, 1, 1, color);
			GRRLIB_DrawImg(X+width-Spr_WindowTopRight->w, 
							Y, 
							Spr_WindowTopRight, 0, 1, 1, color);
			GRRLIB_DrawImg(X, 
							Y+height-Spr_WindowBottomLeft->h, 
							Spr_WindowBottomLeft, 0, 1, 1, color );
			GRRLIB_DrawImg(X+width-Spr_WindowBottomRight->w,
							Y+height-Spr_WindowBottomRight->h, 
							Spr_WindowBottomRight, 0, 1, 1, color );
			unsigned int WX,WY;
			
			for(WX = X+Spr_WindowTopLeft->w;
				WX < X+width-Spr_WindowTopRight->w;
				WX += Spr_WindowTopCenter->w) {
					GRRLIB_DrawImg(WX, Y, Spr_WindowTopCenter, 0, 1, 1, color );
					GRRLIB_DrawImg(WX, Y+height-Spr_WindowBottomCenter->h, Spr_WindowBottomCenter, 0, 1, 1, color );
			}
			for(WY = Y+Spr_WindowTopLeft->h;
				WY < Y+height-Spr_WindowBottomLeft->h;
				WY += Spr_WindowCenterLeft->h) {
					GRRLIB_DrawImg(X, WY, Spr_WindowCenterLeft, 0, 1, 1, color );
					GRRLIB_DrawImg(X+width-Spr_WindowCenterRight->w, WY, Spr_WindowCenterRight, 0, 1, 1, color );
			}
			GRRLIB_Printf(X+Spr_WindowCenterRight->w,Y+(height/3),Spr_Font,color,1,text);
		};
		void ShowMessage(const char *MessageString) {
			int stage=0, XX=120, YY=480;
			GRRLIB_texImg *Spr_Screen = GRRLIB_CreateEmptyTexture(640, 480);
			GRRLIB_Screen2Texture(Spr_Screen);
			
			PlayerOne->Update();
			while(PlayerOne->PadDown != WPAD_BUTTON_PLUS) {
				GRRLIB_DrawImg(0, 0, Spr_Screen, 0, 1, 1, 0xFFFFFFFF );
				switch(stage) {
					case 0:
						GRRLIB_DrawImg(XX, YY, Spr_MessageBox, 0, 1, 1, 0xFFFFFFFF );
						GRRLIB_Printf(XX+32,YY+80,Spr_Font,0x000000FF,1,MessageString);
						DrawButton(XX+100,YY+200,200);
						if (YY > 120) YY -= 40;
						else stage = 1;
					break;
					case 1:
						GRRLIB_DrawImg(XX, YY, Spr_MessageBox, 0, 1, 1, 0xFFFFFFFF );
						GRRLIB_Printf(XX+32,YY+80,Spr_Font,0x000000FF,1,MessageString);
						if (DrawButton(XX+100,YY+200,200)) stage = 2;
					break;
					case 2:
						GRRLIB_DrawImg(XX, YY, Spr_MessageBox, 0, 1, 1, 0xFFFFFFFF );
						GRRLIB_Printf(XX+32,YY+80,Spr_Font,0x000000FF,1,MessageString);
						DrawButton(XX+100,YY+200,200);
						if (YY >-280) YY-=40;
						else stage = 3;
					break;
				};
				if (stage == 3) break;
				PlayerOne->Update();
				GRRLIB_Render();
			};
			
			GRRLIB_FreeTexture(Spr_Screen);
		};

		int GetInteger() {
			return 1;
		};
		
		void GetString() {
		};
		
		void AddLog(const char *text) {
			if (Debug==true) {
				sprintf(DebugLog,"%s \r\n",text);
				fputs(DebugLog,DebugLogFile);
			};
		};
};
		GUI_System *GUI;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sound System
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static MODPlay modplay;
class SND_Resource {
	public:
		unsigned long FileSize;
		int Type;			/**< Type of sound 0=mod 1=mp3 2=wav 3=pcm 4=ogg */
		bool Loop;
		bool Isplaying;
		char *Data;
		SND_Resource(char *filename) {														GUI->AddLog("Creating Sound");
			Loop = false;
			Isplaying = false;
			Type = -1;
			
			size_t result;
			FILE *f=fopen(filename, "rb");
			Data = NULL;
			
			if (f == NULL) {																GUI->AddLog("[FAILED] Could Not Find File");
				fclose(f);
			} else {
				fseek (f , 0, SEEK_END);
				FileSize = ftell (f);
				rewind(f);
				Data = (char*) malloc (sizeof(char)*FileSize);
				if (Data!=NULL) {
					result = fread (Data,1,FileSize,f);
					if (result == FileSize) {												GUI->AddLog("Loaded Successfully");
																							GUI->AddLog("Checking Extention");
						if (strstr(filename,".mod") != NULL) {Type = 0;						GUI->AddLog("Sound is a Mod File");}
						if (strstr(filename,".mp3") != NULL) {Type = 1;						GUI->AddLog("Sound is a MP3 File");}
						if (strstr(filename,".wav") != NULL) {Type = 2;						GUI->AddLog("Sound is a Wav File");}
						if (strstr(filename,".pcm") != NULL) {Type = 3;						GUI->AddLog("Sound is a PCM File");}
						if (strstr(filename,".ogg") != NULL) {Type = 4;						GUI->AddLog("Sound is a OGG File");}
						if (Type == -1) {
							GUI->AddLog("[FAILED] Could not figure out the type based on extention. Make sure the extenstion is lower case.");
							free(Data);
							Data = NULL;
						};
					} else {																GUI->AddLog("[FAILED] Reading From Disk");
						free(Data);
						Data = NULL;
					}; 
				} else GUI->AddLog("[FAILED] Allocating Memory");
				fclose(f);
			};
		};
		~SND_Resource() {
			//if (Data!=NULL)
				//free(Data);
		};
};

class SND_System {
	public:
		SND_Resource *Sounds[50];
		int Current_New_Index;
		bool MODPLAY_ISPLAYING, MP3_ISPLAYING;
		SND_System() {																		GUI->AddLog("[OK] Initializing sound system.");
			int i;
			AUDIO_Init(NULL);
			ASND_Init();
			MODPlay_Init(&modplay);
			MP3Player_Init();
			
			Current_New_Index = 0;
			
			MODPLAY_ISPLAYING = false;
			MP3_ISPLAYING =false;

			for(i=0;i<50;i++)
				Sounds[i]=NULL; 															GUI->AddLog("Success");
		};
		~SND_System() {
			int i;
			if (MODPLAY_ISPLAYING) MODPlay_Stop(&modplay);
			if (MP3_ISPLAYING) MP3Player_Stop();
			for(i=0;i<50;i++)
				if (Sounds[i]->Data != NULL)
					free(Sounds[i]->Data);
		};
		
		int AddSound(char *filename) {														GUI->AddLog("[OK] Adding Sound");
			if (Sounds[Current_New_Index] == NULL) {
				int this_index = Current_New_Index;
				SND_Update_New_Index();
				
				Sounds[this_index] = new SND_Resource(filename);
				if (Sounds[this_index]->Data == NULL) {
					delete Sounds[this_index]; 
					return -1;
				};
				
				return this_index;
			} else { 																		GUI->AddLog("[Failed] Sound Index Exists Already");
				return -1; 
			};
		};
		bool DeleteSound(int index) {
			if (index == -1) return false;
			if (Sounds[index] == NULL) return false;
			
			delete Sounds[index];
			Sounds[index] = NULL;
			return true;
		};
		bool PlaySound(int index) {															GUI->AddLog("[OK] Playing Sound");
			if (index == -1) return false;
			if (Sounds[index] == NULL) return false;
			if (Sounds[index]->Data == NULL) return false;
			
			MODPLAY_ISPLAYING = false;
			MP3_ISPLAYING = false;
			if (Sounds[index]->Type == 0) {												GUI->AddLog("Is a Mod File");
				if (MODPLAY_ISPLAYING) MODPlay_Stop(&modplay);
				if (MP3_ISPLAYING) MP3Player_Stop();
																							GUI->AddLog("Stopped existing music");
				MODPlay_SetMOD(&modplay, Sounds[index]->Data);
				MODPlay_Start(&modplay);
				MODPlay_SetVolume(&modplay, 100, 100);
																							GUI->AddLog("Success");
				Sounds[index]->Isplaying = true;
				Sounds[index]->Loop = false;
				
				MODPLAY_ISPLAYING = true;
				return true;
			};
			if (Sounds[index]->Type == 1) {												GUI->AddLog("Is a MP3 File");
				if (MODPLAY_ISPLAYING) MODPlay_Stop(&modplay);
				if (MP3_ISPLAYING) MP3Player_Stop();
																							GUI->AddLog("Stopped existing music");
				MP3Player_PlayBuffer(Sounds[index]->Data,Sounds[index]->FileSize,NULL);
				MP3Player_Volume(255);
																							GUI->AddLog("Success");
				Sounds[index]->Isplaying = true;
				Sounds[index]->Loop = false;
				
				MP3_ISPLAYING = true;
				return true;
			};
			return false;
		};
		bool LoopSound(int index) {															GUI->AddLog("[OK] Loop Sound");
			if (index == -1) return false;
			if (Sounds[index] == NULL) return false;
			if (Sounds[index]->Data == NULL) return false;
			
			PlaySound(index);
			
			Sounds[index]->Loop = true;

			return true;
		};
		bool StopSound(int index) {															GUI->AddLog("[OK] Stopping Sound");
			if (index == -1) return false;
			if (Sounds[index] == NULL) return false;
			if (Sounds[index]->Data == NULL) return false;
			if (Sounds[index]->Type == 0) {												GUI->AddLog("Is a Mod File");
				if (MODPLAY_ISPLAYING) MODPlay_Stop(&modplay);
				if (MP3_ISPLAYING) MP3Player_Stop();
				
				Sounds[index]->Isplaying = false;
				Sounds[index]->Loop = false;
				MP3_ISPLAYING = false;
				MODPLAY_ISPLAYING = false;
				return true;
			};
			if (Sounds[index]->Type == 1) {												GUI->AddLog("Is a MP3 File");
				if (MODPLAY_ISPLAYING) MODPlay_Stop(&modplay);
				if (MP3_ISPLAYING) MP3Player_Stop();
				
				Sounds[index]->Isplaying = false;
				Sounds[index]->Loop = false;
				MODPLAY_ISPLAYING = false;
				MP3_ISPLAYING = false;
				return true;
			};
			
			return false;
		};
		void StopAllSound() {
			int i;
			for(i=0;i<50;i++)
				StopSound(i);
		};
		bool SoundIsplaying(int index) {
			if (index == -1) return false; 
			if (Sounds[index] == NULL) return false;
			return Sounds[index]->Isplaying;
		};
		void SND_Update_New_Index() {
			Current_New_Index += 1;
		};
};
	SND_System *SoundSystem;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Channel System
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Animation {
	public:
		GRRLIB_texImg *Spr_Resources[10];
		int Image_Startx[10], Image_Time[10],
			Image_Starty[10],
			Image_TransitionIn[10],
			Image_TransitionInStart[10],
			Image_TransitionInLength[10],
			Image_TransitionStep[10],
			Image_TransitionOut[10],
			Image_TransitionOutStart[10],
			Image_TransitionOutLength[10],
			Image_TransitionTotal[10],
			Snd_Resource, Type;
		
		float Image_X[10], Image_Y[10], Image_Xscale[10], Image_Yscale[10], Image_Alpha[10];
		bool Channel_Exists;
		char Directory[260];
		
		Animation(const char *ChannelDirectory) {														GUI->AddLog("Creating Animation");
			Type = -1;
			int i;
			for(i=0;i<10;i+=1)
				{Spr_Resources[i] = NULL;
				Image_Startx[i] = 0;
				Image_Starty[i] = 0;
				Image_Time[i] = 9999;
				Image_Alpha[i] = 255;
				Image_Xscale[i] = 1;
				Image_Yscale[i] = 1;};
			Channel_Exists = false;
			Snd_Resource = NULL;
			sprintf(Directory,"%s",ChannelDirectory);
			char FileName[260];
			
			sprintf(FileName,"%s/Animation.xml",ChannelDirectory);
			if (file_exists(FileName)) Load_Animated_Icon();
			
			sprintf(FileName,"%s/AlbumArt.png",ChannelDirectory);
			if (file_exists(FileName)) Load_MP3_Album();
			
		};
		~Animation() {																			GUI->AddLog("Deleting Animation");
			int i;
			for(i=0;i<10;i+=1) {
				if (Spr_Resources[i]!=NULL){};
					GRRLIB_FreeTexture(Spr_Resources[i]);
			};
		};

		void Draw_Animated_Icon(int X, int Y) {
			GRRLIB_ClipDrawing( X, Y, 128,96 );
			int i;
			for(i=0;i<10;i+=1) {
				if (Spr_Resources[i]!=NULL) {
					Image_Time[i]+=1;
					if (Image_Time[i] > Image_TransitionTotal[i]) {
						Image_Time[i] = 0;
						Image_Alpha[i] = 255;
						Image_Xscale[i] = 1;
						Image_Yscale[i] = 1;
						Image_X[i]=Image_Startx[i];
						Image_Y[i]=Image_Starty[i];
						switch(Image_TransitionIn[i]) {
							case 0:
								Image_Alpha[i]=0;
							break;
							case 1:
								Image_X[i]=128;
							break;
							case 2:
								Image_Y[i]=96;
							break;
							case 3:
								Image_X[i]=-((float)Spr_Resources[i]->w);
							break;
							case 4:
								Image_Y[i]=-((float)Spr_Resources[i]->h);
							break;
						};
					}
					
					//Transition in effect
					if (Image_Time[i] > Image_TransitionInStart[i] && Image_Time[i] <= Image_TransitionInStart[i]+Image_TransitionInLength[i])
					{
						switch(Image_TransitionIn[i]) {
							case 0:
								Image_Alpha[i]+=(float)255/(Image_TransitionInLength[i]);
								if (Image_Alpha[i] > 255) Image_Alpha[i] = 255;
							break;
							case 1:
								Image_X[i]-=(float)(128-Image_Startx[i])/(Image_TransitionInLength[i]);
							break;
							case 2:
								Image_Y[i]-=(float)(96-Image_Starty[i])/(Image_TransitionInLength[i]);
							break;
							case 3:
								Image_X[i]+=(float)(Image_Startx[i]+Spr_Resources[i]->w)/(Image_TransitionInLength[i]);
							break;
							case 4:
								Image_Y[i]+=(float)(Image_Starty[i]+Spr_Resources[i]->h)/(Image_TransitionInLength[i]);
							break;
						};
					};
					if (Image_Time[i] == Image_TransitionInStart[i]+Image_TransitionInLength[i]+1) {
						Image_Alpha[i] = 255;
						Image_Xscale[i] = 1;
						Image_Yscale[i] = 1;
						Image_X[i]=Image_Startx[i];
						Image_Y[i]=Image_Starty[i];
					};
					//Transition Step
					
					//Transition Out
					if (Image_Time[i] >= Image_TransitionOutStart[i] && Image_Time[i] <= Image_TransitionOutStart[i]+Image_TransitionOutLength[i])
					{
						switch(Image_TransitionOut[i]) {
							case 0:
								Image_Alpha[i]-=(float)255/(Image_TransitionOutLength[i]);
								if (Image_Alpha[i] < 0) Image_Alpha[i] = 0;
							break;
							case 1:
								Image_X[i]+=(float)(128-Image_Startx[i])/(Image_TransitionOutLength[i]);
							break;
							case 2:
								Image_Y[i]+=(float)(96-Image_Starty[i])/(Image_TransitionOutLength[i]);
							break;
							case 3:
								Image_X[i]-=(float)(Image_Startx[i]+Spr_Resources[i]->w)/(Image_TransitionOutLength[i]);
							break;
							case 4:
								Image_Y[i]-=(float)(Image_Starty[i]+Spr_Resources[i]->h)/(Image_TransitionOutLength[i]);
							break;
						};
					};
					if (Image_Time[i] == Image_TransitionOutStart[i]+Image_TransitionOutLength[i]+1) {
						Image_Alpha[i] = 255;
						Image_Xscale[i] = 1;
						Image_Yscale[i] = 1;
						Image_X[i]=128;
						Image_Y[i]=96;
					};
					//End of transtions
					GRRLIB_DrawImg(X+Image_X[i], Y+Image_Y[i], Spr_Resources[i], 0, Image_Xscale[i], Image_Yscale[i], GRRLIB_GetColor(255,255,255,round(Image_Alpha[i])) );
					
				};
			};
			GRRLIB_ClipReset();
		};


	private:
		void Load_Animated_Icon() {
			Type = 0;
			mxml_node_t *tree;
			mxml_node_t *data;
			mxml_node_t *main;
			int i;
			char XML_Filepath[255];
			
			sprintf(XML_Filepath,"%s/Animation.XML",Directory);
			
			FILE *XML_File = fopen(XML_Filepath, "rb");
		
			fseek (XML_File , 0, SEEK_END);
			long settings_size = ftell (XML_File);
			rewind (XML_File);
			
			if (settings_size > 0) {
				tree = mxmlLoadFile(NULL, XML_File, MXML_NO_CALLBACK);
				fclose(XML_File);
				
				char Image_Filename[255]="", Tag[6]="";
				
				sprintf(Image_Filename,"%s/folderMusic.mod",Directory);
				Snd_Resource = SoundSystem->AddSound(Image_Filename);
					
				main = mxmlFindElement(tree, tree, "main", NULL, NULL, MXML_DESCEND);
				for(i=0;i<10;i+=1) {
					sprintf(Tag,"image%i",i);
					data = mxmlFindElement(tree, main, Tag, NULL, NULL, MXML_DESCEND);
					
					if (mxmlElementGetAttr(data,"filename") != NULL) {
						sprintf(Image_Filename,"%s/%s",Directory,mxmlElementGetAttr(data,"filename"));
						if (file_exists(Image_Filename)) {
							Spr_Resources[i] = GRRLIB_LoadTextureFromFile(Image_Filename);
							Channel_Exists = true;
							
							if (mxmlElementGetAttr(data,"x")!=NULL &&
								mxmlElementGetAttr(data,"y")!=NULL &&
								mxmlElementGetAttr(data,"in_fx")!=NULL &&
								mxmlElementGetAttr(data,"in_fx_start")!=NULL &&
								mxmlElementGetAttr(data,"step_fx")!=NULL &&
								mxmlElementGetAttr(data,"out_fx")!=NULL &&
								mxmlElementGetAttr(data,"out_fx_start")!=NULL &&
								mxmlElementGetAttr(data,"out_fx_time")!=NULL &&
								mxmlElementGetAttr(data,"loop_time")!=NULL) {
									Image_Startx[i] = atoi(mxmlElementGetAttr(data,"x"));
									Image_Starty[i] = atoi(mxmlElementGetAttr(data,"y"));
									Image_TransitionIn[i] = atoi(mxmlElementGetAttr(data,"in_fx"));
									Image_TransitionInStart[i] = atoi(mxmlElementGetAttr(data,"in_fx_start"));
									Image_TransitionInLength[i] = atoi(mxmlElementGetAttr(data,"in_fx_time"));
									Image_TransitionStep[i] = atoi(mxmlElementGetAttr(data,"step_fx"));
									Image_TransitionOut[i] = atoi(mxmlElementGetAttr(data,"out_fx"));
									Image_TransitionOutStart[i] = atoi(mxmlElementGetAttr(data,"out_fx_start"));
									Image_TransitionOutLength[i] = atoi(mxmlElementGetAttr(data,"out_fx_time"));
									Image_TransitionTotal[i] = atoi(mxmlElementGetAttr(data,"loop_time"));
							};
						} else GUI->AddLog("[Failed] Invalid reference to an image in the xml file");
					};
					mxmlDelete(data);
				};
				mxmlDelete(tree);
			};
		};
		void Load_MP3_Album() {
			Type = 1;
			Channel_Exists = true;
			char Image_Filename[260];
			sprintf(Image_Filename,"%s/AlbumArt.png",Directory);
			Spr_Resources[0] = GRRLIB_LoadTextureFromFile(Image_Filename);
		};

};

class Channel {
	public:
		Animation *My_Animation;
		int XX,YY, Xto, Yto, MyId, MyNameBubble;
		float X,Y, MySelected, MP3_Xscale, MP3_Xscalespeed;
		char MyPath[255];
		bool Dragging, MP3_Flip;
		
		Channel() {
				MySelected=1;
				MyId = -1;
				
				Dragging = false;
				
				X = 0; Y = 0;
				Xto = 320; Yto = 240;
				MP3_Flip=false; MP3_Xscale=1; MP3_Xscalespeed=0;
				
				MyNameBubble = 0;
				My_Animation = NULL;
		};
		~Channel() {
			if (My_Animation != NULL)
				delete My_Animation;
		};
		int Handle() {
			if (My_Animation == NULL) return -1;
			int Returning=-1;
			XX = X-view_xview;
			YY = Y-view_yview;
			switch(My_Animation->Type) {
				case -1:
					GRRLIB_DrawImg(XX, YY, Spr_BlankChannel, 0, 1, 1, 0xFFFFFFFF);
				break;
				case 0:
					Returning=Draw_Animated_Icon();
				break;
				case 1:
					Returning=Draw_Mp3_Album();
				break;
				case 2:
					Returning=Draw_RSS_Feed();
				break;
				case 3:
					Returning=Draw_Photo_Frame();
				break;
			};
			// Handle Dragging
			if (Dragging == true) {
				Xto = view_xview+(PlayerOne->X-64); 
				Yto = view_yview+(PlayerOne->Y-48);
				if (Xto < 0) Xto = 0;
				if (Xto > 2816) Xto = 2816;
				if (Yto < 0) Yto = 0;
				if (Yto > 224) Yto = 224;
				
				GRRLIB_DrawImg(Xto-view_xview, Yto-view_yview, Spr_BlankChannel, 0, 1, 1, 0xFFFFFF55);
				
				Xto = 64+(round(Xto/Spr_MaskChannel->w)*Spr_MaskChannel->w);
				Yto = 32+(round(Yto/Spr_MaskChannel->h)*Spr_MaskChannel->h);
				
				
			};
			if (Dragging == false && (Xto != X || Yto != Y)) {

				if (X != Xto) X+= (float)((Xto-X)/5);
				if (Y != Yto) Y+= (float)((Yto-Y)/5);
				
				if (X > Xto-30 && X < Xto+30) X = Xto;
				
				if (Y > Yto-30 && Y < Yto+30) Y = Yto;
				GRRLIB_DrawImg(Xto-view_xview, Yto-view_yview, Spr_BlankChannel, 0, 1, 1, 0xFFFFFF99);
				
			};
			return Returning;
		};
	private:
		int Draw_Animated_Icon() {
			// Handle Drawing
			My_Animation->Draw_Animated_Icon(XX,YY);
			GRRLIB_DrawImg(XX, YY, Spr_MaskChannel, 0, 1, 1, 0xFFFFFFFF);
			if (MySelected < 1) {
				GRRLIB_DrawImg(XX+(MySelected*12), YY+(MySelected*10), Spr_SelectedChannel, 0, (1-(MySelected/5)), (1-(MySelected/10)), GRRLIB_GetColor(100, 255, 100, (1-MySelected)*255) );
				MySelected += 0.05;
			};
			
			if (MyNameBubble > 0) MyNameBubble -= 20;
			if (MyNameBubble > 100) {
				if (MyNameBubble > 355) MyNameBubble = 355;
				GUI->Draw_Tooltip(XX,YY+96,150,80,MyNameBubble-100,MyPath+9);
			};
			
			//Handle Player Cursor
			if (PlayerOne->X > XX && PlayerOne->Y > YY && PlayerOne->X < XX+128 && PlayerOne->Y < YY+96) {
				MySelected=0;
				MyNameBubble+=40;
				return MyId;
			} else return -1;
		};
		int Draw_Mp3_Album() {
			if (My_Animation->Spr_Resources[0]==NULL) return -1;
			
			int width=180*MP3_Xscale;
			int height=180;

			if (!MP3_Flip)
				GRRLIB_DrawImg(XX, YY, My_Animation->Spr_Resources[0], 0, 
								(float)width/My_Animation->Spr_Resources[0]->w, (float)height/My_Animation->Spr_Resources[0]->h, 0xFFFFFFFF);
			if (MP3_Flip)
				GRRLIB_Rectangle(XX, YY, width, height, 0xFFFFFFFF, true);
				
			if (MP3_Xscale < 0) {MP3_Flip=!MP3_Flip; MP3_Xscalespeed = -MP3_Xscalespeed;};
			if (MP3_Xscale > 1) {MP3_Xscale = 1; MP3_Xscalespeed = 0;};
			MP3_Xscale += MP3_Xscalespeed;
			
			if (PlayerOne->X > XX && PlayerOne->Y > YY && PlayerOne->X < XX+width && PlayerOne->Y < YY+height) {
				MySelected=0;
				if (PlayerOne->PadDown & WPAD_BUTTON_A) {
					MP3_Xscalespeed = -0.1;
				};
				return -1;
			} else return -1;
		};
		int Draw_RSS_Feed() {
			return -1;
		};
		int Draw_Dvd_Channel() {
			return -1;
		};
		int Draw_Photo_Frame() {
			return -1;
		};
		
		
};
		
class Channel_System {
	private:
		int NextEmptyChannel;
		void Update_NextEmptyChannel() {
			NextEmptyChannel = 0;
			while(Channels[NextEmptyChannel]->My_Animation != NULL)
				NextEmptyChannel+=1;
		};
		
	public:
		Channel *Channels[200];
		
		Channel_System() {
			int i;
			for(i=0;i<200;i++)
				Channels[i] = new Channel();
			NextEmptyChannel = 0;
		};
		
		~Channel_System() {
			Clear_All_Channel();
		};
/////////////////////////////////////////////////////////////////////////////////////////////
		int Add_Channel(const char *Directory) {											GUI->AddLog("[OK] Creating Channel");
			int this_channel=NextEmptyChannel;
			
				Channels[this_channel]->My_Animation = new Animation(Directory);
				if (Channels[this_channel]->My_Animation->Channel_Exists == false) {GUI->AddLog("[Failed] Loading failed.");
					Clear_Channel(this_channel);
					return -1;
				};
																							GUI->AddLog("Success");
				sprintf(Channels[this_channel]->MyPath,"%s",Directory);
				
				Channels[this_channel]->MySelected=1;
				Channels[this_channel]->MyId = this_channel;

				Channels[this_channel]->Dragging = false;
				
				Channels[this_channel]->X = -300;
				Channels[this_channel]->Y = -300;
				
				Channels[this_channel]->Xto = 64+(128*this_channel);
				Channels[this_channel]->Yto = 32;
				
			Update_NextEmptyChannel();
			
			return this_channel;
		};
		void Clear_Channel(int Channel_Index) {											GUI->AddLog("[OK] Deleting Channel");
			if (Channels[Channel_Index] == NULL) return;
			if (Channels[Channel_Index]->My_Animation == NULL) return;
			
			delete Channels[Channel_Index]->My_Animation;
			
			Channels[Channel_Index]->My_Animation = NULL;
			Channels[Channel_Index]->MyId = -1;
			
			Channels[Channel_Index]->MySelected=1;
			Channels[Channel_Index]->MyId = Channel_Index;

			Channels[Channel_Index]->Dragging = false;
			
			Channels[Channel_Index]->X = -1000;
			Channels[Channel_Index]->Y = -1000;
			
			Channels[Channel_Index]->Xto = 320;
			Channels[Channel_Index]->Yto = 240;
			
			sprintf(Channels[Channel_Index]->MyPath,"//");
			Channels[Channel_Index] = NULL;
		};
		void Clear_All_Channel() {
			int i;
			for(i=0;i<200;i++) 
				if (Channels[i] != NULL && Channels[i]->My_Animation != NULL) {
					delete Channels[i]->My_Animation;
					
					Channels[i]->My_Animation = NULL;
					Channels[i]->MyId = -1;
					
					Channels[i]->MySelected=1;
					Channels[i]->MyId = -1;
	
					Channels[i]->Dragging = false;
					
					Channels[i]->X = -1000;
					Channels[i]->Y = -1000;
					
					Channels[i]->Xto = 320;
					Channels[i]->Yto = 240;
					
					sprintf(Channels[i]->MyPath,"//");
					Channels[i] = NULL;
				};
		};
/////////////////////////////////////////////////////////////////////////////////////////////
		bool Channel_Set_Dragging(int Channel_Index, bool Action) {
			if (Channel_Index == -1) return false;
			if (Channels[Channel_Index]->My_Animation == NULL) return false;
			Channels[Channel_Index]->Dragging = Action;
			return true;
		};
		
		int Channel_Get_X(int Channel_Index) {
			if (Channel_Index == -1) return -1;
			if (Channels[Channel_Index]->My_Animation == NULL) return -1;
			return Channels[Channel_Index]->X;
		};
		int Channel_Get_Y(int Channel_Index) {
			if (Channel_Index == -1) return -1;
			if (Channels[Channel_Index]->My_Animation == NULL) return -1;
			return Channels[Channel_Index]->Y;
		};
		int Channel_Get_Sound(int Channel_Index) {
			if (Channel_Index == -1) return -1;
			if (Channels[Channel_Index]->My_Animation == NULL) return -1;
			return Channels[Channel_Index]->My_Animation->Snd_Resource;
		};
		
		int Handle_Channel(int Channel_Index) {
			if (Channel_Index == -1) return -1;
			if (Channels[Channel_Index]->My_Animation == NULL) return -1;
			return Channels[Channel_Index]->Handle();
		};
/////////////////////////////////////////////////////////////////////////////////////////////
		bool SetDirectoryToChannel(int Channel_Index) {
			if (Channel_Index == -1) return false;
			if (Channels[Channel_Index] == NULL) return false;
			sprintf(Current_Directory,"%s",Channels[Channel_Index]->MyPath);
			return true;
		};

};
	Channel_System *ChannelSystem;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Main Loop
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MainBrowser {
	public:
		
		GRRLIB_texImg 	*Spr_Transition,
						*Transition_Image,
						*Spr_Wallpaper,
						*Spr_Background;
		
		int MenuType, 
			MenuTypeTo, 
			Alpha, 
			Selected, 
			Transition_Id, 
			Transition_Previous, 
			Dragging, 
			Transition_Type,
			MainChannels[48],
			SubChannels[48];
		float Transition_X, Transition_Y, TranX, TranY, Transition_PreviousX, Transition_PreviousY;
		u32 Color;
		
		MainBrowser() {
			//Define Menutypes
			MenuType = 0;
			
			//Load GUI material
			Spr_Transition = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/Transition.png");
			Spr_Wallpaper = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/Wallpaper.png");
			Spr_Background = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/Background.png");
			
			//Initialize Variables
			MenuType = 0;
			MenuTypeTo = 0;
			Alpha = 255;
			Dragging = -1;
			Selected = -1;
			Transition_Id=-1;
			Transition_Previous=-1;
			view_xview = 0;
			view_xview_to = 0;
			view_hspeed = 0;
			view_yview = 0;
			Transition_Type = -1;
			FOCUS = 0;
			int i;
			for(i=0;i<48;i++) {
				MainChannels[i]=-1;
				SubChannels[i]=-1;
			}
			
			MainChannels[0] = ChannelSystem->Add_Channel("sys/chnl/Super Mario Brothers 1");
			MainChannels[1] = ChannelSystem->Add_Channel("sys/chnl/Super Mario Brothers 2");
			MainChannels[2] = ChannelSystem->Add_Channel("sys/chnl/Super Mario Brothers 3");
			
			//Initialize SOUND
			SoundSystem->PlaySound(Snd_Startup);
			
			while(true) { //Main Loop for the Browser
				/* 
					Handle Type of Menu
						MenuType_Step()
				*/
				int i;
				switch(MenuType) {
					//Initialize
					case 0: 
						//GRRLIB_FillScreen(0x505050ff);
						GRRLIB_DrawImg(0, 0, Spr_Background, 0, 1, 1, 0xFFFFFFFF);
						GRRLIB_DrawImg(0, 0, Spr_Wallpaper, 0, 1, 1, 0xFFFFFFFF);
						GRRLIB_DrawImg(-view_xview+512, view_yview, Spr_Wallpaper, 0, 1, 1, 0xFFFFFFFF);
						Alpha -= 5;
						if (Alpha < 6) {
							Transition_Id=-1;
							Alpha = 0;
							MenuType = 1;
							Selected = -1;
							Dragging = -1;
							Transition_Type = -1;
						};
						
						for(i=0;i<48;i++)
							ChannelSystem->Handle_Channel(MainChannels[i]);
						
						Color = GRRLIB_GetColor(0, 0, 0, Alpha);
						GRRLIB_Rectangle(0,0,640,480,Color,true);
						
					break;
					//Main Menu
					case 1: 
						//GRRLIB_FillScreen(0x505050ff);
						GRRLIB_DrawImg(0, 0, Spr_Background, 0, 1, 1, 0xFFFFFFFF);
						GRRLIB_DrawImg(-view_xview, view_yview, Spr_Wallpaper, 0, 1, 1, 0xFFFFFFFF);
						GRRLIB_DrawImg(-view_xview+512, view_yview, Spr_Wallpaper, 0, 1, 1, 0xFFFFFFFF);
						GRRLIB_DrawImg(-view_xview+1024, view_yview, Spr_Wallpaper, 0, 1, 1, 0xFFFFFFFF);
						GRRLIB_DrawImg(-view_xview+1536, view_yview, Spr_Wallpaper, 0, 1, 1, 0xFFFFFFFF);
						GRRLIB_DrawImg(-view_xview+2048, view_yview, Spr_Wallpaper, 0, 1, 1, 0xFFFFFFFF);
						GRRLIB_DrawImg(-view_xview+2560, view_yview, Spr_Wallpaper, 0, 1, 1, 0xFFFFFFFF);
						
						if (!MP3Player_IsPlaying()) SoundSystem->PlaySound(Snd_MainMenu);
						
						int Returned;
						Selected = -1;
						
						for(i=0;i<48;i++) {
							Returned = ChannelSystem->Handle_Channel(MainChannels[i]);

							if (Returned != -1)
								Selected = Returned;
						};
						
					break;
					//Channel Folderview
					case 2: 
						GRRLIB_FillScreen(0x701050ff);
						GRRLIB_DrawImg(0, 0, 
								Spr_Transition, 0, 1, 1, 0x000000FF);
					break;
				};
				if (FOCUS == -1) {
					Alpha += 3;
					if (Alpha >255) break;
					Color = GRRLIB_GetColor(0, 0, 0, Alpha);
					GRRLIB_Rectangle(0,0,640,480,Color,true);
				};
				/*
					Handle services
						FTP_Update()
						Audio_Update()
						Power_Handling()
				*/
					PlayerOne->Update();
					PlayerTwo->Update();
					PlayerThree->Update();
					PlayerFour->Update();
					if (PlayerOne->WPAD_POWER && FOCUS != -1) {
						FOCUS = -1;
						Alpha = 0;
						PlayerOne->WPAD_POWER = false;
					};
 
				/*
					Handle Transitions
				*/
				if (FOCUS == 0) {
					if (Transition_Id != -1) {
						if (MenuType!=0) {
							switch(Transition_Type) {
								case 0: TransitionZoomIn(); break;
								case 1: TransitionZoomOut(); break;
							};
						};
					} else {
						/*
							Handle Controls
						*/
						switch(MenuType) {
							case 0:
							break;
							case 1:
								if (Selected!=-1) {
									if (PlayerOne->PadDown & WPAD_BUTTON_A) {
										if (PlayerOne->PadHeld & WPAD_BUTTON_B) {
											Dragging = Selected;
											ChannelSystem->Channel_Set_Dragging(Selected,true);
										} else {
											MenuTypeTo = 2;
											Transition_Type = 0;
											Alpha = 0;
											Transition_Id=Selected;
											Transition_X=ChannelSystem->Channel_Get_X(Transition_Id);
											Transition_Y=ChannelSystem->Channel_Get_Y(Transition_Id);
											Transition_PreviousX=Transition_X;
											Transition_PreviousY=Transition_Y;
											TranX=Transition_X/255;
											TranY=Transition_Y/255;
										};
									};
								};
								if (Dragging != -1) {
									if ((PlayerOne->PadHeld & WPAD_BUTTON_A) && (PlayerOne->PadHeld & WPAD_BUTTON_A))	{
										if (PlayerOne->X > 590)
											if (view_xview == view_xview_to) view_xview_to += 512;
										if (PlayerOne->X < 50)
											if (view_xview == view_xview_to) view_xview_to -= 512;
									} else {
										ChannelSystem->Channel_Set_Dragging(Dragging,false);
									};
								};
								if (view_xview_to > 2048) view_xview_to = 2048;
								if (view_xview_to < 0) view_xview_to = 0;
								
								if (PlayerOne->PadDown & WPAD_BUTTON_PLUS) 
									if (view_xview == view_xview_to) view_xview_to += 512;
								if (PlayerOne->PadDown & WPAD_BUTTON_MINUS) 
									if (view_xview == view_xview_to) view_xview_to -= 512;
								
								if (view_xview_to > view_xview) view_hspeed += 1.5;
								if (view_xview_to < view_xview) view_hspeed -= 1.5;
								
								if (view_hspeed > 0 && view_xview > view_xview_to-view_hspeed) {
									view_hspeed = 0; view_xview = view_xview_to;
								};
								if (view_hspeed < 0 && view_xview < view_xview_to-view_hspeed) {
									view_hspeed = 0; view_xview = view_xview_to;
								};
								view_xview += view_hspeed;
							break;
							case 2:
								if (PlayerOne->PadDown & WPAD_BUTTON_B) {
									MenuTypeTo = 1;
									Transition_Type = 1;
									Alpha = 255;
									Transition_Id=Transition_Previous;
									Transition_X=0;
									Transition_Y=0;
									TranX=Transition_PreviousX/255;
									TranY=Transition_PreviousY/255;
								};
							break;
						};
					};
				};
				GUI->Step();
				GRRLIB_Render();
			};
		};
		~MainBrowser() {
			GRRLIB_FreeTexture(Spr_Transition);
			GRRLIB_FreeTexture(Spr_Wallpaper);
		};

		void TransitionZoomIn() {
			//step
			Alpha+=15;
			if (Alpha > 255) {
				SoundSystem->PlaySound(ChannelSystem->Channel_Get_Sound(Transition_Id));
				MP3Player_Volume(255);
				MODPlay_SetVolume(&modplay, 100, 100);
				Transition_Previous = Transition_Id;
				Transition_Id = -1;
			} else {
				MODPlay_SetVolume(&modplay, 100-(Alpha/2.5), 100-(Alpha/2.5));
				MP3Player_Volume(255-(Alpha));
				if (Alpha > 119 && MenuType!=MenuTypeTo) {
					MenuType = MenuTypeTo;
					ChannelSystem->SetDirectoryToChannel(Transition_Id);
				};
				if (Alpha < 128)
					Color = GRRLIB_GetColor(255,255,255,255);
				else 
					Color = GRRLIB_GetColor(255,255,255,255-Alpha);

				GRRLIB_DrawImg(Transition_X, Transition_Y, 
								Spr_BlankChannel, 0, 1+((float)Alpha/63.7), 1+((f32)Alpha/63.7), Color);

				if (Alpha < 128)
					Color = GRRLIB_GetColor(0,0,0,Alpha*2);
				else {
					Color = 0x000000FF;
				};
				GRRLIB_Rectangle(0,
								0,
								Transition_X,
								480,Color,true);
				GRRLIB_Rectangle(Transition_X,
								0,
								640-Transition_X,
								Transition_Y,Color,true);
				GRRLIB_Rectangle(Transition_X,
								Transition_Y+(480*(0.2+((f32)Alpha/318.75))),
								640-Transition_X,
								390,Color,true);
				GRRLIB_Rectangle(Transition_X+(640*(0.2+((f32)Alpha/318.75))),
								Transition_Y,
								640-Transition_X,
								(480*(0.2+((f32)Alpha/318.75))),Color,true);
				GRRLIB_DrawImg(Transition_X, Transition_Y, 
								Spr_Transition, 0, 0.2+((float)Alpha/318.75), 0.2+((f32)Alpha/318.75), Color);

				Transition_X-=TranX*15;
				Transition_Y-=TranY*15;
			};
		};
		void TransitionZoomOut() {
			//step
			Alpha-=15;
			if (Alpha < 0) {
				SoundSystem->StopSound(ChannelSystem->Channel_Get_Sound(Transition_Id));
				MP3Player_Volume(255);
				MODPlay_SetVolume(&modplay, 100, 100);
				SoundSystem->PlaySound(Snd_MainMenu);
				
				Transition_Id = -1;
				Transition_Previous=-1;
			} else {
				MODPlay_SetVolume(&modplay, (Alpha/2.5), (Alpha/2.5));
				MP3Player_Volume(Alpha);
				if (Alpha < 136 && MenuType!=MenuTypeTo) {
					MenuType = MenuTypeTo;
					
					sprintf(Current_Directory,"sys/chnl");
				};
				if (Alpha > 128)
					Color = GRRLIB_GetColor(255,255,255,255-Alpha);
				else 
					Color = GRRLIB_GetColor(255,255,255,255);

				GRRLIB_DrawImg(Transition_X, Transition_Y, 
								Spr_BlankChannel, 0, 1+((float)Alpha/63.7), 1+((f32)Alpha/63.7), Color);

				if (Alpha > 128)
					Color = 0x000000FF;
				else {
					Color = GRRLIB_GetColor(0,0,0,Alpha*2);
				};
				GRRLIB_Rectangle(0,
								0,
								Transition_X,
								480,Color,true);
				GRRLIB_Rectangle(Transition_X,
								0,
								640-Transition_X,
								Transition_Y,Color,true);
				GRRLIB_Rectangle(Transition_X,
								Transition_Y+(480*(0.2+((f32)Alpha/318.75))),
								640-Transition_X,
								390,Color,true);
				GRRLIB_Rectangle(Transition_X+(640*(0.2+((f32)Alpha/318.75))),
								Transition_Y,
								640-Transition_X,
								(480*(0.2+((f32)Alpha/318.75))),Color,true);
				GRRLIB_DrawImg(Transition_X, Transition_Y, 
								Spr_Transition, 0, 0.2+((float)Alpha/318.75), 0.2+((f32)Alpha/318.75), Color);

				Transition_X+=TranX*15;
				Transition_Y+=TranY*15;
			};
		};
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Health and Safety
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class HealthAndSafety {
	public:
		int Stage, Alpha;
		GRRLIB_texImg *Spr_Hands;
		HealthAndSafety() {
			Stage=0; Alpha=0;
			Spr_Hands = GRRLIB_LoadTextureFromFile("SYS/GFX/HANDS.png");
			while(true) {
				switch(Stage) {
					case 0:
						if (Alpha < 245)
							Alpha += 10;
						else if (PlayerOne->PadDown & WPAD_BUTTON_A) {Stage = 1; PlayerOne->Rumble_Time = 10;};
						PlayerOne->Update();
						GRRLIB_DrawImg(0, 0, Spr_Hands , 0, 1, 1, GRRLIB_GetColor(255, 255, 255, Alpha));
						
						break;
					case 1:
						if (Alpha > 10)
							Alpha -= 10;
						else Stage = 2;
						
						PlayerOne->Update();
						GRRLIB_DrawImg(0, 0, Spr_Hands , 0, 1, 1, GRRLIB_GetColor(255, 255, 255, Alpha));
						break;
				};
				GRRLIB_Render();
				if (Stage == 2) break;
			};
		};
		~HealthAndSafety() {
			GRRLIB_FreeTexture(Spr_Hands);
		};
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Intro Clip
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ChannelFadeIn {
	public:
		int Stage, Alpha_Main, Alpha, XX, YY;
		u32 Color;

		ChannelFadeIn() {
			Alpha=0;
			Alpha_Main=0;
			XX=0;
			YY=0;
			Stage=0;
			while(true) {
				switch(Stage) {
					case 0:
						Alpha = Alpha_Main;
						for(XX=64;XX<512;XX+=128)
							for(YY=60;YY<338;YY+=96)
								{
									if (Alpha < 0)
										Alpha = 0;
									if (Alpha > 255)
										Color = GRRLIB_GetColor(75, 75, 75, 255);
									else
										Color = GRRLIB_GetColor(75, 75, 75, Alpha);
									GRRLIB_DrawImg(XX, YY, Spr_BlankChannel, 0, 1, 1, Color );
									if (Alpha < 255)
										Alpha+=10;
									
								};
						Alpha_Main+=5;
						if (Alpha_Main > 280)
							Stage = 1;
					break;
					case 1:
						Alpha = Alpha_Main;
						for(XX=64;XX<512;XX+=128)
							for(YY=60;YY<338;YY+=96)
								{
									if (Alpha < 0)
										Alpha = 0;
									if (Alpha > 255)
										Color = GRRLIB_GetColor(75, 75, 75, 255);
									else
										Color = GRRLIB_GetColor(75, 75, 75, Alpha);
									GRRLIB_DrawImg(XX, YY, Spr_BlankChannel, 0, 1, 1, Color );
									Alpha-=10;
								};
						Alpha_Main-=5;
						if (Alpha_Main < 0)
							Stage = 2;
					break;
				};
				GRRLIB_Render();
				if (Stage == 2) break;
			};
		};
		~ChannelFadeIn() {};
};




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Entry Point
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main() {
    GRRLIB_Init();
	GRRLIB_SetAntiAliasing(false);
    WPAD_Init();
    WPAD_SetIdleTimeout(60);
    WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetDataFormat(WPAD_CHAN_1, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetDataFormat(WPAD_CHAN_2, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetDataFormat(WPAD_CHAN_3, WPAD_FMT_BTNS_ACC_IR);
	
	PlayerOne=new PlayerPad(WPAD_CHAN_0,"SYS/SKN/Default/Player1_Point.png");
	PlayerTwo=new PlayerPad(WPAD_CHAN_1,"SYS/SKN/Default/Player2_Point.png");
	PlayerThree=new PlayerPad(WPAD_CHAN_2,"SYS/SKN/Default/Player3_Point.png");
	PlayerFour=new PlayerPad(WPAD_CHAN_3,"SYS/SKN/Default/Player4_Point.png");
	
	HealthAndSafety *HAS = new HealthAndSafety();
	delete HAS;
	
	GUI = new GUI_System();
	
	ChannelSystem=new Channel_System();
	SoundSystem = new SND_System();
	
	Spr_BlankChannel = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/Channel.png");
	Spr_SelectedChannel = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/ChannelSelected.png");
	Spr_MaskChannel = GRRLIB_LoadTextureFromFile("SYS/SKN/Default/ChannelMask.png");

	char SND_Filename[255]="";
		sprintf(SND_Filename,"SYS/SKN/Default/SND/MainMenu.mp3");
			Snd_MainMenu = SoundSystem->AddSound(SND_Filename);
		sprintf(SND_Filename,"SYS/SKN/Default/SND/Startup.mp3");
			Snd_Startup = SoundSystem->AddSound(SND_Filename);

	ChannelFadeIn *CFI = new ChannelFadeIn();
	delete CFI;
	
	sprintf(Current_Directory,"sys/chnl");
	WPAD_SetPowerButtonCallback(PowerBttn);
	
	MainBrowser *MB = new MainBrowser();
	delete MB;
	//delete ChannelSystem; // need to fix throws exception when this is called does not make it to ~Channel_System();
	//delete SoundSystem; // need to fix - freezes when using free()
	delete GUI;
	
	delete PlayerOne;
	delete PlayerTwo;
	delete PlayerThree;
	delete PlayerFour;
	
	GRRLIB_FreeTexture(Spr_BlankChannel);
	GRRLIB_FreeTexture(Spr_SelectedChannel);
	GRRLIB_FreeTexture(Spr_Font);
	
    GRRLIB_Exit(); // Be a good boy, clear the memory allocated by GRRLIB
    return 0;
	};
//
