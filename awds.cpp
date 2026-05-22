#include <graphics.h>//引用图形库头文件
#include <conio.h>
#include <Windows.h>
#include <mmsystem.h>
#include <exception>//异常处理头文件
#include <fstream>//文件操作
#include <string.h>//字符串处理
#include <iostream>
#include <ctime>
#include <stdlib.h>
#pragma comment(lib,"winmm.lib")

// 添加互斥锁句柄
HANDLE hMutex;

bool bgmEnabled=true;//bgm默认开启
int difficulty=1;//默认简单模式
double speed=1;//默认速度
int score=0;//得分
bool keyA=false,keyW=false,keyD=false,keyS=false;//按键状态
int left,top,right,bottom;//按键位置
DWORD timeA=0,timeW=0,timeD=0,timeS=0;//按键生成时间记录
bool gameRunning=true;//游戏是否继续
char howToEnd[20];//失败形式
char whichKeyFail;//哪一个按键导致的失败
bool gameEnd=false;//判断游戏是否结束

struct KeyRect{//存储每个按键位置的矩形区域
	int left, top, right, bottom;
}rectA,rectW,rectD,rectS;


void gameOver(char howToEnd[],char whichKeyFail){//游戏结束
	if(gameEnd) return;//防止反复调用
	gameEnd=true;

	setfillcolor(BLACK);
	setlinecolor(BLACK);
	fillrectangle(10,100,500,450);
	//清除剩余按键
	if(whichKeyFail=='a'){
		setfillcolor(RED);
		setlinecolor(BLACK);
		fillrectangle(100,250,150,300);
		outtextxy(100+15,250+8,whichKeyFail);
	}
	if(whichKeyFail=='w'){
		setfillcolor(RED);
		setlinecolor(BLACK);
		fillrectangle(225,150,275,200);
		outtextxy(225+15,150+8,whichKeyFail);
	}
	if(whichKeyFail=='d'){
		setfillcolor(RED);
		setlinecolor(BLACK);
		fillrectangle(350,250,400,300);
		outtextxy(350+15,250+8,whichKeyFail);
	}
	if(whichKeyFail=='s'){
		setfillcolor(RED);
		setlinecolor(BLACK);
		fillrectangle(225,250,275,300);
		outtextxy(225+15,250+8,whichKeyFail);
	}

	if(strcmp(howToEnd,"timeout")==0){//游戏结束以时间耗尽结束
		settextstyle(35,25,"微软雅黑");
		settextcolor(RED);
		char failc[]="Time Out! You Fail!";
		outtextxy(20,70,failc);//显示失败标签
	}

	if(strcmp(howToEnd,"fatfinger")==0){//游戏结束以按错键结束
		settextstyle(35,25,"微软雅黑");
		settextcolor(RED);
		char failc[]="Fat Finger! You Fail!";
		outtextxy(10,70,failc);//显示失败标签
	}

	settextstyle(25,15,"微软雅黑");
	char restartc[]="Press Any Key To Esc";
	outtextxy(50,110,restartc);//显示退出标签
}

void graphLoad(){
	setfillcolor(BLACK);
	setlinecolor(BLACK);
	fillrectangle(0,10,200,50);
	//用黑色背景覆盖原score

	settextstyle(30,15,"微软雅黑");//设置字体格式
	setfillcolor(WHITE);
	settextcolor(WHITE);

	char sc[]="Score:";
	char scInt[50];
	itoa(score,scInt,10);//将int转为char数组
	outtextxy(45,10,sc);//score标签
	outtextxy(145,10,scInt);//得分栏

	char dc[]="Difficulty:";
	char dInt[10];
	itoa(difficulty,dInt,10);
	outtextxy(245,10,dc);//difficulty标签
	outtextxy(395,10,dInt);//难度栏
}

void gameConfig(){//游戏配置
	FILE* file=fopen("config.txt","r");//打开配置文件
	if(file==NULL){
		return;//文件不存在,退出配置读取采用默认值
	}
	int bgmValue,diffValue;
	fscanf(file,"%d",&bgmValue);//读取第一行bgm
	fscanf(file,"%d",&diffValue);//读取第二行difficulty

	bgmEnabled=(bgmValue==1);
	difficulty=diffValue;

	fclose(file);
	if(difficulty==2){
		speed=0.7;//中等难度
	}
	if(difficulty==3){
		speed=0.5;//困难难度
	}
	if(difficulty==4){
		speed=0.1;//外挂难度
	}

}

void loadMusic(){//载入音乐
	if (!bgmEnabled){//如果bgmEnabled为false则不载入music
		return;
	}
	try{
		mciSendString("open \"bgm.mp3\" type mpegvideo alias myMusic",NULL,0,NULL);//打开音频文件
		mciSendString("play myMusic repeat",NULL,0,NULL);//设置循环播放
			
	}catch(std::exception& e){//标准异常类要全小写
		;
	}
}

void generateOne(char one){
	if (one=='a'){//a按键位置
		left=100;
		top=250;
		right=150;
		bottom=300;

		rectA.left = left;
		rectA.top = top;
		rectA.right = right;
		rectA.bottom = bottom;
		//保存a位置
	}
	if (one=='w'){//w按键位置
		left=225;
		top=150;
		right=275;
		bottom=200;
		rectW.left = left;
		rectW.top = top;
		rectW.right = right;
		rectW.bottom = bottom;
		//保存w位置
	}
	if (one=='d'){//d按键位置
		left=350;
		top=250;
		right=400;
		bottom=300;
		rectD.left = left;
		rectD.top = top;
		rectD.right = right;
		rectD.bottom = bottom;
		//保存d位置
	}
	if (one=='s'){//s按键位置
		left=225;
		top=250;
		right=275;
		bottom=300;
		rectS.left = left;
		rectS.top = top;
		rectS.right = right;
		rectS.bottom = bottom;
		//保存a位置
	}
	setfillcolor(WHITE);//填充颜色
	setlinecolor(BLACK);//边框颜色
	fillrectangle(left,top,right,bottom);//填充框架
	setbkmode(TRANSPARENT);
	settextcolor(BLACK);//文本颜色
	outtextxy(left+15,top+8,one);//按键字符

}

DWORD WINAPI thinkGenerateKey(LPVOID lpParam){//判断创建awds按键函数

	char awdsArr[]={'a','w','d','s'};//创建awds数组
	char remainingKeys[4];//存储还未生成的按键
	int remainingCount;//剩余按键数量

	srand((unsigned)time(NULL));//设置随机种子

	while(gameRunning){
		Sleep(speed*1000);//每次刷新按键等待时间

		if(!gameRunning){//再次检查游戏是否还在运行(防止最后生成一个按键)
			break;
		}
		
		WaitForSingleObject(hMutex, INFINITE);  // 加锁
		
		remainingCount=0;
		if(!keyA) remainingKeys[remainingCount++]='a';
		if(!keyW) remainingKeys[remainingCount++]='w';
		if(!keyD) remainingKeys[remainingCount++]='d';
		if(!keyS) remainingKeys[remainingCount++]='s';

		if(remainingCount==0){
			ReleaseMutex(hMutex);
			continue;//如果所有按键都生成了,跳过,等待玩家按完其他键
		}
		
		int randomIndex=rand()%remainingCount;
		char generatedKey=remainingKeys[randomIndex];
		//从未生成的按键中随机选一个

		switch(generatedKey){//生成按钮的时候记录时间
			case 'a':keyA=true;generateOne('a');timeA=GetTickCount();break;
			case 'w':keyW=true;generateOne('w');timeW=GetTickCount();break;
			case 'd':keyD=true;generateOne('d');timeD=GetTickCount();break;
			case 's':keyS=true;generateOne('s');timeS=GetTickCount();break;
		}
		
		ReleaseMutex(hMutex);  // 解锁
	}
	return NULL;//退出线程
}

void removeKey(char one){
	WaitForSingleObject(hMutex, INFINITE);
	
	setfillcolor(BLACK);
	setlinecolor(BLACK);
	//用黑色背景覆盖按键
	switch(one){
		case 'a':
			fillrectangle(rectA.left,rectA.top,rectA.right,rectA.bottom);
			break;
		case 'w':
			fillrectangle(rectW.left,rectW.top,rectW.right,rectW.bottom);
			break;
		case 's':
			fillrectangle(rectS.left,rectS.top,rectS.right,rectS.bottom);
			break;
		case 'd':
			fillrectangle(rectD.left,rectD.top,rectD.right,rectD.bottom);
			break;
	}
	
	ReleaseMutex(hMutex);
}

DWORD WINAPI timeOutFail(LPVOID IpParam){//时间耗尽失败
	while(gameRunning){
		Sleep(50);//每50ms检查一次
		
		WaitForSingleObject(hMutex, INFINITE);
		
		DWORD currentTime=GetTickCount();//记录现在时间(必须用DWORD,因为GetTickCount返回的是DWORD32位无符号整数)
		DWORD timeOut=(DWORD)(speed*3000);//超时时间=speedx3
		if(keyA && timeA!=0 && (currentTime-timeA)>timeOut){
			whichKeyFail='a';
			gameRunning=false;
			ReleaseMutex(hMutex);
			break;
		}
		if(keyW && timeW!=0 && (currentTime-timeW)>timeOut){
			whichKeyFail='w';
			gameRunning=false;
			ReleaseMutex(hMutex);
			break;
		}
		if(keyD && timeD!=0 && (currentTime-timeD)>timeOut){
			whichKeyFail='d';
			gameRunning=false;
			ReleaseMutex(hMutex);
			break;
		}
		if(keyS && timeS!=0 && (currentTime-timeS)>timeOut){
			whichKeyFail='s';
			gameRunning=false;
			ReleaseMutex(hMutex);
			break;
		}
		
		ReleaseMutex(hMutex);
		//如果超时,退出游戏继续循环
	}
	if(howToEnd[0]=='\0'){
		strcpy(howToEnd,"timeout");
		gameOver(howToEnd,whichKeyFail);//跳转到游戏结束画面
	}

	return NULL;
}

DWORD WINAPI checkKeyPress(LPVOID lpParam){//检测按键按下状态
	while(gameRunning){
		if(_kbhit()){
			char key=_getch();//获取键盘输入
			
			WaitForSingleObject(hMutex, INFINITE);
			
			switch(key){
				case 'a':
				case 'A':
					if(keyA){
						keyA=false;
						removeKey('a');//删除a按键状态
						score+=1;//加1分
						graphLoad();
					}else{//按了不存在的a键
						whichKeyFail='a';
						strcpy(howToEnd,"fatfinger");
						gameRunning=false;
						ReleaseMutex(hMutex);
						gameOver(howToEnd,whichKeyFail);
						return NULL;
					}
					break;
				case 'w':
				case 'W':
					if(keyW){
						keyW=false;
						removeKey('w');//删除w按键状态
						score+=1;//加1分
						graphLoad();
					}else{//按了不存在的w键
						whichKeyFail='w';
						strcpy(howToEnd,"fatfinger");
						gameRunning=false;
						ReleaseMutex(hMutex);
						gameOver(howToEnd,whichKeyFail);
						return NULL;
					}
					break;
			
				case 'd':
				case 'D':
					if(keyD){
						keyD=false;
						removeKey('d');//删除d按键状态
						score+=1;//加1分
						graphLoad();
					}else{//按了不存在的d键
						whichKeyFail='d';
						strcpy(howToEnd,"fatfinger");
						gameRunning=false;
						ReleaseMutex(hMutex);
						gameOver(howToEnd,whichKeyFail);
						return NULL;
					}
					break;
			
				case 's':
				case 'S':
					if(keyS){
						keyS=false;
						removeKey('s');//删除s按键状态
						score+=1;//加1分
						graphLoad();
					}else{//按了不存在的s键
						whichKeyFail='s';
						strcpy(howToEnd,"fatfinger");
						gameRunning=false;
						ReleaseMutex(hMutex);
						gameOver(howToEnd,whichKeyFail);
						return NULL;
					}
					break;
				case 27:
					gameRunning=false;
					ReleaseMutex(hMutex);
					return NULL;
			}
			
			ReleaseMutex(hMutex);
		}
		Sleep(10);
	}
	return NULL;
}

int main()
{
	initgraph(500, 400,NULL);//创建绘图窗口,大小为500x400像素
	
	// 创建互斥锁
	hMutex = CreateMutex(NULL, FALSE, NULL);

	gameConfig();//调用gameConfig函数
	loadMusic();//调用loadMusic函数

	graphLoad();//调用graphLoad函数

	// 创建Windows线程
	HANDLE hThinkThread=CreateThread(NULL,0,thinkGenerateKey,NULL,0,NULL);//创建thinkGenerateKey的线程
	HANDLE hTimeFailThread=CreateThread(NULL,0,timeOutFail,NULL,0,NULL);//创建timeOutFail的线程
	HANDLE hCheckThread=CreateThread(NULL,0,checkKeyPress, NULL,0,NULL);//创建checkKeyPress的线程

	// 等待线程结束
	WaitForSingleObject(hThinkThread,INFINITE);
	WaitForSingleObject(hTimeFailThread,INFINITE);
	WaitForSingleObject(hCheckThread,INFINITE);

	// 关闭线程句柄
	CloseHandle(hThinkThread);
	CloseHandle(hTimeFailThread);
	CloseHandle(hCheckThread);
	CloseHandle(hMutex);

	
	_getch();//按任意键继续
	closegraph();//关闭绘图窗口
	return 0;
}