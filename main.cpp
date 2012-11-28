#include "DxLib.h"
#include <time.h>

#define REFRATE 48.0

int Key[256]; // キーが押されているフレーム数を格納する

// キーの入力状態を更新する
int gpUpdateKey(){
        char tmpKey[256]; // 現在のキーの入力状態を格納する
        GetHitKeyStateAll( tmpKey ); // 全てのキーの入力状態を得る
        for( int i=0; i<256; i++ ){ 
                if( tmpKey[i] != 0 ){ // i番のキーコードに対応するキーが押されていたら
                        Key[i]++;     // 加算
                } else {              // 押されていなければ
                        Key[i] = 0;   // 0にする
                }
        }
        return 0;
}

int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int){
	ChangeWindowMode(TRUE), DxLib_Init(), SetDrawScreen( DX_SCREEN_BACK ); //ウィンドウモード変更と初期化と裏画面設定

	char filename[1025];
	double BPM,base;
	char temp[1025];
	bool Adjust;
	int mode;
	int Handle;
	int play=-120,playstate,SampleNum;
	double BasePos,AdjustPos,SampleSum;
	bool AlreadySet;
	int i,j,k;
	FILE *file;
	clock_t start,end;
	file=fopen("errorlog.txt","w");

	DrawString( 120 , 60 , "Enter the path to the music file." , GetColor( 255 , 255 , 255 ) ) ;

	do {
		KeyInputString( 120 , 80 , 1024 , filename , FALSE ) ;
	} while((Handle = LoadSoundMem( filename )) == -1);

	DrawFormatString( 120 , 80 , GetColor( 255 , 255 , 255 ) , "%s" , filename );

	DrawString( 120 , 120 , "Enter BPM. (0.10-512.00)" , GetColor( 255 , 255 , 255 ) ) ;

	do {
		KeyInputSingleCharString( 120 , 140 , 1024 , temp , FALSE) ;
		BPM = atof(temp);
	} while( BPM < 0.1 || 512.0 < BPM );

	DrawFormatString( 120 , 140 , GetColor( 255 , 255 , 255 ) , "%f" , BPM );

	DrawString( 120 , 180 , "Do you want to adjust time? (y/n)" , GetColor( 255 , 255 , 255 ) ) ;
	ScreenFlip();

	do {
		WaitKey();
	} while( CheckHitKey( KEY_INPUT_Y ) == 0 && CheckHitKey( KEY_INPUT_N ) == 0 );

	if ( CheckHitKey( KEY_INPUT_Y ) == 1 ) Adjust = TRUE;
	else Adjust = FALSE;

	if (!Adjust) {

		DrawString( 120 , 220 , "Enter the base time(sec). (0.00-512.00)" , GetColor( 255 , 255 , 255 ) ) ;

		do {
			KeyInputSingleCharString( 120 , 240 , 1024 , temp , FALSE) ;
			base = atof(temp);
		} while( base < 0.0 || 512.0 < base );

		DrawFormatString( 120 , 240 , GetColor( 255 , 255 , 255 ) , "%f" , base );

		mode=0;

	} else {

		mode=1;

		play=-REFRATE*2;
		playstate=0;
		AlreadySet=FALSE;
		SampleNum=0;

	}

	DrawString( 120 , 280 , "Hit any key to go to the next step." , GetColor( 255 , 255 , 255 ) ) ;
	ScreenFlip();

	WaitKey();

	while( ScreenFlip()==0 && ProcessMessage()==0 && ClearDrawScreen()==0 && gpUpdateKey()==0 ){

		switch(mode) {
		case 1:

			switch(playstate) {
			case 0:
				if (Key[KEY_INPUT_LEFT]==1) {
					if (play-75<=-120) play=-120;
					else play-=75;
				}
				if (Key[KEY_INPUT_RIGHT]==1) {
					play+=75;
				}
				if (Key[KEY_INPUT_RETURN]==1) {
					playstate=1;
					if (play>=0) {
						SetCurrentPositionSoundMem( 44100.0/REFRATE*play, Handle ) ;
						PlaySoundMem(Handle,DX_PLAYTYPE_BACK,FALSE);
					}
				}
				break;
			case 1:
				play++;
				if (Key[KEY_INPUT_LEFT]==1) {
					if (play-75<=-120) play=-120;
					else play-=75;
					if (CheckSoundMem(Handle)) StopSoundMem(Handle);
					if (play>0) {
						SetCurrentPositionSoundMem( (int)(play/REFRATE*44100.0), Handle ) ;
						PlaySoundMem(Handle,DX_PLAYTYPE_BACK,FALSE);
					}
				}
				if (Key[KEY_INPUT_RIGHT]==1) {
					play+=75;
					if (CheckSoundMem(Handle)) StopSoundMem(Handle);
					if (play>0) {
						SetCurrentPositionSoundMem( (int)(play/REFRATE*44100.0), Handle ) ;
						PlaySoundMem(Handle,DX_PLAYTYPE_BACK,FALSE);
					}
				}
				if (Key[KEY_INPUT_RETURN]==1) {
					playstate=0;
					StopSoundMem(Handle);
					break;
				}
				if (play==0) {
					if (CheckSoundMem(Handle)) StopSoundMem(Handle);
					PlaySoundMem(Handle,DX_PLAYTYPE_BACK,TRUE);
				}
				int gettm=GetSoundCurrentTime(Handle);
				/*if (play>0 && play%(int)(REFRATE/2)==0) {
					if (abs((int)(play*20.0)-gettm)>100) {
						if ((int)(play*20.0)>gettm) {
							while((int)(play*20.0)>gettm) gettm=GetSoundCurrentTime(Handle);
							fprintf(file, "Modified. play\n", (int)(play*20.0), gettm);
						} else {
							StopSoundMem(Handle);
							SetCurrentPositionSoundMem( (int)((play*20.0)*44.10), Handle ) ;
							PlaySoundMem(Handle,DX_PLAYTYPE_BACK,FALSE);
							fprintf(file, "Modified. gettm\n", (int)(play*20.0), gettm);
						}
					}
				}*/
				fprintf(file, "play=%d gettm=%d\n", (int)(play*20.0), gettm);
				break;
			}

			if ( Key[KEY_INPUT_SPACE]==1 || Key[KEY_INPUT_DOWN]==1 ) {
				if (!AlreadySet || Key[KEY_INPUT_DOWN]==1 ) {
					AdjustPos=BasePos=(play-0.5)/REFRATE;
					AlreadySet=TRUE;
					SampleNum=0;
					SampleSum=0;
				} else {
					double NowTime=(play-0.5)/REFRATE;
					SampleSum+=NowTime-60.0/BPM*(int)(NowTime/(60.0/BPM)+0.5);
					SampleNum++;
					AdjustPos=BasePos+SampleSum/(double)SampleNum;
				}
			}

			if (play==0) start=clock();
			if (play==60) {
				end=clock();
				double second=(double)(end-start)/CLOCKS_PER_SEC;
				int intt=0;
			}


			if (AlreadySet) {
				for (i=(int)((play/REFRATE-AdjustPos-350.0/250.0)/(60.0/BPM));i<=(int)((play/REFRATE-AdjustPos+350.0/250.0)/(60.0/BPM))+1;i++) {
					if ( 1 ) {
						DrawLine( (int)((i*(60.0/BPM)-(play/REFRATE-AdjustPos))*250.0+320.0), 155,
							(int)((i*(60.0/BPM)-(play/REFRATE-AdjustPos))*250.0+320.0), 245,
							GetColor(128,128,128));
					}
				}
			}

			DrawCircle( 320 , 200 , 30 , GetColor( 255, 255, 255 ) , FALSE ) ;

			for (i=(int)((play/REFRATE*250.0-350.0)/125.0);i<=(int)((play/REFRATE*250.0+350.0)/125.0)+1;i++) {
				if ( -4 <= i ) {
					int color;
					char type[1024];
					if ( i<0 ) color=GetColor(128,128,128);
					else color=GetColor(255,255,255);
					DrawLine( (int)(i*125.0-play/REFRATE*250.0+320.0), 130,
						(int)(i*125.0-play/REFRATE*250.0+320.0), 150,
						color);
					SetFontSize( 12 );
					if ( i<0 ) sprintf(type, "-00:%02d.%01d", (-i)/2%60, (-i)%2*5);
					else sprintf(type, "%02d:%02d.%01d", i/2/60, i/2%60, i%2*5);
					DrawString( (int)(i*125.0-play/REFRATE*250.0+320.0-GetDrawStringWidth( type, strlen(type) )/2.0), 110,
						type, color);
				}
			}

			if (AlreadySet) {
				DrawFormatString( 500, 360, GetColor( 255, 255, 255 ), "%8.4f", AdjustPos );
				double height;
				double t=play/REFRATE-AdjustPos;
				double beat=60.0/BPM;
				height=1-(t-(int)(t/(60.0/BPM))*(60.0/BPM))/(60.0/BPM);
				height=height*height*height;
				DrawBox( 40, 440-160*height,
					100, 440,
					GetColor( 255, 0, 0 ), TRUE );
			}

			break;
		}

	}

	fclose(file);
        
	DxLib_End(); // DXライブラリ終了処理
	return 0;
}