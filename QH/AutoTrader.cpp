// AutoTrader.cpp : ����Լ�汾,���뾭�͹�˾���룬ʵ���ʺţ����뼴���µ���
//�Զ����涩�ĺ�ԼTICK���ݵ�\Bin\TickData�£��ļ�����Լ����_����.txt	
//
//
//
//AutoTrader.cpp : �������̨Ӧ�ó������ڵ㡣
#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <time.h>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <stdlib.h>
using namespace std;

#include ".\ThostTraderApi\ThostFtdcTraderApi.h"
#include ".\ThostTraderApi\ThostFtdcMdApi.h"
#include "TraderSpi.h"
#include "MdSpi.h"
#include "Common.h"
#include "DataSniffer.h"
#include "MyTrader.h"
#include "QH.h"
#pragma warning(disable : 4996)
// UserApi����
CThostFtdcTraderApi *pUserApi;
// MdApi����
CThostFtdcMdApi *pMdApi;

CThostFtdcMdSpi* pMdSpi;

CTraderSpi* pUserSpi;

// ���ò���
char  FRONT_ADDR_1A[] = "tcp://front111.ctp.gtjafutures.com:41205";		// ǰ�õ�ַ1����:ʵ��
char  FRONT_ADDR_1B[] = "tcp://front111.ctp.gtjafutures.com:41213";		// ǰ�õ�ַ2����:ʵ��

char  FRONT_ADDR_2A[] = "tcp://csv101.ctp.gtjafutures.com:41205";		// ǰ�õ�ַ1����:�̺�
char  FRONT_ADDR_2B[] = "tcp://csv101.ctp.gtjafutures.com:41213";		// ǰ�õ�ַ2����:�̺�

char  FRONT_ADDR_3A[] = "tcp://asp-sim2-front1.financial-trading-platform.com:26205";	// ǰ�õ�ַ3����:���� 17:00��ʼ
char  FRONT_ADDR_3B[] = "tcp://asp-sim2-md1.financial-trading-platform.com:26213";		// ǰ�õ�ַ3����:���� 17:00��ʼ

char  Trade[] = "tcp://180.168.146.187:10000";
char  Market[] = "tcp://180.168.146.187:10010";

//TThostFtdcBrokerIDType	BROKER_ID = "7090";								// ʵ�̣����͹�˾���� ��̩����=7090
TThostFtdcBrokerIDType	BROKER_ID = "9999";
TThostFtdcInvestorIDType INVESTOR_ID = "054399";						// ʵ�̣�Ͷ���ߴ���
TThostFtdcPasswordType  PASSWORD = "";							// ʵ�̣��û�����
//TThostFtdcBrokerIDType	BROKER_ID = "2030";							// ���͹�˾����:����
//TThostFtdcInvestorIDType INVESTOR_ID = "00092";						// Ͷ���ߴ���:����
//TThostFtdcPasswordType  PASSWORD = "888888";							// �û�����:����

TThostFtdcInstrumentIDType INSTRUMENT_ID = "ag1412";					// ���׺�Լ����
TThostFtdcDirectionType	DIRECTION;										// ������������
TThostFtdcOffsetFlagType MARKETState;									// ��ƽ��
TThostFtdcPriceType	LIMIT_PRICE;										// ���׼۸�

//char *ppInstrumentID[] = {"IF1406", "rb1410", "j1409", "ru1409","SR409", "m1409", "y1409", "p1409","ag1412", "cu1408"};			// ���鶩���б�
char *ppInstrumentID[] = {"ag1412"};									// ���鶩���б�
int iInstrumentID = 1;													// ���鶩������

bool	RunMode=0;														// ʵ���µ�=1,����=0��  Ӱ��Common.h�е�SendOrder()����
bool	ReceiveTick = false;

// ������
int iRequestID = 0;
// ����ʱ��
bool	JustRun = false;	//����������־

TThostFtdcDateExprType	TradingDay;

// User��������
extern CQHApp theApp;
extern	char	*InstrumentID_name;	//
extern	string	Q_BarTime_s;		//ʱ���ַ���
extern	int		Q_BarTime_1;		//ʱ��������
extern	double	Q_BarTime_2;		//ʱ���ʽ0.145100
extern	double	Q_UpperLimit;	//
extern	double	Q_LowerLimit;	//

extern	double	NewPrice;		//
extern	int		FirstVolume;	//ǰһ�γɽ�������

extern	double  Mn_open[3];		// 
extern	double  Mn_high[3];		// 
extern	double  Mn_low[3];		// 
extern	double  Mn_close[3];	// 

extern	double  BuyPrice;		//���ּ�
extern	double  SellPrice;		//���ּ�
extern	int		BNum;			//���ִ���
extern	int		SNum;			//���ִ���

extern	bool	BuySignal;		//
extern	bool	SellSignal;		//

extern	double	BSVolume;		//������

extern	int		TickABS;
extern	double  TickAPrice[4];		//
extern	int		TickBNum;
extern	double  TickBPrice[4];		//

extern	char    LogFilePaths[80];	//

// �Ự����
extern	TThostFtdcFrontIDType	FRONT_ID;	//ǰ�ñ��
extern	TThostFtdcSessionIDType	SESSION_ID;	//�Ự���
//extern	TThostFtdcOrderRefType	ORDER_REF;	//��������



void CTP(void)
{
	void Erasefiles();
	void Sniffer();
	void Trading();
	bool ReadConfiguration(char *filepaths);
	void WriteConfiguration(char *filepaths);
	
	Erasefiles();
	Sleep(2000);

	cerr << "--->>> " << "Welcom MyAutoTrader System!" << endl;
	cerr << "--->>> " << "Version 1.0.0!" << endl;
	// ��ʼ��UserApi
	pUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi("./thosttraderapi.dll");			// ����UserApi//"./thosttraderapi.dll"
	pUserSpi = new CTraderSpi();
	pUserApi->RegisterSpi((CThostFtdcTraderSpi*)pUserSpi);			// ע���¼���
	pUserApi->SubscribePublicTopic(THOST_TERT_RESTART);				// ע�ṫ����
	pUserApi->SubscribePrivateTopic(THOST_TERT_RESTART);			// ע��˽����
	pUserApi->RegisterFront(Trade);							// connect

	pUserApi->Init();
	cerr << "--->>> " << "Initialing UserApi" << endl;

	// ��ʼ��MdApi
	pMdApi = CThostFtdcMdApi::CreateFtdcMdApi("./thostmduserapi.dll");					// ����MdApi//"./thostmduserapi.dll"
	pMdSpi = new CMdSpi();
	pMdApi->RegisterSpi(pMdSpi);									// ע���¼���
	pMdApi->RegisterFront(Market);							// connect		���������ַ
	pMdApi->RegisterFront(Market);							// connect		���������ַ��1B�Ͽ����Զ�����2B��ַ

	pMdApi->Init();
	cerr << "--->>> " << "Initialing MdApi" << endl;
	//pMdApi->Join();
	//pMdApi->Release();
	
	//Sleep(2000);
	ReadConfiguration("./AutoTrader.dat");			//�Զ������ݣ���ֲ����ݵȾ���
	cerr << "--->>> " << "��ʼ�����!" << endl;
	

	//while(1)
	{
			
		//ָ�����,����ֻ�Ǹ�������
		//���Խ����������и��Ӵ���  ��DataSniffer.h
		//Sniffer();
		
		//�µ�����
		//���Խ��������������Ӵ���	��MyTrader.h
		//Trading();

		//Sleep(50);


	}

}


