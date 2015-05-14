#include 	<EMV41.h>
#include	<util.h>
#include	<include.h>
#include	<global.h>
#include <xdata.h>
#include <py_input.h>
#include <emvinterface.h>
//#include <EMVLib.h>
#if SANDREADER
#include    <QPBOC.h>
#endif

#include <iconv.h>

extern void des(uchar *binput, uchar *boutput, uchar *bkey);

unsigned char cflag,ucComm;
unsigned char  SetBankflag=0;
//unsigned char aucOperData[16];
#define COMMMAINKEY "\xC0\xEE\xD3\xF1\xB4\xA8\xD0\xB4\xB5\xC4\xCE\xA1\xBF\xB5\xBF\xA8"

//#define tCOMMPRINT

enum {COMM1=1,COMM2};


unsigned char UTIL_Tiaoshi(unsigned char *pucInData,unsigned short uiInLen)
{
	unsigned int uiI;

	for(uiI=0;uiI<uiInLen;uiI++)
	{
		if(uiI>0&&(uiI%10==0))	util_Printf("\n");
		util_Printf("%02x ",*(pucInData+uiI));
	}

		util_Printf("\n");
	return SUCCESS;
}
unsigned char UTIL_DisplayMenu(SELMENU *pMenu)
{
	unsigned char ucI,ucJ;
	unsigned char ucKBKey;
	unsigned char ucCurrLine;
	unsigned char ucCurrPage;
	unsigned char ucMaxPage;
	unsigned char aucLine[9];
	unsigned char aucDispBuf[25];


	if(!pMenu->ucMenuCnt)
		return(ERR_OSFUNC);
	ucMaxPage = pMenu->ucMenuCnt/4;
	if(pMenu->ucMenuCnt%4)
		ucMaxPage++;
	ucCurrPage = 0;
	memset(aucLine,0,sizeof(aucLine));
	do{
        //Os__clr_display(255);
		ucCurrLine =  pMenu->ucMenuCnt-ucCurrPage*4;
		for(ucI=0;ucI<4&&ucI<ucCurrLine;ucI++)
		{
			ucJ = ucI+ucCurrPage*4;
			aucLine[0] = (ucJ+1)+0x30;	/*��1��ſ�ʼ��ʾ*/
			aucLine[1] = '.';
            //Os__display(ucI,0,aucLine);
			memset(aucDispBuf,0,sizeof(aucDispBuf));
			MSG_GetMsg(pMenu->DispMenu[ucJ].uiDispIndex,aucDispBuf,sizeof(aucDispBuf));
            //Os__GB2312_display(ucI,1,aucDispBuf);
		}
		ucKBKey = Os__xget_key();
		Os__light_on();
		switch(ucKBKey)
		{
			case KEY_ENTER:
				ucCurrPage++;
				if(ucCurrPage>=ucMaxPage)
					ucCurrPage = 0;
				break;
			case KEY_CLEAR:
				return(ERR_END);
			case KEY_F3:
				ucCurrPage++;
				if(ucCurrPage>=ucMaxPage)
					ucCurrPage = 0;
				break;
			case KEY_F4:
				if(ucCurrPage)
					ucCurrPage--;
				else
					ucCurrPage = ucMaxPage-1;
				break;
			case KEY_PAPER_FEED:
				PRINT_xlinefeed(6*LINENUM);
				continue;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				ucKBKey = ucKBKey&0x0F;
				if(SetBankflag)
		                {
		                    Os__saved_set(&DataSave0.ConstantParamData.BankID, ucKBKey,sizeof(unsigned char));
		                    XDATA_Write_Vaild_File(DataSaveConstant);
		                }
				// chenzx add 2005-9-28 17:09
				G_NORMALTRANS_ucTransType = pMenu->DispMenu[ucKBKey-1].ucTransType;
				if(ucKBKey<=pMenu->ucMenuCnt)
				{
					if( pMenu->DispMenu[ucKBKey-1].pfnProc )
					{
						return((*(pMenu->DispMenu[ucKBKey-1].pfnProc))());
					}
				}
				break;
			default:
				break;
		}
	}while(1);
}

unsigned char UTIL_InputAmount(unsigned char ucLine,unsigned long *pulAmount,
				unsigned long ulMinAmount, unsigned long ulMaxAmount)
{
	 unsigned char aucDisp[CFG_MAXDISPCHAR+1];  		/* Buffer for Display */
	 unsigned char aucInput[CFG_MAXAMOUNTLEN+1]; 		/* Buffer for Key Input */
	 unsigned char aucInputTmp1[CFG_MAXAMOUNTLEN+1],aucInputTmp2[CFG_MAXAMOUNTLEN+1],aucInputTmp3[CFG_MAXAMOUNTLEN+1]; /* Buffer for Key Input */
	 unsigned char ucKey;
	 unsigned char ucRedraw;
	 unsigned char ucCount;
	 unsigned char ucI;
	 unsigned char ucJ;
	 DRV_OUT *pdKey;
	 unsigned int uiTimeout;


     //Os__GB2312_display(3, 0, (uchar *)"����밴[���]��");
	 memset(&aucDisp[0],0,sizeof(aucDisp));
	 memset(&aucDisp[0],' ',CFG_MAXDISPCHAR);
	 UTIL_Form_Montant(&aucDisp[5],*pulAmount,2);
	 util_Printf( "aucDisp=%s\n",aucDisp );

	 memset(&aucInput[0],0,sizeof(aucInput));
	 long_asc( aucInput,10,pulAmount );
	 ucRedraw = TRUE;
	 uiTimeout = 50*ONESECOND;
	 ucCount = 10;
	 while(aucInput[10-ucCount]=='0'&&ucCount>0) ucCount--;
	// util_Printf("\naucInput:%s   ucCount:%x",aucInput,ucCount);
	 for(ucI=0;ucI<ucCount;ucI++)
	  aucInput[ucI]=aucInput[ucI+10-ucCount];
	 aucInput[ucCount]=0;
	// util_Printf("\naucInput1:%s   ucCount:%x",aucInput,ucCount);


	 do
	 {
	 Os__timer_start(&uiTimeout);
		  if( ucRedraw == TRUE)
		  {
           //Os__clr_display(ucLine);
           //Os__display(ucLine,0,&aucDisp[0]);
		   ucRedraw = FALSE;
		  }
		  pdKey = Os__get_key();
		  do{
		  }while(  (pdKey->gen_status==DRV_RUNNING)
			  &&(uiTimeout !=0)
			  );
		  if (uiTimeout == 0)
		  {
		   Os__abort_drv(drv_mmi);
		   return(ERR_CANCEL);
		  }else
		  {
		   ucKey = pdKey->xxdata[1];
		  }
		  Os__timer_stop(&uiTimeout);
		  switch(ucKey)
		  {
		  case KEY_CLEAR:
			   if( ucCount )
			   {
					ucCount = 0;
					memset(&aucDisp[0],0,sizeof(aucDisp));
					memset(&aucDisp[0],' ',CFG_MAXDISPCHAR);
					aucDisp[CFG_MAXDISPCHAR-1] = '0';
					aucDisp[CFG_MAXDISPCHAR-2] = '0';
					aucDisp[CFG_MAXDISPCHAR-3] = '.';
					aucDisp[CFG_MAXDISPCHAR-4] = '0';
					ucRedraw = TRUE;
			   }else
			   {
					return(ERR_CANCEL);
			   }
			   break;
		  case KEY_BCKSP:
				if( ucCount )
				{
					ucCount --;
					aucInput[ucCount] = 0;
					ucRedraw = TRUE;
				}
				break;
			case KEY_ENTER:
				memset(aucInputTmp1,0,sizeof(aucInputTmp1));
				long_asc(aucInputTmp1,CFG_MAXAMOUNTLEN,&ulMaxAmount);
				memset(aucInputTmp2,0,sizeof(aucInputTmp2));
				memset(aucInputTmp2,'0',CFG_MAXAMOUNTLEN);
				memcpy(&aucInputTmp2[CFG_MAXAMOUNTLEN-ucCount],aucInput,ucCount);
				memset(aucInputTmp3,0,sizeof(aucInputTmp3));
				long_asc(aucInputTmp3,CFG_MAXAMOUNTLEN,&ulMinAmount);
				if(memcmp(aucInputTmp2,aucInputTmp3,CFG_MAXAMOUNTLEN)<0)
				{
					Os__beep();
				}else
				{
					if(memcmp(aucInputTmp1,aucInputTmp2,CFG_MAXAMOUNTLEN)<0)
					{
		 				Os__beep();
					}else
					{
						 *pulAmount = asc_long(aucInput,ucCount);
						 return(SUCCESS);
					}
				}
				break;
		  case KEY_00_PT:
	  			if(ucCount ==0 ) continue;
				if( ucCount < CFG_MAXAMOUNTLEN)
				{
					 aucInput[ucCount] = '0';
					 ucCount ++;
					 if( ucCount < CFG_MAXAMOUNTLEN)
					 {
						  aucInput[ucCount] = '0';
						  ucCount ++;
					 }
					 ucRedraw = TRUE;
				}
				break;
		  default :
			   if(  (ucKey <= '9')
				  &&(ucKey >= '0')
				 )
			   {
					if(ucCount ==0 &&ucKey=='0') continue;
					if( ucCount < CFG_MAXAMOUNTLEN)
					{
						 aucInput[ucCount] = ucKey;
						 ucCount ++;
						 ucRedraw = TRUE;
					}
			   }
		   break;
		  }
		  /* Copy data from Input buffer to Display buffer */
		  memset(&aucDisp[0],0,sizeof(aucDisp));
		  memset(&aucDisp[0],' ',CFG_MAXDISPCHAR);
		  aucDisp[CFG_MAXDISPCHAR-1] = '0';
		  aucDisp[CFG_MAXDISPCHAR-2] = '0';
		  aucDisp[CFG_MAXDISPCHAR-3] = '.';
		  aucDisp[CFG_MAXDISPCHAR-4] = '0';
		  if( ucCount )
		  {
			   for(ucI=0,ucJ=0;ucI<ucCount;ucI++,ucJ++)
			   {
					if( ucJ == 2)
					{
						ucJ ++;
					}
					aucDisp[CFG_MAXDISPCHAR-ucJ-1] = aucInput[ucCount-ucI-1];
				}
		  }
	 }while(1);
}


unsigned char UTIL_Input_EMV_pp(unsigned char ucLine,unsigned char ucClrFlag,
				unsigned char ucMin, unsigned char ucMax,
				unsigned char ucType,
				unsigned char *pucBuf)

{
	unsigned char	aucDisp[40];	/* Buffer for Display */
	unsigned char	aucInput[40];	/* Buffer for Key Input */
	unsigned char	ucKey;
	unsigned char	ucLastKey;
	unsigned char	ucEnd;
	unsigned char	ucRedraw;
	unsigned char	ucCount;
	DRV_OUT *pdKey;
	static unsigned int	uiTimeout;
	static unsigned int	uiLastTime;

	memset(&aucDisp[0],0,sizeof(aucDisp));
	memset(&aucInput[0],0,sizeof(aucInput));
	ucLastKey = 0;
	ucEnd = FALSE;
	ucRedraw = FALSE;
	uiTimeout = 50*ONESECOND;
	uiLastTime = uiTimeout;
	ucCount = 0;

	if( ucClrFlag )
	{
		//if((!ReaderSupport)||(ReaderType!=READER_SAND))
		{
			Os__clr_display_pp(ucLine);
		}
	}
//	UTIL_display_amount(0,G_NORMALTRANS_ulAmount);
	if( strlen((char *)pucBuf))
	{
		ucRedraw = TRUE;
		ucCount = strlen((char *)pucBuf);
		if( ucCount > sizeof(aucInput))
			ucCount = sizeof(aucInput);
		memcpy(aucInput,pucBuf,ucCount);
	}


	do
	{
	Os__timer_start(&uiTimeout);
		if( ucRedraw == TRUE)
		{
			memset(&aucDisp[0],0,sizeof(aucDisp));
			if( ucCount > 15)
			{
				if(  (ucType == 'p')
				   ||(ucType == 'P')
				  )
				{
					memset(&aucDisp[0],'*',15);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[ucCount-15],15);
				}
			}else
			{
				if(  (ucType == 'p')
				   ||(ucType == 'P')
				  )
				{
					if(ucCount == 0)
					{
						memset(aucDisp,' ',sizeof(aucDisp));
						aucDisp[15] = 0x00;
					}
					else
						memset(&aucDisp[0],'*',ucCount);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[0],ucCount);
				}
			}
			Os__clr_display_pp(ucLine);
			Os__display_pp(ucLine,0,&aucDisp[0]);
			ucRedraw = FALSE;
		}
//		util_Printf("\nOs__get_key_pp");
		pdKey = Os__get_key_pp();

		do{
		}while(  (pdKey->gen_status==DRV_RUNNING)
			   &&(uiTimeout !=0)
			   );

		if (uiTimeout == 0)
		{
			Os__abort_drv(drv_pad);
			Os__xdelay(10);
			ucKey = ERR_END;
			break;
		}else
		{
			ucKey = pdKey->xxdata[1];
		}
		if(  (ucKey >= '0')
		   &&(ucKey <= '9')
		  )
		{
			if( ucCount >= ucMax)
			{
				Os__beep();
				continue;
			}
			aucInput[ucCount++] = ucKey;
			ucRedraw = TRUE;
		}else
		{
			switch(ucKey)
			{
			case KEY_CLEAR:
				if( ucCount )
				{
					ucCount = 0;
					ucLastKey = 0;
					memset(&aucInput[0],0,sizeof(aucInput));
					memset(&aucDisp[0],0,sizeof(aucDisp));
					ucRedraw = TRUE;
				}else
				{
					ucKey = ERR_END;
					ucEnd = TRUE;
				}
				break;
			case KEY_ENTER:
				if(ucCount == 0)
				{
					memset((char *)pucBuf,0x00,sizeof(pucBuf));
					ucEnd = TRUE;
					ucKey = ERR_CANCEL;
				}
				else
				if( ucCount < ucMin )
				{
					Os__beep();
				}else
				{
					memset((char *)pucBuf,0x00,ucMax);
					strcpy((char *)pucBuf,(char *)&aucInput[0]);
					ucEnd = TRUE;
				}
				break;
			case KEY_BCKSP:
				if( ucCount )
				{
					ucCount --;
					aucInput[ucCount] = 0;
					ucRedraw = TRUE;
				}
				break;
			default :
				break;
			}
		}
		uiLastTime = uiTimeout;
		if( ucEnd == TRUE)
		{
			break;
		}
	}while(1);

	Os__timer_stop(&uiTimeout);


	return(ucKey);
}



unsigned char UTIL_Input_EMV_PIN(unsigned char ucLine,unsigned char ucClrFlag,
				unsigned char ucMin, unsigned char ucMax,
				unsigned char ucType,
				unsigned char *pucBuf,
				unsigned char *pucMask)
{
	const unsigned char aucKeyTab[][7]
	={
//		{"0 *\0"},
		{"0 *\0"},
		{"1QZ\0"},
		{"2ABC\0"},
		{"3DEF\0"},
		{"4GHI\0"},
		{"5JKL\0"},
		{"6MNO\0"},
		{"7PRS\0"},
		{"8TUV\0"},
		{"9WXY\0"},
		{"0,.-\0"}
	};

	unsigned char	aucDisp[40];	/* Buffer for Display */
	unsigned char	aucInput[40];	/* Buffer for Key Input */
	unsigned char	ucKey;
	unsigned char	ucLastKey;
	unsigned char	ucKeyTabOffset;
	unsigned char	ucEnd;
	unsigned char	ucRedraw;
	unsigned char	ucCount;
	unsigned char	ucOffset;
	DRV_OUT *pKey;
	static unsigned int	uiTimeout;
	static unsigned int	uiLastTime;
	memset(&aucDisp[0],0,sizeof(aucDisp));
	memset(&aucInput[0],0,sizeof(aucInput));
	ucLastKey = 0;
	ucEnd = FALSE;
//	ucRedraw = FALSE;
	ucRedraw = TRUE;
	uiTimeout = 50*ONESECOND;
	uiLastTime = uiTimeout;
	ucCount = 0;
	if( ucClrFlag )
	{
        //Os__clr_display(ucLine);
	}
	if( strlen((char *)pucBuf))
	{
		ucRedraw = TRUE;
		ucCount = strlen((char *)pucBuf);
		if( ucCount > sizeof(aucInput))
			ucCount = sizeof(aucInput);
		memcpy(aucInput,pucBuf,ucCount);
	}


//	 util_Printf( "9999\n" );

	do
	{
	Os__timer_start(&uiTimeout);
		if( ucRedraw == TRUE)
		{
			memset(&aucDisp[0],0,sizeof(aucDisp));
			if( ucCount > MAXLINECHARNUM)
			{
				if(  (ucType == 'p')
				   ||(ucType == 'P')
				  )
				{
					memset(&aucDisp[0],'*',MAXLINECHARNUM);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[ucCount-MAXLINECHARNUM],MAXLINECHARNUM);
				}
				aucDisp[ucCount] = '_';
			}else
			{
				if(  (ucType == 'p')
				   ||(ucType == 'P')
				  )
				{
					memset(&aucDisp[0],'*',ucCount);
				}else if(  (ucType == 'a')
				   ||(ucType == 'A')
				  )
				{
					if(ucCount>=ucMax)
						memcpy(&aucDisp[0],&aucInput[0],ucCount-1);
					else
						memcpy(&aucDisp[0],&aucInput[0],ucCount);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[0],ucCount);
				}
				aucDisp[ucCount] = '_';
			}
            //Os__clr_display(ucLine);
            //Os__display(ucLine,0,&aucDisp[0]);
			ucRedraw = FALSE;
		}
//	 util_Printf( "7999\n" );

		pKey = Os__get_key();
//	 util_Printf( "8999\n" );

		do{
		}while(  (pKey->gen_status==DRV_RUNNING)
			   &&(uiTimeout !=0)
			   );

		if (uiTimeout == 0)
		{
			Os__abort_drv(drv_mmi);
			ucKey = ERR_END;
			break;
		}else
		{
			uiTimeout = 50*ONESECOND;
			ucKey = pKey->xxdata[1];
	//		util_Printf("\n ucKey=%02x\n",ucKey);

		}
		if(  (ucKey >= '0')
		   &&(ucKey <= '9')
		  )
		{
			if( pucMask )
			{
				if( !strchr((char *)pucMask, ucKey ))
					continue;
			}
			switch(ucType)
			{
            case 'h':
			case 'H':
				if( ucCount >= ucMax)
				{
					Os__beep();
					break;
				}
				if( ucLastKey != ucKey)
				{
					ucLastKey = ucKey;
					ucKeyTabOffset = 0;
					aucInput[ucCount++] = ucKey;
					ucRedraw = TRUE;
				}else
				{
                    if(  (uiLastTime-uiTimeout) < 1*ONESECOND)
					{
                        if( ucCount )
						{
							ucOffset = ucCount - 1;
						}else
						{
							ucOffset = 0;
						}
                        if(  (ucKey >= '2')
						   &&(ucKey <= '3')
						  )
						{
                            if( ucKeyTabOffset < strlen((char *)aucKeyTab[ucKey-'0'])-1)
							{
								ucKeyTabOffset ++;
								ucRedraw = TRUE;
							}else
							{
								if( ucKeyTabOffset )
								{
									ucKeyTabOffset = 0;
									ucRedraw = TRUE;
								}
							}
							aucInput[ucOffset] = aucKeyTab[ucKey-'0'][ucKeyTabOffset];
                        }else
						{
							ucLastKey = ucKey;
							ucKeyTabOffset = 0;
							aucInput[ucCount++] = ucKey;
							ucRedraw = TRUE;
						}
                    }else
					{
						if( ucCount >= ucMax)
						{
							Os__beep();
							break;
						}
						ucLastKey = ucKey;
						ucKeyTabOffset = 0;
						aucInput[ucCount++] = ucKey;
						ucRedraw = TRUE;
					}
                }
				break;
            case 'a':
			case 'A':
				if( ucCount > ucMax)
				{
					Os__beep();
					break;
				}
				if( ucLastKey != ucKey)
				{
			//		 util_Printf( "4999\n" );
					ucLastKey = ucKey;
					ucKeyTabOffset = 0;
					aucInput[ucCount++] = ucKey;
					ucRedraw = TRUE;
				}else
				{
					if(  (uiLastTime-uiTimeout) < 1*ONESECOND)
					{
						if( ucCount )
						{
							ucOffset = ucCount - 1;
						}else
						{
							ucOffset = 0;
						}
						if( ucKeyTabOffset < strlen((char *)aucKeyTab[ucKey-'0'])-1)
						{
							ucKeyTabOffset ++;
							ucRedraw = TRUE;
						}else
						{
							if( ucKeyTabOffset != 0)
							{
								ucKeyTabOffset = 0;
								ucRedraw = TRUE;
							}
						}
						aucInput[ucOffset] = aucKeyTab[ucKey-'0'][ucKeyTabOffset];
					}else
					{
						if( ucCount >= ucMax)
						{
							Os__beep();
							break;
						}
						ucLastKey = ucKey;
						ucKeyTabOffset = 0;
						aucInput[ucCount++] = ucKey;
						ucRedraw = TRUE;
					}
				}
				break;
            default:
				if( ucCount >= ucMax)
				{
					Os__beep();
					break;
				}
				aucInput[ucCount++] = ucKey;
				ucRedraw = TRUE;
				break;
			}
		}else
		{
			switch(ucKey)
			{
			case KEY_CLEAR:
				if( ucCount )
				{
					ucCount = 0;
					ucLastKey = 0;
					memset(&aucInput[0],0,sizeof(aucInput));
					memset(&aucDisp[0],0,sizeof(aucDisp));
					ucRedraw = TRUE;
				}else
				{
					ucKey = ERR_END;
					ucEnd = TRUE;
				}
				break;
			case KEY_00_PT:
//			case 0x00:
				if(  (ucType == 'h')
				   ||(ucType == 'H')
				   ||(ucType == 'a')
				   ||(ucType == 'A')
				  )
				{
//					if( ucCount < strlen((char *)&aucInput[0]))
//						ucCount ++;
					if( ucCount >= ucMax)
					{
						Os__beep();
						break;
					}
					if( ucLastKey != ucKey)
					{
						ucLastKey = ucKey;
						ucKeyTabOffset = 0;
						if((ucKey != '0')&&(ucKey != ',')&&(ucKey != '.'))
							aucInput[ucCount++] = '0';
						else
							aucInput[ucCount++] = ucKey;
						ucRedraw = TRUE;
					}else
					{

						if(  (uiLastTime-uiTimeout) < 1*ONESECOND)
						{
							if( ucCount )
							{
								ucOffset = ucCount - 1;
							}else
							{
								ucOffset = 0;
							}
							if( ucKeyTabOffset < strlen((char *)aucKeyTab[10])-1)
							{
								ucKeyTabOffset ++;
								ucRedraw = TRUE;
							}else
							{
								if( ucKeyTabOffset != 0)
								{
									ucKeyTabOffset = 0;
									ucRedraw = TRUE;
								}
							}
							aucInput[ucOffset] = aucKeyTab[10][ucKeyTabOffset];
						}else
						{
							if( ucCount >= ucMax)
							{
								Os__beep();
								break;
							}
							ucLastKey = ucKey;
							ucKeyTabOffset = 0;
							if((ucKey != '0')&&(ucKey != ',')&&(ucKey != '.'))
								aucInput[ucCount++] = '0';
							else
								aucInput[ucCount++] = ucKey;
							ucRedraw = TRUE;
						}
					}
				}else
				{
					if( ucCount > (ucMax-2))
					{
						Os__beep();
						break;
					}
//					aucInput[ucCount] = ucKey;
					memcpy(&aucInput[ucCount],"00",2);
					ucCount = ucCount +2;
					ucRedraw = TRUE;
					break;
				}
				break;
			case KEY_BCKSP:
				if( ucCount )
				{
				//������2007-01-17
					ucLastKey = 0;

					ucCount --;
					aucInput[ucCount] = 0;
					ucRedraw = TRUE;
				}
				break;
			case KEY_ENTER:
				if(ucCount == 0)
				{
					memset((char *)pucBuf,0x00,sizeof(pucBuf));
					ucEnd = TRUE;
					ucKey = ERR_CANCEL;
				}
				else
				if( ucCount < ucMin )
				{
					Os__beep();
				}else
				{
					memset((char *)pucBuf,0x00,ucMax);
					strcpy((char *)pucBuf,(char *)&aucInput[0]);
					ucEnd = TRUE;
				}
				break;
			default :
				break;
			}
	}
		uiLastTime = uiTimeout;
		if( ucEnd == TRUE)
		{
			break;
		}
	}while(1);

	Os__timer_stop(&uiTimeout);
	return(ucKey);
}


unsigned char UTIL_Input_pp(unsigned char ucLine,unsigned char ucClrFlag,
				unsigned char ucMin, unsigned char ucMax,
				unsigned char ucType,
				unsigned char *pucBuf)

{
	unsigned char	aucDisp[40];	/* Buffer for Display */
	unsigned char	aucInput[40];	/* Buffer for Key Input */
	unsigned char	ucKey;
	unsigned char	ucLastKey;
	unsigned char	ucEnd;
	unsigned char	ucRedraw;
	unsigned char	ucCount;
	DRV_OUT *pdKey;
	static unsigned int	uiTimeout;
	static unsigned int	uiLastTime;
	memset(&aucDisp[0],0,sizeof(aucDisp));
	memset(&aucInput[0],0,sizeof(aucInput));
	ucLastKey = 0;
	ucEnd = FALSE;
	ucRedraw = FALSE;
	uiTimeout = 50*ONESECOND;
	uiLastTime = uiTimeout;
	ucCount = 0;

	if( ucClrFlag )
	{
		Os__clr_display_pp(ucLine);
	}

	if( strlen((char *)pucBuf))
	{
		ucRedraw = TRUE;
		ucCount = strlen((char *)pucBuf);
		if( ucCount > sizeof(aucInput))
			ucCount = sizeof(aucInput);
		memcpy(aucInput,pucBuf,ucCount);
	}


	do
	{
	Os__timer_start(&uiTimeout);
		if( ucRedraw == TRUE)
		{
			memset(&aucDisp[0],0,sizeof(aucDisp));
			if( ucCount > 15)
			{
				if(  (ucType == 'p')
				   ||(ucType == 'P')
				  )
				{
					memset(&aucDisp[0],'*',15);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[ucCount-15],15);
				}
			}else
			{
				if(  (ucType == 'p')
				   ||(ucType == 'P')
				  )
				{
					if(ucCount == 0)
					{
						memset(aucDisp,' ',sizeof(aucDisp));
						aucDisp[15] = 0x00;
					}
					else
						memset(&aucDisp[0],'*',ucCount);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[0],ucCount);
				}
			}
			Os__clr_display_pp(ucLine);
			Os__display_pp(ucLine,0,&aucDisp[0]);
			ucRedraw = FALSE;
		}
//		util_Printf("\nOs__get_key_pp");
		pdKey = Os__get_key_pp();

		do{
		}while(  (pdKey->gen_status==DRV_RUNNING)
			   &&(uiTimeout !=0)
			   );

		if (uiTimeout == 0)
		{
			Os__abort_drv(drv_pad);
			Os__xdelay(10);
			ucKey = ERR_END;
			break;
		}else
		{
			ucKey = pdKey->xxdata[1];
		}
		if(  (ucKey >= '0')
		   &&(ucKey <= '9')
		  )
		{
			if( ucCount >= ucMax)
			{
				Os__beep();
				continue;
			}
			aucInput[ucCount++] = ucKey;
			ucRedraw = TRUE;
		}else
		{
			switch(ucKey)
			{
			case KEY_CLEAR:
				if( ucCount )
				{
					ucCount = 0;
					ucLastKey = 0;
					memset(&aucInput[0],0,sizeof(aucInput));
					memset(&aucDisp[0],0,sizeof(aucDisp));
					ucRedraw = TRUE;
				}else
				{
					ucKey = ERR_END;
					ucEnd = TRUE;
				}
				break;
			case KEY_ENTER:
				/*
				if(ucCount == 0)
				{
					memset((char *)pucBuf,0x00,sizeof(pucBuf));
					ucEnd = TRUE;
					ucKey = ERR_CANCEL;
				}
				else*/
				if(ucCount == 0)
					ucEnd = TRUE;
				if( ucCount < ucMin )
				{
					Os__beep();
				}else
				{
					memset((char *)pucBuf,0x00,ucMax);
					strcpy((char *)pucBuf,(char *)&aucInput[0]);
					ucEnd = TRUE;
				}
				break;
			case KEY_BCKSP:
				if( ucCount )
				{
					ucCount --;
					aucInput[ucCount] = 0;
					ucRedraw = TRUE;
				}
				break;
			default :
				break;
			}
		}
		uiLastTime = uiTimeout;
		if( ucEnd == TRUE)
		{
			break;
		}
		util_Printf("uiTimeout\n");
	}while(1);

	Os__timer_stop(&uiTimeout);

	util_Printf("uiTimeout = %u\n", uiTimeout);

	return(ucKey);
}


unsigned char UTIL_Input_EMV(ALIGN enAlign,unsigned char ucLine,unsigned char ucClrFlag,
				unsigned char ucMin, unsigned char ucMax,
				unsigned char ucType,
				unsigned char *pucBuf,
				unsigned char *pucMask)
{
    const unsigned char aucKeyTab[][7]
	={
//		{"0 *\0"},
		{"0 *\0"},
		{"1QZ\0"},
		{"2ABC\0"},
		{"3DEF\0"},
		{"4GHI\0"},
		{"5JKL\0"},
		{"6MNO\0"},
		{"7PRS\0"},
		{"8TUV\0"},
		{"9WXY\0"},
		{"0,.\0"}
	};

	unsigned char	aucDisp[40];	/* Buffer for Display */
	unsigned char	aucInput[40];	/* Buffer for Key Input */
	unsigned char	ucKey;
	unsigned char	ucLastKey;
	unsigned char	ucKeyTabOffset;
	unsigned char	ucEnd;
	unsigned char	ucRedraw;
	unsigned char	ucCount;
	unsigned char	ucOffset;
	DRV_OUT *pKey;
	static unsigned int	uiTimeout;
	static unsigned int	uiLastTime;
	memset(&aucDisp[0],0,sizeof(aucDisp));
	memset(&aucInput[0],0,sizeof(aucInput));
	ucLastKey = 0;
	ucEnd = FALSE;
//	ucRedraw = FALSE;
	ucRedraw = TRUE;
	uiTimeout = 50*ONESECOND;
	uiLastTime = uiTimeout;
	ucCount = 0;
	if( ucClrFlag )
	{
        //Os__clr_display(ucLine);
	}
	if( strlen((char *)pucBuf))
	{
		ucRedraw = TRUE;
		ucCount = strlen((char *)pucBuf);
		if( ucCount > sizeof(aucInput))
			ucCount = sizeof(aucInput);
		memcpy(aucInput,pucBuf,ucCount);
	}



	do
	{
	Os__timer_start(&uiTimeout);
		if( ucRedraw == TRUE)
		{
			memset(&aucDisp[0],0,sizeof(aucDisp));
			if( ucCount > MAXLINECHARNUM)
			{
				if(  (ucType == 'p')
				   ||(ucType == 'P')
				  )
				{
					memset(&aucDisp[0],'*',MAXLINECHARNUM);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[ucCount-MAXLINECHARNUM],MAXLINECHARNUM);
				}
				aucDisp[ucCount] = '_';
			}else
			{
				if(  (ucType == 'p')
				   ||(ucType == 'P')
				  )
				{
					memset(&aucDisp[0],'*',ucCount);
				}else if(  (ucType == 'a')
				   ||(ucType == 'A')
				  )
				{
					if(ucCount>=ucMax)
						memcpy(&aucDisp[0],&aucInput[0],ucCount-1);
					else
						memcpy(&aucDisp[0],&aucInput[0],ucCount);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[0],ucCount);
				}
				aucDisp[ucCount] = '_';
			}
            //Os__clr_display(ucLine);
			switch(enAlign)
			{
			case LEFT_ALIGN:
                //Os__display(ucLine,0,&aucDisp[0]);
				break;
			case MIDDLE_ALIGN:
                //Os__display(ucLine,(MAXLINECHARNUM-strlen((char*)aucDisp)+1)/2,&aucDisp[0]);
				break;
			case RIGHT_ALIGN:
                //Os__display(ucLine,MAXLINECHARNUM-strlen((char*)aucDisp)+1,&aucDisp[0]);
				break;
			}
			ucRedraw = FALSE;
		}

		pKey = Os__get_key();

		do{
		}while(  (pKey->gen_status==DRV_RUNNING)
			   &&(uiTimeout !=0)
			   );

		if (uiTimeout == 0)
		{
			Os__abort_drv(drv_mmi);
			ucKey = EMVERROR_CANCEL;
			break;
		}else
		{
			ucKey = pKey->xxdata[1];
//			util_Printf("\n Key Value %02x",ucKey);
		}
		if(  (ucKey >= '0')
		   &&(ucKey <= '9')
		  )
		{
			if( pucMask )
			{
				if( !strchr((char *)pucMask, ucKey ))
					continue;
			}
			switch(ucType)
			{
            case 'h':
			case 'H':
				if( ucLastKey != ucKey)
				{
					if( ucCount >= ucMax)
					{
						Os__beep();
						break;
					}
					ucLastKey = ucKey;
					ucKeyTabOffset = 0;
					aucInput[ucCount++] = ucKey;
					ucRedraw = TRUE;
				}else
				{
                    if(  (uiLastTime-uiTimeout) < 1*ONESECOND)
					{
                        if( ucCount )
						{
							ucOffset = ucCount - 1;
						}else
						{
							ucOffset = 0;
						}
                        if(  (ucKey >= '2')
						   &&(ucKey <= '3')
						  )
						{
                            if( ucKeyTabOffset < strlen((char *)aucKeyTab[ucKey-'0'])-1)
							{
								ucKeyTabOffset ++;
								ucRedraw = TRUE;
							}else
							{
								if( ucKeyTabOffset )
								{
									ucKeyTabOffset = 0;
									ucRedraw = TRUE;
								}
							}
							aucInput[ucOffset] = aucKeyTab[ucKey-'0'][ucKeyTabOffset];
							ucLastKey = ucKey;
                        }else
						{
							if( ucCount >= ucMax)
							{
								Os__beep();
								break;
							}
							ucLastKey = ucKey;
							ucKeyTabOffset = 0;
							aucInput[ucCount++] = ucKey;
							ucRedraw = TRUE;
						}
                    }else
					{
						if( ucCount >= ucMax)
						{
							Os__beep();
							break;
						}
						ucLastKey = ucKey;
						ucKeyTabOffset = 0;
						aucInput[ucCount++] = ucKey;
						ucRedraw = TRUE;
					}
                }
				break;
            case 'a':
			case 'A':
				if( ucLastKey != ucKey)
				{
					if( ucCount >= ucMax)
					{
						Os__beep();
						break;
					}
					ucLastKey = ucKey;
					ucKeyTabOffset = 0;
					aucInput[ucCount++] = ucKey;
					ucRedraw = TRUE;
				}else
				{
					if(  (uiLastTime-uiTimeout) < 1*ONESECOND)
					{
						if( ucCount )
						{
							ucOffset = ucCount - 1;
						}else
						{
							ucOffset = 0;
						}
						if( ucKeyTabOffset < strlen((char *)aucKeyTab[ucKey-'0'])-1)
						{
							ucKeyTabOffset ++;
							ucRedraw = TRUE;
						}else
						{
							if( ucKeyTabOffset != 0)
							{
								ucKeyTabOffset = 0;
								ucRedraw = TRUE;
							}
						}
						aucInput[ucOffset] = aucKeyTab[ucKey-'0'][ucKeyTabOffset];
					}else
					{
						if( ucCount >=ucMax)
						{
							Os__beep();
							break;
						}
						ucLastKey = ucKey;
						ucKeyTabOffset = 0;
						aucInput[ucCount++] = ucKey;
						ucRedraw = TRUE;
					}
				}
				break;
            default:
				if( ucCount >=ucMax)
				{
					Os__beep();
					break;
				}
				aucInput[ucCount++] = ucKey;
				ucRedraw = TRUE;
				break;
			}
		}else
		{
			switch(ucKey)
			{
			case KEY_CLEAR:
				if( ucCount )
				{
					ucCount = 0;
					ucLastKey = 0;
					memset(&aucInput[0],0,sizeof(aucInput));
					memset(&aucDisp[0],0,sizeof(aucDisp));
					ucRedraw = TRUE;
				}else
				{
					ucKey = EMVERROR_CANCEL;
					ucEnd = TRUE;
				}
				break;
			case KEY_00_PT:
//			case 0x00:
				if(  (ucType == 'h')
				   ||(ucType == 'H')
				   ||(ucType == 'a')
				   ||(ucType == 'A')
				  )
				{
//					if( ucCount < strlen((char *)&aucInput[0]))
//						ucCount ++;
					if( ucCount >= ucMax)
					{
						Os__beep();
						break;
					}
					if( ucLastKey != ucKey)
					{
						ucLastKey = ucKey;
						ucKeyTabOffset = 0;
						if((ucKey != '0')&&(ucKey != ',')&&(ucKey != '.'))
							aucInput[ucCount++] = '0';
						else
							aucInput[ucCount++] = ucKey;
						ucRedraw = TRUE;
					}else
					{

						if(  (uiLastTime-uiTimeout) < 1*ONESECOND)
						{
							if( ucCount )
							{
								ucOffset = ucCount - 1;
							}else
							{
								ucOffset = 0;
							}
							if( ucKeyTabOffset < strlen((char *)aucKeyTab[10])-1)
							{
								ucKeyTabOffset ++;
								ucRedraw = TRUE;
							}else
							{
								if( ucKeyTabOffset != 0)
								{
									ucKeyTabOffset = 0;
									ucRedraw = TRUE;
								}
							}
							aucInput[ucOffset] = aucKeyTab[10][ucKeyTabOffset];
						}else
						{
							if( ucCount >= ucMax)
							{
								Os__beep();
								break;
							}
							ucLastKey = ucKey;
							ucKeyTabOffset = 0;
							if((ucKey != '0')&&(ucKey != ',')&&(ucKey != '.'))
								aucInput[ucCount++] = '0';
							else
								aucInput[ucCount++] = ucKey;
							ucRedraw = TRUE;
						}
					}
				}else
				{
					if( ucCount > (ucMax-2))
					{
						Os__beep();
						break;
					}
//					aucInput[ucCount] = ucKey;
					memcpy(&aucInput[ucCount],"00",2);
					ucCount = ucCount +2;
					ucRedraw = TRUE;
					break;
				}
				break;
			case KEY_BCKSP:
				if( ucCount )
				{
					ucCount --;
					aucInput[ucCount] = 0;
					ucRedraw = TRUE;
				}
				ucLastKey = 0;
				break;
			case KEY_ENTER:
				if( ucCount < ucMin )
				{
					Os__beep();
				}else
				{
					memset((char *)pucBuf,0x00,ucMax);
					strcpy((char *)pucBuf,(char *)&aucInput[0]);
					ucEnd = TRUE;
				}
				break;
			default :
				break;
			}
	}
		uiLastTime = uiTimeout;
		if( ucEnd == TRUE)
		{
			break;
		}
	}while(1);

	Os__timer_stop(&uiTimeout);
	return(ucKey);
}


/***********************************************************************
��������:UTIL_Input
����ʱ��:2007/01/16
������Ա:����
��������:���뷨
��ڲ���:unsigned char ucLine   ��ʾ��
			  unsigned char ucClrFlag      �����־
			  unsigned char ucMin     ��С����
			  unsigned char ucMax   �������
			  unsigned char ucType  ��������
			  unsigned char *pucBuf �����ַ���
			  unsigned char *pucMask  ����ʹ�õ��ַ�
����ֵ:			  0     �ɹ�
					����ʧ��
�޸ļ�¼:	�޸��˰������ʱҪ���ϴ��ַ���־��0

***********************************************************************/
unsigned char UTIL_InputTelNumber(unsigned char ucLine,unsigned char ucClrFlag,
				unsigned char ucMin, unsigned char ucMax,
				unsigned char ucType,
				unsigned char *pucBuf,
				unsigned char *pucMask)
{
	const unsigned char aucKeyTab[][7]
	={
//		{"0 *\0"},
		{"0 *\0"},
		{"1QZ\0"},
		{"2ABC\0"},
		{"3DEF\0"},
		{"4GHI\0"},
		{"5JKL\0"},
		{"6MNO\0"},
		{"7PRS\0"},
		{"8TUV\0"},
		{"9WXY\0"},
		{"0,.-\0"}
	};

	unsigned char	aucDisp[40];	/* Buffer for Display */
	unsigned char	aucInput[40];	/* Buffer for Key Input */
	unsigned char	ucKey;
	unsigned char	ucLastKey;
	unsigned char	ucKeyTabOffset;
	unsigned char	ucEnd;
	unsigned char	ucRedraw;
	unsigned char	ucCount;
	unsigned char	ucOffset;
	DRV_OUT *pKey;
	static unsigned int	uiTimeout;
	static unsigned int	uiLastTime;
	memset(&aucDisp[0],0,sizeof(aucDisp));
	memset(&aucInput[0],0,sizeof(aucInput));
	ucLastKey = 0;
	ucEnd = FALSE;
//	ucRedraw = FALSE;
	ucRedraw = TRUE;
	uiTimeout = 50*ONESECOND;
	uiLastTime = uiTimeout;
	ucCount = 0;
	if( ucClrFlag )
	{
        //Os__clr_display(ucLine);
	}
	if( strlen((char *)pucBuf))
	{
		ucRedraw = TRUE;
		ucCount = strlen((char *)pucBuf);
		if( ucCount > sizeof(aucInput))
			ucCount = sizeof(aucInput);
		memcpy(aucInput,pucBuf,ucCount);
	}

	Os__timer_start(&uiTimeout);
//	 util_Printf( "9999\n" );

	do
	{
		if( ucRedraw == TRUE)
		{
			memset(&aucDisp[0],0,sizeof(aucDisp));
			if( ucCount > PARAM_TELNUMBERLEN)
			{
				if(  (ucType == 'p')
				   ||(ucType == 'P')
				  )
				{
					memset(&aucDisp[0],'*',PARAM_TELNUMBERLEN);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[ucCount-PARAM_TELNUMBERLEN],PARAM_TELNUMBERLEN);
				}
				aucDisp[ucCount] = '_';
			}else
			{
				if(  (ucType == 'p')
				   ||(ucType == 'P')
				  )
				{
					memset(&aucDisp[0],'*',ucCount);
				}else if(  (ucType == 'a')
				   ||(ucType == 'A')
				  )
				{
					if(ucCount>=ucMax)
						memcpy(&aucDisp[0],&aucInput[0],ucCount-1);
					else
						memcpy(&aucDisp[0],&aucInput[0],ucCount);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[0],ucCount);
				}
				aucDisp[ucCount] = '_';
			}
            //Os__clr_display(ucLine);
            //Os__display(ucLine,0,&aucDisp[0]);
			ucRedraw = FALSE;
		}
//	 util_Printf( "7999\n" );

		pKey = Os__get_key();
//	 util_Printf( "8999\n" );

		do{
		}while(  (pKey->gen_status==DRV_RUNNING)
			   &&(uiTimeout !=0)
			   );

		if (uiTimeout == 0)
		{
			Os__abort_drv(drv_mmi);
			ucKey = ERR_END;
			break;
		}else
		{
			uiTimeout = 50*ONESECOND;
			ucKey = pKey->xxdata[1];
	//		util_Printf("\n ucKey=%02x\n",ucKey);

		}
		if(  (ucKey >= '0')
		   &&(ucKey <= '9')
		  )
		{
			if( pucMask )
			{
				if( !strchr((char *)pucMask, ucKey ))
					continue;
			}
			switch(ucType)
			{
            case 'h':
			case 'H':
				if( ucCount >= ucMax)
				{
					Os__beep();
					break;
				}
				if( ucLastKey != ucKey)
				{
					ucLastKey = ucKey;
					ucKeyTabOffset = 0;
					aucInput[ucCount++] = ucKey;
					ucRedraw = TRUE;
				}else
				{
                    if(  (uiLastTime-uiTimeout) < 1*ONESECOND)
					{
                        if( ucCount )
						{
							ucOffset = ucCount - 1;
						}else
						{
							ucOffset = 0;
						}
                        if(  (ucKey >= '2')
						   &&(ucKey <= '3')
						  )
						{
                            if( ucKeyTabOffset < strlen((char *)aucKeyTab[ucKey-'0'])-1)
							{
								ucKeyTabOffset ++;
								ucRedraw = TRUE;
							}else
							{
								if( ucKeyTabOffset )
								{
									ucKeyTabOffset = 0;
									ucRedraw = TRUE;
								}
							}
							aucInput[ucOffset] = aucKeyTab[ucKey-'0'][ucKeyTabOffset];
                        }else
						{
							ucLastKey = ucKey;
							ucKeyTabOffset = 0;
							aucInput[ucCount++] = ucKey;
							ucRedraw = TRUE;
						}
                    }else
					{
						if( ucCount >= ucMax)
						{
							Os__beep();
							break;
						}
						ucLastKey = ucKey;
						ucKeyTabOffset = 0;
						aucInput[ucCount++] = ucKey;
						ucRedraw = TRUE;
					}
                }
				break;
            case 'a':
			case 'A':
				if( ucCount > ucMax)
				{
					Os__beep();
					break;
				}
				if( ucLastKey != ucKey)
				{
			//		 util_Printf( "4999\n" );
					ucLastKey = ucKey;
					ucKeyTabOffset = 0;
					aucInput[ucCount++] = ucKey;
					ucRedraw = TRUE;
			//			 util_Printf( "6999\n" );

				}else
				{
			//		 util_Printf( "5999\n" );

					if(  (uiLastTime-uiTimeout) < 1*ONESECOND)
					{
						if( ucCount )
						{
							ucOffset = ucCount - 1;
						}else
						{
							ucOffset = 0;
						}
						if( ucKeyTabOffset < strlen((char *)aucKeyTab[ucKey-'0'])-1)
						{
							ucKeyTabOffset ++;
							ucRedraw = TRUE;
						}else
						{
							if( ucKeyTabOffset != 0)
							{
								ucKeyTabOffset = 0;
								ucRedraw = TRUE;
							}
						}
						aucInput[ucOffset] = aucKeyTab[ucKey-'0'][ucKeyTabOffset];
					}else
					{
						if( ucCount >= ucMax)
						{
							Os__beep();
							break;
						}
						ucLastKey = ucKey;
						ucKeyTabOffset = 0;
						aucInput[ucCount++] = ucKey;
						ucRedraw = TRUE;
					}
				}
				break;
            default:
				if( ucCount >= ucMax)
				{
					Os__beep();
					break;
				}
				aucInput[ucCount++] = ucKey;
				ucRedraw = TRUE;
				break;
			}
		}else
		{
			switch(ucKey)
			{
			case KEY_CLEAR:
				if( ucCount )
				{
					ucCount = 0;
					ucLastKey = 0;
					memset(&aucInput[0],0,sizeof(aucInput));
					memset(&aucDisp[0],0,sizeof(aucDisp));
					ucRedraw = TRUE;
				}else
				{
					ucKey = ERR_END;
					ucEnd = TRUE;
				}
				break;
			case KEY_00_PT:
//			case 0x00:
				if(  (ucType == 'h')
				   ||(ucType == 'H')
				   ||(ucType == 'a')
				   ||(ucType == 'A')
				  )
				{
//					if( ucCount < strlen((char *)&aucInput[0]))
//						ucCount ++;
					if( ucCount >= ucMax)
					{
						Os__beep();
						break;
					}
					if( ucLastKey != ucKey)
					{
						ucLastKey = ucKey;
						ucKeyTabOffset = 0;
						if((ucKey != '0')&&(ucKey != ',')&&(ucKey != '.'))
							aucInput[ucCount++] = '0';
						else
							aucInput[ucCount++] = ucKey;
						ucRedraw = TRUE;
					}else
					{

						if(  (uiLastTime-uiTimeout) < 1*ONESECOND)
						{
							if( ucCount )
							{
								ucOffset = ucCount - 1;
							}else
							{
								ucOffset = 0;
							}
							if( ucKeyTabOffset < strlen((char *)aucKeyTab[10])-1)
							{
								ucKeyTabOffset ++;
								ucRedraw = TRUE;
							}else
							{
								if( ucKeyTabOffset != 0)
								{
									ucKeyTabOffset = 0;
									ucRedraw = TRUE;
								}
							}
							aucInput[ucOffset] = aucKeyTab[10][ucKeyTabOffset];
						}else
						{
							if( ucCount >= ucMax)
							{
								Os__beep();
								break;
							}
							ucLastKey = ucKey;
							ucKeyTabOffset = 0;
							if((ucKey != '0')&&(ucKey != ',')&&(ucKey != '.'))
								aucInput[ucCount++] = '0';
							else
								aucInput[ucCount++] = ucKey;
							ucRedraw = TRUE;
						}
					}
				}else
				{
					if( ucCount > (ucMax-2))
					{
						Os__beep();
						break;
					}
//					aucInput[ucCount] = ucKey;
					memcpy(&aucInput[ucCount],"00",2);
					ucCount = ucCount +2;
					ucRedraw = TRUE;
					break;
				}
				break;
			case KEY_BCKSP:
				if( ucCount )
				{
				//������2007-01-17
					ucLastKey = 0;

					ucCount --;
					aucInput[ucCount] = 0;
					ucRedraw = TRUE;
				}
				break;
			case KEY_ENTER:
				if( ucCount < ucMin )
				{
					Os__beep();
				}else
				{
					//memset((char *)pucBuf,0x00,sizeof(pucBuf));
					strcpy((char *)pucBuf,(char *)&aucInput[0]);
					pucBuf[strlen((char *)aucInput)] = NULL;
					ucEnd = TRUE;
				}
				break;
			default :
				break;
			}
	}
		uiLastTime = uiTimeout;
		if( ucEnd == TRUE)
		{
			break;
		}
	}while(1);

	Os__timer_stop(&uiTimeout);
	return(ucKey);
}


unsigned char UTIL_Input_YP(unsigned char ucLine,unsigned char ucClrFlag,
				unsigned char ucMin, unsigned char ucMax,
				unsigned char ucType,
				unsigned char *pucBuf,
				unsigned char *pucMask)
{
	const unsigned char aucKeyTab[][7]
	={	{"0*,.-\0"},
		{"1#\0"},
		{"2ABC\0"},
		{"3DEF\0"},
		{"4GHI\0"},
		{"5JKL\0"},
		{"6MNO\0"},
		{"7PQRS\0"},
		{"8TUV\0"},
		{"9WXYZ\0"}
	};

	unsigned char	aucDisp[40];	/* Buffer for Display */
	unsigned char	aucInput[40];	/* Buffer for Key Input */
	unsigned char	ucKey;
	unsigned char	ucLastKey;
	unsigned char	ucKeyTabOffset;
	unsigned char	ucEnd;
	unsigned char	ucRedraw;
	unsigned char	ucCount;
	unsigned char	ucOffset;
	DRV_OUT *pdKey;
	static unsigned int	uiTimeout;
	static unsigned int	uiLastTime;
	memset(&aucDisp[0],0,sizeof(aucDisp));
	memset(&aucInput[0],0,sizeof(aucInput));
	ucLastKey = 0;
	ucEnd = FALSE;
	ucRedraw = FALSE;
	uiTimeout = 50*ONESECOND;
	uiLastTime = uiTimeout;
	ucCount = 0;

	if( ucClrFlag )
	{
        //Os__clr_display(ucLine);
	}
	if( strlen((char *)pucBuf))
	{
		ucRedraw = TRUE;
		ucCount = strlen((char *)pucBuf);
		if( ucCount > sizeof(aucInput))
			ucCount = sizeof(aucInput);
		memcpy(aucInput,pucBuf,ucCount);
	}


	do
	{
	Os__timer_start(&uiTimeout);
		if( ucRedraw == TRUE)
		{
			memset(&aucDisp[0],0,sizeof(aucDisp));
			if( ucCount > 15)
			{
				if(  (ucType == 'p')
				   ||(ucType == 'P')
				  )
				{
					memset(&aucDisp[0],'*',15);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[ucCount-15],15);
				}
			}else
			{
				if(  (ucType == 'p')
				   ||(ucType == 'P')
				  )
				{
					memset(&aucDisp[0],'*',ucCount);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[0],ucCount);
				}

			}
            //Os__clr_display(ucLine);
            //Os__display(ucLine,0,&aucDisp[0]);
			ucRedraw = FALSE;
		}

		pdKey = Os__get_key();

		do{
		}while(  (pdKey->gen_status==DRV_RUNNING)
			   &&(uiTimeout !=0)
			   );

		if (uiTimeout == 0)
		{
			Os__abort_drv(drv_mmi);
			Os__xdelay(10);
			ucKey = ERR_CANCEL;
			break;
		}else
		{
			ucKey = pdKey->xxdata[1];
		}
		if(  (ucKey >= '0')
		   &&(ucKey <= '9')
		  )
		{

			if( pucMask )
			{
				if( !strchr((char *)pucMask, ucKey ))
					continue;
			}
			switch(ucType)
			{
           		case 'h':
			case 'H':
				if( ucCount > ucMax)
				{
					Os__beep();
					break;
				}
				if( ucLastKey != ucKey)
				{
					ucLastKey = ucKey;
					ucKeyTabOffset = 0;
					aucInput[ucCount++] = ucKey;
					ucRedraw = TRUE;
				}else
				{
                    		if(  (uiLastTime-uiTimeout) < 1*ONESECOND)
				{
                       			 if( ucCount )
						{
							ucOffset = ucCount - 1;
						}else
						{
							ucOffset = 0;
				 }
                       	 if(  (ucKey >= '2')
						   &&(ucKey <= '3')
						  )
						{
                            if( ucKeyTabOffset < strlen((char *)aucKeyTab[ucKey-'0'])-1)
							{
								ucKeyTabOffset ++;
								ucRedraw = TRUE;
							}else
							{
								if( ucKeyTabOffset )
								{
									ucKeyTabOffset = 0;
									ucRedraw = TRUE;
								}
							}
							aucInput[ucOffset] = aucKeyTab[ucKey-'0'][ucKeyTabOffset];
                        }else
						{
							ucLastKey = ucKey;
							ucKeyTabOffset = 0;
							aucInput[ucCount++] = ucKey;
							ucRedraw = TRUE;
						}
                    }else
					{
						if( ucCount >= ucMax)
						{
							Os__beep();
							break;
						}
						ucLastKey = ucKey;
						ucKeyTabOffset = 0;
						aucInput[ucCount++] = ucKey;
						ucRedraw = TRUE;
					}
                }
				break;
            case 'a':
			case 'A':
				if( ucCount > ucMax)
				{
					Os__beep();
					break;
				}
				if( ucLastKey != ucKey)
				{
					ucLastKey = ucKey;
					ucKeyTabOffset = 0;
					aucInput[ucCount++] = ucKey;
					ucRedraw = TRUE;
				}else
				{

					if(  (uiLastTime-uiTimeout) < 1*ONESECOND)
					{
						if( ucCount )
						{
							ucOffset = ucCount - 1;
						}else
						{
							ucOffset = 0;
						}
						if( ucKeyTabOffset < strlen((char *)aucKeyTab[ucKey-'0'])-1)
						{
							ucKeyTabOffset ++;
							ucRedraw = TRUE;
						}else
						{
							if( ucKeyTabOffset != 0)
							{
								ucKeyTabOffset = 0;
								ucRedraw = TRUE;
							}
						}
						aucInput[ucOffset] = aucKeyTab[ucKey-'0'][ucKeyTabOffset];
					}else
					{
						if( ucCount >= ucMax)
						{
							Os__beep();
							break;
						}
						ucLastKey = ucKey;
						ucKeyTabOffset = 0;
						aucInput[ucCount++] = ucKey;
						ucRedraw = TRUE;
					}
				}
				break;
            default:
				if( ucCount >= ucMax)
				{
					Os__beep();
					break;
				}
				aucInput[ucCount++] = ucKey;
				ucRedraw = TRUE;
				break;
			}
		}else
		{
			switch(ucKey)
			{
			case KEY_CLEAR:
				if( ucCount )
				{
					ucCount = 0;
					ucLastKey = 0;
					memset(&aucInput[0],0,sizeof(aucInput));
					memset(&aucDisp[0],0,sizeof(aucDisp));
					ucRedraw = TRUE;
				}else
				{
					ucKey = ERR_CANCEL;
					ucEnd = TRUE;
				}
				break;
			case KEY_00_PT:
				if(  (ucType == 'h')
				   ||(ucType == 'H')
				   ||(ucType == 'a')
				   ||(ucType == 'A')
				  )
				{
					if( ucCount < strlen((char *)&aucInput[0]))
						ucCount ++;
				}
				break;
			case KEY_BCKSP:
				if( ucCount )
				{
					ucCount --;
					aucInput[ucCount] = 0;
					ucRedraw = TRUE;
				}
				break;
			case KEY_ENTER:
				if( ucCount < ucMin )
				{
					Os__beep();
				}else
				{
					//memset((char *)pucBuf,0x00,sizeof(pucBuf));
					strcpy((char *)pucBuf,(char *)&aucInput[0]);
					pucBuf[strlen((char*)aucInput)] = NULL;
					ucEnd = TRUE;
				}
				break;
			default :
				break;
			}
		}
		uiLastTime = uiTimeout;
		if( ucEnd == TRUE)
		{
			break;
		}
	}while(1);

	Os__timer_stop(&uiTimeout);
	return(ucKey);
}

/***********************************************************************
��������:UTIL_Input
����ʱ��:2007/01/16
������Ա:����
��������:���뷨
��ڲ���:unsigned char ucLine   ��ʾ��
			  unsigned char ucClrFlag      �����־
			  unsigned char ucMin     ��С����
			  unsigned char ucMax   �������
			  unsigned char ucType  ��������
			  unsigned char *pucBuf �����ַ���
			  unsigned char *pucMask  ����ʹ�õ��ַ�
����ֵ:			  0     �ɹ�
					����ʧ��
�޸ļ�¼:	�޸��˰������ʱҪ���ϴ��ַ���־��0

***********************************************************************/
unsigned char UTIL_Input(unsigned char ucLine,unsigned char ucClrFlag,
				unsigned char ucMin, unsigned char ucMax,
				unsigned char ucType,
				unsigned char *pucBuf,
				unsigned char *pucMask)
{
	const unsigned char aucKeyTab[][7]
	={
		{"0 *\0"},
		{"1QZ\0"},
		{"2ABC\0"},
		{"3DEF\0"},
		{"4GHI\0"},
		{"5JKL\0"},
		{"6MNO\0"},
		{"7PRS\0"},
		{"8TUV\0"},
		{"9WXY\0"},
		{"0,.-\0"}
	};

	unsigned char	aucDisp[40];	/* Buffer for Display */
	unsigned char	aucInput[40];	/* Buffer for Key Input */
	unsigned char	ucKey;
	unsigned char	ucLastKey;
	unsigned char	ucKeyTabOffset;
	unsigned char	ucEnd;
	unsigned char	ucRedraw;
	unsigned char	ucCount;
	unsigned char	ucOffset;
	DRV_OUT *pKey;
	static unsigned int	uiTimeout;
	static unsigned int	uiLastTime;
	memset(&aucDisp[0],0,sizeof(aucDisp));
	memset(&aucInput[0],0,sizeof(aucInput));
	ucLastKey = 0;
	ucEnd = FALSE;
	ucRedraw = TRUE;
	uiTimeout = 50*ONESECOND;
	uiLastTime = uiTimeout;
	ucCount = 0;

	if( ucClrFlag )
	{
        //Os__clr_display(ucLine);
	}
	if( strlen((char *)pucBuf))
	{
		ucRedraw = TRUE;
		ucCount = strlen((char *)pucBuf);
		if( ucCount > sizeof(aucInput))
			ucCount = sizeof(aucInput);
		memcpy(aucInput,pucBuf,ucCount);
	}


	do
	{
#if PS100||PS400
	Os__timer_start(&uiTimeout);
#endif
		if( ucRedraw == TRUE)
		{
			memset(&aucDisp[0],0,sizeof(aucDisp));
			if( ucCount > MAXLINECHARNUM)
			{
				if(  (ucType == 'p')
				   ||(ucType == 'P')
				  )
				{
					memset(&aucDisp[0],'*',MAXLINECHARNUM);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[ucCount-MAXLINECHARNUM],MAXLINECHARNUM);
				}
				aucDisp[ucCount] = '_';
			}else
			{
				if(  (ucType == 'p')
				   ||(ucType == 'P')
				  )
				{
					memset(&aucDisp[0],'*',ucCount);
				}
				else if(  (ucType == 'a')
				   ||(ucType == 'A')
				  )
				{
					if(ucCount>ucMax)
						memcpy(&aucDisp[0],&aucInput[0],ucCount-1);
					else
						memcpy(&aucDisp[0],&aucInput[0],ucCount);
				}else
				{
					memcpy(&aucDisp[0],&aucInput[0],ucCount);
				}
				aucDisp[ucCount] = '_';
			}
            //Os__clr_display(ucLine);
            //Os__display(ucLine,0,&aucDisp[0]);
			ucRedraw = FALSE;
		}
		pKey = Os__get_key();
		do{
		}while(  (pKey->gen_status==DRV_RUNNING)
			   &&(uiTimeout !=0)
			   );

		if (uiTimeout == 0)
		{
			Os__abort_drv(drv_mmi);
			ucKey = ERR_END;
			break;
		}else
		{
			uiTimeout = 50*ONESECOND;
			ucKey = pKey->xxdata[1];
		}
		if(  (ucKey >= '0')
		   &&(ucKey <= '9')
		  )
		{
			if( pucMask )
			{
				if( !strchr((char *)pucMask, ucKey ))
					continue;
			}
			switch(ucType)
			{
            case 'h':
			case 'H':
				if( ucCount >= ucMax)
				{
					Os__beep();
					break;
				}
				if( ucLastKey != ucKey)
				{
					ucLastKey = ucKey;
					ucKeyTabOffset = 0;
					aucInput[ucCount++] = ucKey;
					ucRedraw = TRUE;
				}else
				{
                    if(  (uiLastTime-uiTimeout) < 1*ONESECOND)
					{
                        if( ucCount )
						{
							ucOffset = ucCount - 1;
						}else
						{
							ucOffset = 0;
						}
                        if(  (ucKey >= '2')
						   &&(ucKey <= '3')
						  )
						{
                            if( ucKeyTabOffset < strlen((char *)aucKeyTab[ucKey-'0'])-1)
							{
								ucKeyTabOffset ++;
								ucRedraw = TRUE;
							}else
							{
								if( ucKeyTabOffset )
								{
									ucKeyTabOffset = 0;
									ucRedraw = TRUE;
								}
							}
							aucInput[ucOffset] = aucKeyTab[ucKey-'0'][ucKeyTabOffset];
                        }else
						{
							ucLastKey = ucKey;
							ucKeyTabOffset = 0;
							aucInput[ucCount++] = ucKey;
							ucRedraw = TRUE;
						}
                    }else
					{
						if( ucCount >= ucMax)
						{
							Os__beep();
							break;
						}
						ucLastKey = ucKey;
						ucKeyTabOffset = 0;
						aucInput[ucCount++] = ucKey;
						ucRedraw = TRUE;
					}
                }
				break;
            case 'a':
			case 'A':
				if( ucCount+1> ucMax)
				{
					Os__beep();
					break;
				}

				if( ucLastKey != ucKey)
				{
					ucLastKey = ucKey;
					ucKeyTabOffset = 0;
					aucInput[ucCount++] = ucKey;
					ucRedraw = TRUE;
				}else
				{

					if(  (uiLastTime-uiTimeout) < 1*ONESECOND)
					{
						if( ucCount )
						{
							ucOffset = ucCount - 1;
						}else
						{
							ucOffset = 0;
						}
						if( ucKeyTabOffset < strlen((char *)aucKeyTab[ucKey-'0'])-1)
						{
							ucKeyTabOffset ++;
							ucRedraw = TRUE;
						}else
						{
							if( ucKeyTabOffset != 0)
							{
								ucKeyTabOffset = 0;
								ucRedraw = TRUE;
							}
						}
						aucInput[ucOffset] = aucKeyTab[ucKey-'0'][ucKeyTabOffset];
					}else
					{
						if( ucCount >= ucMax)
						{
							Os__beep();
							break;
						}

						ucLastKey = ucKey;
						ucKeyTabOffset = 0;
						aucInput[ucCount++] = ucKey;
						ucRedraw = TRUE;
					}
				}

				break;
            default:
				if( ucCount >= ucMax)
				{
					Os__beep();
					break;
				}
				aucInput[ucCount++] = ucKey;
				ucRedraw = TRUE;
				break;
			}
		}else
		{
			switch(ucKey)
			{
			case KEY_CLEAR:
				if( ucCount )
				{
					ucCount = 0;
					ucLastKey = 0;
					memset(&aucInput[0],0,sizeof(aucInput));
					memset(&aucDisp[0],0,sizeof(aucDisp));
					ucRedraw = TRUE;
				}else
				{
					ucKey = ERR_END;
					ucEnd = TRUE;
				}
				break;
			case KEY_00_PT:
				if(  (ucType == 'h')
				   ||(ucType == 'H')
				   ||(ucType == 'a')
				   ||(ucType == 'A')
				  )
				{
					if( ucCount >= ucMax)
					{
						Os__beep();
						break;
					}
					if( ucLastKey != ucKey)
					{
						ucLastKey = ucKey;
						ucKeyTabOffset = 0;
						if((ucKey != '0')&&(ucKey != ',')&&(ucKey != '.'))
							aucInput[ucCount++] = '0';
						else
							aucInput[ucCount++] = ucKey;
						ucRedraw = TRUE;
					}else
					{
						if(  (uiLastTime-uiTimeout) < 1*ONESECOND)
						{
							if( ucCount )
							{
								ucOffset = ucCount - 1;
							}else
							{
								ucOffset = 0;
							}
							if( ucKeyTabOffset < strlen((char *)aucKeyTab[10])-1)
							{
								ucKeyTabOffset ++;
								ucRedraw = TRUE;
							}else
							{
								if( ucKeyTabOffset != 0)
								{
									ucKeyTabOffset = 0;
									ucRedraw = TRUE;
								}
							}
							aucInput[ucOffset] = aucKeyTab[10][ucKeyTabOffset];
						}else
						{
							if( ucCount >= ucMax)
							{
								Os__beep();
								break;
							}
							ucLastKey = ucKey;
							ucKeyTabOffset = 0;
							if((ucKey != '0')&&(ucKey != ',')&&(ucKey != '.'))
								aucInput[ucCount++] = '0';
							else
								aucInput[ucCount++] = ucKey;
							ucRedraw = TRUE;
						}
					}
				}else
				{
					if( ucCount > (ucMax-2))
					{
						Os__beep();
						break;
					}
					memcpy(&aucInput[ucCount],"00",2);
					ucCount = ucCount +2;
					ucRedraw = TRUE;
					break;
				}

				break;
			case KEY_BCKSP:
				if( ucCount )
				{
					ucLastKey = 0;

					ucCount --;
					aucInput[ucCount] = 0;
					ucRedraw = TRUE;
				}
				break;
			case KEY_ENTER:
				if(ucCount == 0)
					ucEnd = TRUE;
				if( ucCount < ucMin )
				{
					Os__beep();
				}
				else
				{
					memset((char *)pucBuf,0x00,ucMax);
					strcpy((char *)pucBuf,(char *)&aucInput[0]);
					ucEnd = TRUE;
				}
				break;
			default :
				break;
			}
	}
		uiLastTime = uiTimeout;
		if( ucEnd == TRUE)
		{
			break;
		}
	}while(1);

	Os__timer_stop(&uiTimeout);
	return(ucKey);
}


void UTIL_Form_Montant(unsigned char *Mnt_Fmt,unsigned long Montant,unsigned char Pos_Virgule)
{
	uchar   i;
	uchar   j;

		long_str(Mnt_Fmt,10,&Montant);
		j = Pos_Virgule;
		for (i=9; j ; i--, j--)
			Mnt_Fmt[i+1] = Mnt_Fmt[i];
		if (Pos_Virgule)
		{
			Mnt_Fmt[i+1] ='.';
			Mnt_Fmt[i+1+Pos_Virgule+1] = 0;
		}
		for(j=0 ; ((j < i) && (Mnt_Fmt[j]=='0')) ; Mnt_Fmt[j++]=' ');
}

void UTIL_Form_Montant_Total(unsigned char *Mnt_Fmt,unsigned long Montant,unsigned char Pos_Virgule)
{
	uchar   i;
	uchar   j;

		long_str(Mnt_Fmt,11,&Montant);
		j = Pos_Virgule;
		for (i=10; j ; i--, j--)
			Mnt_Fmt[i+1] = Mnt_Fmt[i];
		if (Pos_Virgule)
		{
			Mnt_Fmt[i+1] ='.';
			Mnt_Fmt[i+1+Pos_Virgule+1] = 0;
		}
		for(j=0 ; ((j < i) && (Mnt_Fmt[j]=='0')) ; Mnt_Fmt[j++]=' ');
}
#if 0
POPTION UTIL_SelectOption(PUCHAR szOptionTitle,EMV_FONT ucFont,POPTION pOption ,UCHAR ucOptionNums)
{
	UCHAR	ucI,ucMaxRows,ucCurIndex,ucDispRows;
	POPTION pCurOption;
	UCHAR	ucKey;

	ucCurIndex=0;
	while(1)
	{
        //Os__clr_display(255);
		if(ucFont==ASCII_FONT)
		{
			ucMaxRows=8;
			OSMMI_DisplayASCII(0x30|0x80,0,0,szOptionTitle);
		}
		else
		{
			ucMaxRows=4;
			OSMMI_GB2312Display(0x31|0x80,0,0,szOptionTitle);
		}
		ucDispRows=ucCurIndex+ucMaxRows-1<ucOptionNums-1? ucMaxRows-1:ucOptionNums-ucCurIndex;

		for(ucI=0;ucI<ucDispRows;ucI++)
		{

			if(ucFont==ASCII_FONT)
				OSMMI_DisplayASCII(0x30,ucI+1,0,(pOption+ucI+ucCurIndex)->szOptionMsg);
			else
				OSMMI_GB2312Display(0x31,ucI+1,0,(pOption+ucI+ucCurIndex)->szOptionMsg);
		}
		ucKey=Os__xget_key();
		switch(ucKey)
		{
			case KEY_F3:
				if(ucCurIndex+ucMaxRows-1<ucOptionNums-1)
					ucCurIndex+=ucMaxRows-1;
				break;
			case KEY_F4:
				if(ucCurIndex+1>ucMaxRows-1)
					ucCurIndex-=ucMaxRows-1;
				break;
			case KEY_ENTER:
				return pOption+ucCurIndex;

			case KEY_CLEAR:
				return NULL;
				break;
			default:
				for(ucI=0;ucI<ucOptionNums;ucI++)
				{
					if((pOption+ucI)->ucOptionValue==ucKey-0x30)
						return pOption+ucI;
				}
		}

	}

}
#endif
UCHAR	UTIL_OfflinePIN(PUCHAR	pucOfflinePin)
{
	unsigned char aucAmountBuf[20],ucResult;
	/*TIT-PBOC-LOAD-007*/
//	if(G_NORMALTRANS_ucTransType==TRANS_EC_CASHLOAD||G_NORMALTRANS_ucTransType==TRANS_EC_UNASSIGNLOAD)
//		return SUCCESS;
	memset( aucAmountBuf, 0, sizeof(aucAmountBuf));
	UTIL_Form_Montant(&aucAmountBuf[5],G_NORMALTRANS_ulAmount,2);
	memcpy(aucAmountBuf,"��",6);
    //Os__clr_display(255);
	if(DataSave0.ConstantParamData.Pinpadflag ==1)
	{
		//---POS DISPLAY
		//if((!ReaderSupport)||(ReaderType!=READER_SAND))
		{
			Os__light_on_pp();
			Os__clr_display_pp(255);
		}
		MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
		if(G_NORMALTRANS_ucTransType!=TRANS_QUERYBAL)
		{
            //Os__GB2312_display(1, 0, aucAmountBuf);
			//if((!ReaderSupport)||(ReaderType!=READER_SAND))
				Os__GB2312_display_pp(0, 0, aucAmountBuf);
		}
		//if((!ReaderSupport)||(ReaderType!=READER_SAND))
			Os__GB2312_display_pp(1, 0, (uchar *) "�������ѻ�����");
        //Os__GB2312_display(2, 0, (uchar *) "��ֿ�����������");
        //Os__GB2312_display(3, 0, (uchar * )"�������밴ȷ�ϼ�");
		//Os__GB2312_display_pp(3, 0, (uchar * )"�������밴ȷ�ϼ�");
	}
	else
	{
		if(G_NORMALTRANS_ucTransType!=TRANS_QUERYBAL)
		{
            //Os__GB2312_display(0, 0, aucAmountBuf);
            //Os__GB2312_display(1, 0, (uchar *) "�������ѻ�����:");
            //Os__GB2312_display(2, 0, (uchar *) "��ֿ�����������");
            //Os__GB2312_display(3, 0, (uchar * )"�������밴ȷ�ϼ�");
		}else
		{
			MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
            //Os__GB2312_display(1, 0, (uchar * )"�������ѻ�����:");
            //Os__GB2312_display(2, 0, (uchar *) "��ֿ�����������");
            //Os__GB2312_display(3, 0, (uchar * )"�������밴ȷ�ϼ�");
		}
	}

	if(DataSave0.ConstantParamData.Pinpadflag ==1)
	{
//		if((ReaderSupport)&&(ReaderType==READER_SAND))
//		{
//			ucResult =DIGITAL_InputPin(pucOfflinePin);
//			if(ucResult =0x00)
//				ucResult =KEY_ENTER;
//		}
//		else
			ucResult = UTIL_Input_EMV_pp(1,false,4,12,'P',pucOfflinePin);
		//ucResult =DIGITAL_InputPin(pucOfflinePin);
	}
	else
		ucResult = UTIL_Input_EMV_PIN(1,false,4,12,'P',pucOfflinePin,NULL);

	if(DataSave0.ConstantParamData.Pinpadflag ==1)
	{
		//if((!ReaderSupport)||(ReaderType!=READER_SAND))
		{
			Os__clr_display_pp(255);
			//Os__GB2312_display_pp(0,0,(uchar *)"   ɼ������ͨ");
			Os__GB2312_display_pp(0,0,(uchar *)"    ��ӭʹ��");
		}
	}

	if(ucResult  ==KEY_ENTER )
		return SUCCESS;
	else
	if(ucResult == ERR_END)
	{
		return EMVERROR_CANCEL;
	}else
	{
		return EMVERROR_BYPASS;
	}
}

UCHAR	UTIL_OnlinePIN(PUCHAR pucEnPIN,PUCHAR pucPinLen)
{
	UCHAR	aucPinData[20],ucPinLen,ucResult,aucBuff[20];
	const UCHAR  PINKEY[] ="12345678";

	Os__clr_display_pp(255);
	Os__GB2312_display_pp(0,0,(PUCHAR)"Online PIN pls");
	memset(aucPinData,0x00,sizeof(aucPinData));
	ucResult =DIGITAL_InputPin(aucPinData);
	if(ucResult ==0x00)
	{
		memset(aucBuff,0xFF,sizeof(aucBuff));
		ucPinLen = strlen((char*)aucPinData);
		aucBuff[0] = 0x20 + ucPinLen;
		if(ucPinLen%2)
		{
			aucPinData[ucPinLen] =0x3F;
			ucPinLen++;
		}
		CONV_AscBcd(&aucBuff[1],ucPinLen/2,aucPinData,ucPinLen);
		des(aucBuff,pucEnPIN,(PUCHAR)PINKEY);
		*pucPinLen =8;
		return EMVERROR_SUCCESS;


	}
	return EMVERROR_CANCEL;

}

UCHAR UTIL_WriteConstParamFile(EMV_CONSTPARAM * pConstParam)
{
	int	iHandle,iFileLen;
	unsigned char ucResult,iFileResult;

	iHandle = OSAPP_OpenFile((char *)CONSTPARAMFILE,O_WRITE);
	if(iHandle>=0)
	{
		iFileResult=OSAPP_FileSeek(iHandle,0,SEEK_SET);

		if(!iFileResult) iFileLen=OSAPP_FileWrite(iHandle,(PUCHAR)pConstParam,sizeof(EMV_CONSTPARAM));
	}
	OSAPP_FileClose(iHandle);
	ucResult = OSAPP_FileGetLastError();
	if(ucResult)
		ucResult =EMVERROR_SAVEFILE;
	return ucResult;
}

UCHAR UTIL_WriteEMVConfigFile(TERMCONFIG * pEMVConfig)
{
	int	iHandle,iFileLen;
	unsigned char ucResult,iFileResult;

       ucResult=SUCCESS;
	iHandle = OSAPP_OpenFile((char *)EMVCONFIGFILE,O_WRITE);
	util_Printf("\n---EMVCONFIGFILE �ļ����:%d\n",iHandle);
	if(iHandle>=0)
	{
		iFileResult=OSAPP_FileSeek(iHandle,0,SEEK_SET);

		if(!iFileResult) iFileLen=OSAPP_FileWrite(iHandle,(PUCHAR)pEMVConfig,sizeof(TERMCONFIG));
	}
	OSAPP_FileClose(iHandle);

	ucResult = OSAPP_FileGetLastError();
        if (ucResult)
        {
            G_RUNDATA_ucErrorFileCode = ucResult;
            ucResult = ERR_WRITEFILE;
        }
        util_Printf("EMVCONFIGFILE�ļ���������:%02x\n",ucResult);
        return(ucResult);
}


UCHAR UTIL_SaveCAPKFile(void)
{
	UCHAR	ucResult,ucI,ucJ;
	USHORT	uiRecordNum;
	ucResult =UTIL_DeleteAllData((PUCHAR)CAPKFILE);
	for(ucI=0;!ucResult && ucI<ucTermCAPKNum;ucI++)
	{
		uiRecordNum =FILE_InsertRecordByFileName(CAPKFILE,&TermCAPK[ucI],sizeof(CAPK));
		if(uiRecordNum==0) ucResult =EMVERROR_SAVEFILE;
#if 1
		util_Printf("\n RID:");
		for(ucJ =0;ucJ<5;ucJ++) util_Printf("%02x ",TermCAPK[ucI].aucRID[ucJ]);
		util_Printf(" CAPKI:%02x",TermCAPK[ucI].ucCAPKI);
#endif
	}
	return ucResult;
}

UCHAR UTIL_SaveAIDFile(void)
{
	UCHAR	ucResult,ucI,ucJ;
	USHORT	uiRecordNum;

	ucResult =UTIL_DeleteAllData((PUCHAR)TERMSUPPORTAPPFILE);
	for(ucI=0;!ucResult && ucI<ucTermAIDNum;ucI++)
	{
		uiRecordNum =FILE_InsertRecordByFileName(TERMSUPPORTAPPFILE,&TermAID[ucI],sizeof(TERMSUPPORTAPP));
		if(uiRecordNum==0) ucResult =EMVERROR_SAVEFILE;
#if 1
		util_Printf("\n---------------UTIL_SaveAIDFile()-------------------\n");
		util_Printf("\nASI: %02X",TermAID[ucI].ucASI);
		util_Printf("AID:");
		for(ucJ=0;ucJ<TermAID[ucI].ucAIDLen;ucJ++)
			util_Printf("%02X ",TermAID[ucI].aucAID[ucJ]);
		util_Printf("\n---------------UTIL_SaveAIDFile()----end---------------\n");
#endif
	}
	return ucResult;
}

UCHAR UTIL_SaveAIDParamFile(void)
{
	UCHAR	ucResult,ucI;
	USHORT	uiRecordNum;

	ucResult =UTIL_DeleteAllData((PUCHAR)EMVAIDPARAMFILE);
	for(ucI=0;!ucResult && ucI<ucTermAIDNum;ucI++)
	{
		uiRecordNum =FILE_InsertRecordByFileName(EMVAIDPARAMFILE,&AIDParam[ucI],sizeof(EMVAIDPARAM));
		if(uiRecordNum==0) ucResult =EMVERROR_SAVEFILE;
	}
	return ucResult;
}

UCHAR UTIL_SaveExceptFile(void)
{
	UCHAR	ucResult,ucI,ucJ;
	USHORT	uiRecordNum;

	ucResult =UTIL_DeleteAllData((PUCHAR)EXCEPTFILE);
	for(ucI=0;!ucResult && ucI<ucExceptFileNum;ucI++)
	{
		uiRecordNum =FILE_InsertRecordByFileName(EXCEPTFILE,&ExceptFile[ucI],sizeof(EXCEPTPAN));
		if(uiRecordNum==0) ucResult =EMVERROR_SAVEFILE;
#if 1
		util_Printf("\nPAN:");
		for(ucJ=0; ucJ<MAXPANDATALEN;ucJ++)
			util_Printf("%02X ",ExceptFile[ucI].aucPAN[ucJ]);
		util_Printf(" %02X",ExceptFile[ucI].ucPANSeq);
#endif
	}
	return ucResult;
}

UCHAR UTIL_SaveIPKRevokeFile(void)
{
	UCHAR	ucResult,ucI;
	USHORT	uiRecordNum;

	ucResult =UTIL_DeleteAllData((PUCHAR)IPKREVOKEFILE);
	for(ucI=0;!ucResult && ucI<ucIPKRevokeNum;ucI++)
	{
		uiRecordNum =FILE_InsertRecordByFileName(IPKREVOKEFILE,&IPKRevoke[ucI],sizeof(IPKREVOKE));
		if(uiRecordNum==0) ucResult =EMVERROR_SAVEFILE;
#if 0
		util_Printf("\n RID:");
		for(ucJ =0;ucJ<5;ucJ++) util_Printf("%02x ",IPKRevoke[ucI].aucRID[ucJ]);
		util_Printf(" CAPKI:%02x",IPKRevoke[ucI].ucCAPKI);
		for(ucJ =0;ucJ<3;ucJ++) util_Printf("%02x ",IPKRevoke[ucI].aucCertSerial[ucJ]);
#endif
	}
	return ucResult;

}

unsigned char UTIL_GetKey(unsigned char wait_sec)
{
	unsigned int Timeout;
	DRV_OUT *KBoard;

return KEY_ENTER;

	Timeout=wait_sec*100;
	Os__timer_start(&Timeout);
	Os__xdelay(1);
	KBoard=Os__get_key();
	while ((Timeout!=0)&&(KBoard->gen_status==DRV_RUNNING));
	Os__timer_stop(&Timeout);
	if(Timeout==0)
	{
		Os__abort_drv(drv_mmi);
		Os__xdelay(10);
		return  99;
	}
	return KBoard->xxdata[1];
}

UCHAR	UTIL_DeleteAllData(UCHAR* pucFileName)
{
	int		iHandle;
	long	uiFileSize;

	uiFileSize = OSAPP_FileSize((char*)pucFileName);
	if(uiFileSize>0)
	{
		iHandle = OSAPP_OpenFile((char*)pucFileName,O_WRITE);
		if(iHandle>=0)
		{
			OSAPP_FileTrunc(iHandle,0);
			OSAPP_FileClose(iHandle);
		}
	}
	return (EMVERROR_SUCCESS);
}

void UTIL_DeletaAllEMVfileData(void)
{
	UTIL_DeleteAllData((PUCHAR)CONSTPARAMFILE);
	UTIL_DeleteAllData((PUCHAR)EMVCONFIGFILE);
	UTIL_DeleteAllData((PUCHAR)CAPKFILE);
	UTIL_DeleteAllData((PUCHAR)TERMSUPPORTAPPFILE);
	UTIL_DeleteAllData((PUCHAR)TRANSLOG);
	UTIL_DeleteAllData((PUCHAR)EXCEPTFILE);
	UTIL_DeleteAllData((PUCHAR)BATCHRECORD);
	UTIL_DeleteAllData((PUCHAR)IPKREVOKEFILE);
	UTIL_DeleteAllData((PUCHAR)ERRICCTRANS);
}

UCHAR	UTIL_EMVInformMsg(PUCHAR	pucInformMsg)
{
    //Os__clr_display(255);
    //Os__GB2312_display(2,0,pucInformMsg);
	UTIL_GetKey(3);
	return EMVERROR_SUCCESS;
}


UCHAR	UTIL_IDCardVerify(void)
{
	UCHAR	ucResult,ucIDType,aucID[30],aucBuff[40],ucKey;
	USHORT	uiIDLen,uiLen;


	ucResult=EMVERROR_SUCCESS;

	if(!ucResult)
	{
		memset(aucID,0x00,sizeof(aucID));
		uiIDLen=sizeof(aucID);
		ucResult=EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x9F\x61",aucID,&uiIDLen);
	}
	if(!ucResult)
	{
		uiLen =sizeof(UCHAR);
		ucResult=EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x9F\x62",&ucIDType,&uiLen);
	}

	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"ID:");
		memcpy(aucBuff+strlen((char*)aucBuff),aucID,uiIDLen);

        //Os__clr_display(255);
		OSMMI_DisplayASCII(0x30,0,0,aucBuff);

		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"���ͣ�");
		switch(ucIDType)
		{
		case 0x00:
			strcpy((char*)aucBuff+strlen((char*)aucBuff),"����֤");
			break;
		case 0x01:
			strcpy((char*)aucBuff+strlen((char*)aucBuff),"����֤");
			break;
		case 0x02:
			strcpy((char*)aucBuff+strlen((char*)aucBuff),"����");
			break;
		case 0x03:
			strcpy((char*)aucBuff+strlen((char*)aucBuff),"�뾳֤");
			break;
		case 0x04:
			strcpy((char*)aucBuff+strlen((char*)aucBuff),"��ʱ����֤");
			break;
		default://05
			strcpy((char*)aucBuff+strlen((char*)aucBuff),"����");
			break;

		}
        //Os__GB2312_display(1,0,aucBuff);
		OSMMI_DisplayASCII(0x30,4,0,(PUCHAR)"1.ID Verify Pass");
		OSMMI_DisplayASCII(0x30,5,0,(PUCHAR)"2.ID Verify Fail");
		OSMMI_DisplayASCII(0x30,7,0,(PUCHAR)"Pls Confirm");

		while(1)
		{
			ucKey=Os__xget_key();
			if(ucKey=='1')
			{
				ucResult=EMVERROR_SUCCESS;
				break;
			}
			else if(ucKey=='2')
			{
				ucResult=EMVERROR_CANCEL;
				break;
			}
			else
				continue;
		}
	}
	return ucResult;
}

void UTIL_ClearGlobalData(void)
{
	memset((unsigned char *)&NormalTransData,0,sizeof(NormalTransData));
	memset((unsigned char *)&ExtraTransData,0,sizeof(ExtraTransData));
	memset((unsigned char *)&RunData,0,sizeof(RunData));
	memset((unsigned char *)&ISO8583Data,0,sizeof(ISO8583Data));
	memset((unsigned char *)&DialParam,0,sizeof(DialParam));
	OSMEM_Memset((unsigned char *)&DataSave0,0,sizeof(DATASAVEPAGE0));
	OSMEM_Memset((unsigned char *)&DataSave1,0,sizeof(DATASAVEPAGE1));
	XDATA_Read_Vaild_File(DataSaveConstant);
	XDATA_Read_Vaild_File(DataSaveChange);
	XDATA_Read_Vaild_File(DataSaveCashier);
	XDATA_Read_Vaild_File(DataSaveTrans8583);
	XDATA_Read_Vaild_File(DataSaveTransInfo);

	EMV_Check_file();
	G_NORMALTRANS_ulTraceNumber = DataSave0.ChangeParamData.ulTraceNumber;

}


void UTIL_GetTerminalInfo(void)
{
	memset(G_RUNDATA_aucTerminalID,0,sizeof(G_RUNDATA_aucTerminalID));
	memcpy(G_RUNDATA_aucTerminalID,DataSave0.ConstantParamData.aucTerminalID,
		PARAM_TERMINALIDLEN);
	memcpy(G_RUNDATA_aucMerchantID,DataSave0.ConstantParamData.aucMerchantID,
		PARAM_MERCHANTIDLEN);

	memcpy(ConstParam.aucTerminalID,DataSave0.ConstantParamData.aucTerminalID,TERMINALIDLEN);
	memcpy(ConstParam.aucMerchantID,DataSave0.ConstantParamData.aucMerchantID,MERCHANTIDLEN);
	memcpy(ConstParam.aucMerchantName,DataSave0.ConstantParamData.aucMerchantName,MERCHANTNAMELEN);

	UTIL_WriteConstParamFile(&ConstParam);
}

unsigned char UTIL_UnLockKeyBoard(void)
{
	unsigned char aucCashierPass[CASH_MAXSUPERPASSWDLEN] ,aucCashierNo[CASH_MAXSUPERNO];
	unsigned char ucCashierIndex;
	unsigned char ucResult,ucKey;
	unsigned char FLAG = 0;

	while(1)
	{
		if(ucResult != SUCCESS)
			MSG_DisplayErrMsg(ucResult);

		ucResult = SUCCESS ;
		ucCashierIndex = DataSave0.ChangeParamData.ucCashierLogonIndex;

        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char *)" �������������");
        //Os__GB2312_display(1,0,(unsigned char *)" ���������Ա��:");

		memset(aucCashierNo,0,sizeof(aucCashierNo));
		ucKey =UTIL_Input(2,true,CASH_MAXSUPERNO,CASH_MAXSUPERNO,'N',aucCashierNo,NULL);
		util_Printf("\nucKey===%02x",ucKey);
		if (ucKey ==ERR_END)
		    return(ucKey);

		if(  ucKey!= KEY_ENTER)
		{
			continue;
		}
		if( ucResult == SUCCESS)
		{
			if(DataSave0.ChangeParamData.ucCashierLogonFlag == 0x55)
			{
				if((memcmp(DataSave0.Cashier_SysCashier_Data.aucCashierNo[ucCashierIndex] ,aucCashierNo ,CASH_CASHIERNOLEN ))
				&&(memcmp(DataSave0.Cashier_SysCashier_Data.aucSYSCashierNo ,aucCashierNo , CASH_SYSCASHIERNOLEN ))
				)
				{
					ucResult = ERR_CASH_NOTEXIST;
					continue;
				}else
				{

					if(!memcmp(DataSave0.Cashier_SysCashier_Data.aucSYSCashierNo ,aucCashierNo , CASH_SYSCASHIERNOLEN ))
					{
						FLAG = 1;
					}
					if(!memcmp(DataSave0.Cashier_SysCashier_Data.aucCashierNo[ucCashierIndex],aucCashierNo , CASH_CASHIERNOLEN ))
					{
						FLAG = 0;
					}
				}
			}else
			{
				if(memcmp(DataSave0.Cashier_SysCashier_Data.aucSYSCashierNo ,aucCashierNo , CASH_SYSCASHIERNOLEN)  )
				{
					ucResult = ERR_CASH_NOTEXIST;
					continue;
				}else
				{
					FLAG = 1;
				}
			}
		}
		//-----------CASH PASS-----
		if( ucResult == SUCCESS)
		{
            //Os__clr_display(1);
            //Os__GB2312_display(1,0,(unsigned char *)"���������Ա����:");
			memset(aucCashierPass,0,sizeof(aucCashierPass));
			if(FLAG == 0)
			{
				if( UTIL_Input(2,true,CASH_CASHIERPASSLEN,CASH_CASHIERPASSLEN,'P',aucCashierPass,NULL) != KEY_ENTER )
				{
					continue;
				}
			}else
			{
				if( UTIL_Input(2,true,CASH_SYSCASHIERPASSLEN,CASH_SYSCASHIERPASSLEN,'P',aucCashierPass,NULL) != KEY_ENTER )
				{
					continue;
				}
			}
		}
		//------------------------
		if(FLAG == 0)
		{
			if( memcmp(aucCashierPass,&DataSave0.Cashier_SysCashier_Data.aucCashierPass[ucCashierIndex],CASH_CASHIERPASSLEN) ==0)
			{
				ucResult =SUCCESS;
				break;
			}
			else
			{
                //Os__clr_display(255);
                //Os__GB2312_display(0,0,(unsigned char *)"�������");
				MSG_WaitKey(3);
				ucResult =SUCCESS;
				continue;
			}
		}else
		{
			if(memcmp(aucCashierPass,DataSave0.Cashier_SysCashier_Data.aucSYSCashierPass,CASH_SYSCASHIERPASSLEN) == 0)
			{
				DataSave0.ChangeParamData.ucCashierLogonFlag= 0 ;
				ucResult =SUCCESS;
				break;
			}
			else
			{
                //Os__clr_display(255);
                //Os__GB2312_display(0,0,(unsigned char *)"�������");
				MSG_WaitKey(3);
				ucResult =SUCCESS;
				continue;
			}
		}
	}
	DataSave0.ChangeParamData.ucCashierLogonFlag= 0 ;
	Os__saved_set(&DataSave0.ChangeParamData.ucTerminalLockFlag,0,1);
	XDATA_Write_Vaild_File(DataSaveChange);
    //Os__clr_display(255);
    //Os__GB2312_display(0,0,(unsigned char *)"�������!");
	MSG_WaitKey(5);
}

unsigned char UTIL_DispDateAndTime(unsigned char lin,unsigned col )
{
	unsigned char DispBuf[17];
	unsigned char Buf[11];

	memset(Buf,0,sizeof(Buf));
	memset(DispBuf,0,sizeof(DispBuf));

	UTIL_READ_DateTimeAndFormat(Buf);/*YYMMDDhhmm*/
	memcpy(DispBuf,"20",2);
	memcpy(&DispBuf[2],Buf,2);//YY
	memcpy(&DispBuf[4],"/",1);
	memcpy(&DispBuf[5],&Buf[2],2);//MM
	memcpy(&DispBuf[7],"/",1);
	memcpy(&DispBuf[8],&Buf[4],2);//DD
	memcpy(&DispBuf[10]," ",1);
	memcpy(&DispBuf[11],&Buf[6],2);//hh
	memcpy(&DispBuf[13],":",1);
	memcpy(&DispBuf[14],&Buf[8],2);//mm

    //Os__GB2312_display(lin,col,DispBuf);

}

unsigned char UTIL_SetInputCardMode(unsigned char * ucReadCardMode)
{

#ifdef USEINSERTCARD
			*ucReadCardMode=TRANS_INPUTMODE_SWIPECARD
						|TRANS_INPUTMODE_MANUALINPUT
						|TRANS_INPUTMODE_INSERTCARD
#ifdef READUNTOUCHCARD
						|TRANS_INPUTMODE_PUTCARD
#endif
						;
#else
			*ucReadCardMode=TRANS_INPUTMODE_SWIPECARD
						|TRANS_INPUTMODE_MANUALINPUT;
#endif
}

void UTIL_WaitPullOutICC(void)
{
	unsigned char ucResult;

	util_Printf("\n=====UTIL_WaitPullOutICC=======\n");
	//return 0;

	if(!Os__ICC_detect(0))
	{
        //Os__clr_display(255);
        //Os__GB2312_display(1,1,(unsigned char *)"��γ�IC��");
		Os__ICC_remove();
	}
#if SANDREADER
	if(ReaderSupport&&(ReaderType==READER_SAND))
	{
	    util_Printf("\n=======++++++++===========ucResult:%d",ucResult);
	    ucResult = Os__MIFARE_Remove();
	    util_Printf("\n=======++++++++===========ucResult:%d",ucResult);
	    while(ucResult)
	    {
                //Os__clr_display(255);
                //Os__GB2312_display(2,2,(uchar *)"��ȡ��...");
	            ucResult = Os__MIFARE_Remove();
	            util_Printf("����ֵ:%02x\n",ucResult);
	    }
	       Os__MIFARE_PowerRF(0);
	}
#endif
}



void UTIL_READ_DateTimeAndFormat(unsigned char * aucOutBuf/*YYMMDDhhmm*/)
{
	unsigned char aucDateTimeBuf[10];
	unsigned char aucDateTimeFormat[10];

	Os__read_date_time(aucDateTimeBuf);

	aucDateTimeFormat[0] = aucDateTimeBuf[8];
	aucDateTimeFormat[1] = aucDateTimeBuf[9];
	aucDateTimeFormat[2] = aucDateTimeBuf[6];
	aucDateTimeFormat[3] = aucDateTimeBuf[7];
	aucDateTimeFormat[4] = aucDateTimeBuf[4];
	aucDateTimeFormat[5] = aucDateTimeBuf[5];
	aucDateTimeFormat[6] = aucDateTimeBuf[0];
	aucDateTimeFormat[7] = aucDateTimeBuf[1];
	aucDateTimeFormat[8] = aucDateTimeBuf[2];
	aucDateTimeFormat[9] = aucDateTimeBuf[3];

	memcpy(aucOutBuf ,aucDateTimeFormat ,10 );

}

void UTIL_Beep(void)
{
	char i;
//	for(i=0;i<7;i++)Os__beep();
	return ;
}


unsigned char UTIL_WaitGetKey(unsigned char wait_sec)
{
    	unsigned int Timeout;
    	DRV_OUT *KBoard;

#ifdef GUI_PROJECT
	return KEY_ENTER;
#endif

    	Timeout=wait_sec*100;
    	Os__light_on();
    	Os__timer_start(&Timeout);
    	Os__xdelay(1);
      KBoard=Os__get_key();
      while ((Timeout!=0)&&(KBoard->gen_status==DRV_RUNNING));
      Os__timer_stop(&Timeout);
      if(Timeout==0)
      {
            Os__abort_drv(drv_mmi);
            Os__xdelay(10);
            if(!DataSave0.ConstantParamData.BackLightFlag)
                Os__light_off();
            return  99;
      }
      return KBoard->xxdata[1];
}

unsigned char UTIL_WaitGetKey_AD(unsigned char wait_sec)
{
    	unsigned int Timeout;
    	DRV_OUT *KBoard;

    	Timeout=wait_sec;
    	Os__light_on();

    	Os__timer_start(&Timeout);
    	Os__xdelay(1);
        KBoard=Os__get_key();
        while ((Timeout!=0)&&(KBoard->gen_status==DRV_RUNNING));
        Os__timer_stop(&Timeout);
        if(Timeout==0)
        {
            Os__abort_drv(drv_mmi);
            Os__xdelay(10);
	        return  99;
        }
        return KBoard->xxdata[1];
}


unsigned char UTIL_AccumulatRunTime( void )
{
unsigned char aucBuf[32],aucTmp[32];
unsigned long ulTemp;

	Os__read_time(aucBuf);
	asc_bcd(aucTmp,2,aucBuf,4);
	if( aucTmp[0]!=DataSave0.ChangeParamData.ucDialStartStamp )
	{
		Os__saved_set(&DataSave0.ChangeParamData.ucDialStartStamp,
						aucTmp[0],1);
		ulTemp=DataSave0.ChangeParamData.ulDialTimeTotal;
		ulTemp++;
		if( ulTemp>=9999 )
		{
			ulTemp=0;
			Os__saved_copy(	(unsigned char *)&ulTemp,
				(unsigned char *)&DataSave0.ChangeParamData.ulDialConnectCnt,
				sizeof(unsigned long));
			Os__saved_copy(	(unsigned char *)&ulTemp,
					(unsigned char *)&DataSave0.ChangeParamData.ulDialTotalCount,
					sizeof(unsigned long));
			Os__saved_copy(	(unsigned char *)&ulTemp,
					(unsigned char *)&DataSave0.ChangeParamData.ulDialConnectRate,
					sizeof(unsigned long));
		}
		Os__saved_copy(	(unsigned char *)&ulTemp,
			(unsigned char *)&DataSave0.ChangeParamData.ulDialTimeTotal,
			sizeof(unsigned long));
		XDATA_Write_Vaild_File(DataSaveChange);
//		util_Printf( "ulDialTimeTotal=%d ucDialStartStamp=%d aucTmp[0]=%2x\n",
//				DataSave0.ChangeParamData.ulDialTimeTotal,
//				DataSave0.ChangeParamData.ucDialStartStamp,
//				aucTmp[0] );
	}
	return 1;
}

unsigned char UTIL_format_time_bcd_str( unsigned char *Ptd, unsigned char *Pts )
{
    unsigned char i ;

    for ( i = 0 ; i < 3 ; i++, Pts++)
    {
	*Ptd++ = (*Pts >> 4) | 0x30;
	*Ptd++ = (*Pts & 0x0F) | 0x30;
	if ( (!i)||(i==1))
	    *Ptd++ = ':';
    }
    *Ptd = 0;
    return(SUCCESS);
}

unsigned char UTIL_format_date_bcd_str( unsigned char *Ptd, unsigned char *Pts )
{
    unsigned char i ;

	for ( i = 0 ; i < 4; i++,Pts++)   //old is 3
	{
	*Ptd++ = (*Pts >> 4) | 0x30;
	*Ptd++ = (*Pts & 0x0F) | 0x30;
	if ( ( i == 1) || ( i == 2) ) *Ptd++ = '/';
	}
//	*Ptd = 0;
    return(SUCCESS);
}

unsigned char UTIL_Is_Trans_Empty(void)
{
	unsigned int uiTransNum=0 ,ucResult;
	if((DataSave0.TransInfoData.ForeignTransTotal.uiTotalNb == 0)
		  &&(DataSave0.TransInfoData.TransTotal.uiTotalNb == 0)
		)
		{
			if(G_NORMALTRANS_ucTransType==TRANS_SETTLE)
			{
				ucResult = FILE_ReadRecordNumByFileName(ERRICCTRANS, &uiTransNum);
				util_Printf("UTIL_Is_Trans_Empty()----FILE_ReadRecord ---uiTransNum=%d ----ucResult=%02x \n",uiTransNum,ucResult);
				if(!ucResult)
				{
					if(uiTransNum)
						return SUCCESS;
					else
						return(ERR_TRANSEMPTY);
				}else
				{
            //Os__clr_display(255);
            //Os__GB2312_display(0,0,(unsigned char *)"------����------");
            //Os__GB2312_display(2,0,(uchar *)"  �޽�����ˮ��");
					UTIL_Beep();
					MSG_WaitKey(5);
					return(ERR_CANCEL);
				}
			}
			else return(ERR_TRANSEMPTY);
		}
	else
		return(SUCCESS);
}

void UTIL_IncreaseTraceNumber(void)
{
	ULONG_C51 ulTraceNumber;

	TransReqInfo.uiTraceNumber = ConstParam.uiTraceNumber;
	ulTraceNumber = DataSave0.ChangeParamData.ulTraceNumber;
	G_NORMALTRANS_ulTraceNumber = ulTraceNumber;
    	ulTraceNumber ++;
	if( ulTraceNumber > 999999 )
	{
		ulTraceNumber = 1;
	}
	ConstParam.uiTraceNumber = ulTraceNumber;
	Os__saved_copy(	(unsigned char *)&ulTraceNumber,
			(unsigned char *)&DataSave0.ChangeParamData.ulTraceNumber,
			sizeof(ULONG_C51));

	UTIL_WriteConstParamFile(&ConstParam);
	XDATA_Write_Vaild_File(DataSaveChange);
}


unsigned char OSUTIL_Input(
                unsigned int uiTimeout,
                unsigned char ucLine,
                unsigned char ucMin,
                unsigned char ucMax,
                unsigned char ucType,
                unsigned char *pucBuf,
                char *pcMask)

{
    const char acKeyTab[][7]=
    {
        {"0 _*\0"},
        {"1QZ\0"},
        {"2ABC\0"},
        {"3DEF\0"},
        {"4GHI\0"},
        {"5JKL\0"},
        {"6MNO\0"},
        {"7PRS\0"},
        {"8TUV\0"},
        {"9WXY\0"},
	 {"0.,-\0"},
    };
    unsigned char ucResult;
    unsigned char aucDisp[40];
    unsigned char aucInput[40];
    unsigned char ucKey;
    unsigned char ucLastKey;
    unsigned char ucKeyTabOffset;
    unsigned char ucEnd;
    unsigned char ucRedraw;
    unsigned char ucCount;
    unsigned char ucOffset;
    DRV_OUT *pdKey;
    unsigned int    uiLastTime;

    memset(&aucDisp[0],0,sizeof(aucDisp));
    memset(&aucInput[0],0,sizeof(aucInput));
    ucLastKey = 0;
    ucEnd = FALSE;
    ucRedraw = FALSE;
    ucCount = 0;
    ucResult = SUCCESS;

    if( strlen((char *)pucBuf))
    {
        ucRedraw = TRUE;
        ucCount = strlen((char *)pucBuf);
        if( ucCount > sizeof(aucInput))
            ucCount = sizeof(aucInput);
        memcpy(aucInput,pucBuf,ucCount);
    }
    if( uiTimeout )
    {
        uiTimeout = uiTimeout*ONESECOND;
        Os__timer_start(&uiTimeout);
    }else
    {
        uiTimeout = 1;
    }
    uiLastTime = uiTimeout;

    do
    {
        if( ucRedraw == TRUE)
        {
            memset(&aucDisp[0],0,sizeof(aucDisp));
            if( ucCount > MAXLINECHARNUM)
            {
                if(  (ucType == 'p')
                   ||(ucType == 'P')
                  )
                {
                    memset(&aucDisp[0],'*',MAXLINECHARNUM);
                }else
                {
                    memcpy(&aucDisp[0],&aucInput[ucCount-16],MAXLINECHARNUM);
                }
            }else
            {
                if(  (ucType == 'p')
                   ||(ucType == 'P')
                  )
                {
                    memset(&aucDisp[0],'*',ucCount);
                }else
                {
                    memcpy(&aucDisp[0],&aucInput[0],ucCount);
                }

            }
            //Os__clr_display(ucLine);
            //Os__display(ucLine,0,&aucDisp[0]);
            ucRedraw = FALSE;
        }

        pdKey = Os__get_key();

		do{
		}while(  (pdKey->gen_status==DRV_RUNNING)
			   &&(uiTimeout !=0)
			   );

        if (uiTimeout == 0)
        {
           Os__abort_drv(drv_mmi);
            ucResult = ERR_CANCEL;
            break;
        }else
        {
            ucKey = pdKey->xxdata[1];
        }
        if(  (ucKey >= '0')
           &&(ucKey <= '9')
          )
        {
            if( pcMask )
            {
                if( !strchr((char *)pcMask,ucKey ))
                    continue;
            }
            switch(ucType)
            {
            case 'h':
            case 'H':
                if( ucLastKey != ucKey)
                {
                    if( ucCount >= ucMax)
                    {
                        Os__beep();
                    }else
                    {
                        ucLastKey = ucKey;
                        ucKeyTabOffset = 0;
                        aucInput[ucCount++] = ucKey;
                        ucRedraw = TRUE;
                    }
                }else
                {
                    if(  uiLastTime )
                    {
                        if( ucCount )
                        {
                            ucOffset = ucCount - 1;
                        }else
                        {
                            ucOffset = 0;
                        }
                        if(  (ucKey >= '2')
                           &&(ucKey <= '3')
                          )
                        {
                            if( ucKeyTabOffset < strlen(acKeyTab[ucKey-'0'])-1)
                           {
                                ucKeyTabOffset ++;
                                ucRedraw = TRUE;
                            }else
                            {
                                if( ucKeyTabOffset )
                                {
                                    ucKeyTabOffset = 0;
                                    ucRedraw = TRUE;
                                }
                            }
                            aucInput[ucOffset] = acKeyTab[ucKey-'0'][ucKeyTabOffset];
                        }else
                        {
                            if( ucCount >= ucMax)
                            {
                                Os__beep();
                            }else
                            {
                                ucLastKey = ucKey;
                                ucKeyTabOffset = 0;
                                aucInput[ucCount++] = ucKey;
                                ucRedraw = TRUE;
                            }
                        }
                    }else
                    {
                        if( ucCount >= ucMax)
                        {
                            Os__beep();
                        }else
                        {
                            ucLastKey = ucKey;
                            ucKeyTabOffset = 0;
                            aucInput[ucCount++] = ucKey;
                            ucRedraw = TRUE;
                        }
                    }
                }
//                Os__timer_stop(&uiLastTime);
//                uiLastTime = 200;
//                Os__timer_start(&uiLastTime);
                break;
            case 'a':
            case 'A':
                if( ucLastKey != ucKey)
                {
                    if( ucCount >= ucMax)
                    {
                        Os__beep();
                    }else
                    {
                        ucLastKey = ucKey;
                        ucKeyTabOffset = 0;
                        aucInput[ucCount++] = ucKey;
                        ucRedraw = TRUE;
                    }
                }else
                {
                    if(  uiLastTime )
                    {
                        if( ucCount )
                        {
                            ucOffset = ucCount - 1;
                        }else
                        {
                            ucOffset = 0;
                        }
                        if( ucKeyTabOffset < strlen(acKeyTab[ucKey-'0'])-1)
                        {
                            ucKeyTabOffset ++;
                            ucRedraw = TRUE;
                        }else
                        {
                            if( ucKeyTabOffset != 0)
                            {
                                ucKeyTabOffset = 0;
                                ucRedraw = TRUE;
                            }
                        }
                        aucInput[ucOffset] = acKeyTab[ucKey-'0'][ucKeyTabOffset];
                    }else
                    {
                        if( ucCount >= ucMax)
                        {
                            Os__beep();
                        }else
                        {
                            ucLastKey = ucKey;
                            ucKeyTabOffset = 0;
                            aucInput[ucCount++] = ucKey;
                            ucRedraw = TRUE;
                        }
                    }
                }
//                Os__timer_stop(&uiLastTime);
//                uiLastTime = 200;
//                Os__timer_start(&uiLastTime);
                break;
            default:
                if( ucCount >= ucMax)
                {
                    Os__beep();
                    break;
                }
                aucInput[ucCount++] = ucKey;
                ucRedraw = TRUE;
                break;
            }
        }else
        {
            switch(ucKey)
            {
            case KEY_CLEAR:
                if( ucCount )
                {
                    ucCount = 0;
                    ucLastKey = 0;
                    memset(&aucInput[0],0,sizeof(aucInput));
                    memset(&aucDisp[0],0,sizeof(aucDisp));
                    ucRedraw = TRUE;
                }else
                {
                    ucEnd = TRUE;
                    ucResult = ERR_CANCEL;
                }
                break;
            case KEY_00_PT:
		if(  (ucType == 'h')
		   ||(ucType == 'H')
		   ||(ucType == 'a')
		   ||(ucType == 'A')
		  )
		{
			if( ucLastKey != ucKey)
			{
        			if( ucCount >= ucMax)
        			{
        				Os__beep();
        				break;
        			}
				ucLastKey = ucKey;
				ucKeyTabOffset = 0;
				if((ucKey != '0')&&(ucKey != ',')&&(ucKey != '.')&&(ucKey != '-'))
					aucInput[ucCount++] = '0';
				else
					aucInput[ucCount++] = ucKey;
				ucRedraw = TRUE;
			}else
			{
				if(  (uiLastTime-uiTimeout) < 1*ONESECOND)
				{
					if( ucCount )
					{
						ucOffset = ucCount - 1;
					}else
					{
						ucOffset = 0;
					}
					if( ucKeyTabOffset < strlen((char *)acKeyTab[10])-1)
					{
						ucKeyTabOffset ++;
						ucRedraw = TRUE;
					}else
					{
						if( ucKeyTabOffset != 0)
						{
							ucKeyTabOffset = 0;
							ucRedraw = TRUE;
						}
					}
					aucInput[ucOffset] = acKeyTab[10][ucKeyTabOffset];
				}else
				{
					if( ucCount >= ucMax)
					{
						Os__beep();
						break;
					}
					ucLastKey = ucKey;
					ucKeyTabOffset = 0;
					if((ucKey != '0')&&(ucKey != ',')&&(ucKey != '.')&&(ucKey != '-'))
						aucInput[ucCount++] = '0';
					else
						aucInput[ucCount++] = ucKey;
					ucRedraw = TRUE;
				}
			}
		}else
		{
			if( ucCount > (ucMax-2))
			{
				Os__beep();
				break;
			}
			memcpy(&aucInput[ucCount],"00",2);
			ucCount = ucCount +2;
			ucRedraw = TRUE;
			break;
		}
		break;
            case KEY_BCKSP:
                if( ucCount )
                {
                    ucCount --;
                    aucInput[ucCount] = 0;
                    ucRedraw = TRUE;
                }
                break;
            case KEY_ENTER:
                if( ucCount < ucMin )
                {
                    Os__beep();
                }else
                {
                    if( pucBuf )
                    {
                        memcpy(pucBuf,&aucInput[0],ucCount);
                        *(pucBuf+ucCount) = 0x00;
                    }
                    ucEnd = TRUE;
                }
                break;
            default :
                break;
            }
        }
        uiLastTime = uiTimeout;
        if( ucEnd == TRUE)
        {
            break;
        }
    }while(1);
    Os__timer_stop(&uiLastTime);
    Os__timer_stop(&uiTimeout);
    return(ucResult);
}


unsigned char UTIL_EMVInputTransAmount(void)
{
	unsigned char ucResult=SUCCESS,aucAmountBuf[40],ucKey;
	unsigned long ulAmount,ulAmountLimit;
	unsigned char tmpBuf[17];
	util_Printf("\nUTIL_EMVInputTransAmount==abc");
	util_Printf("\nG_NORMALTRANS_ucTransType==ABC=%d\n",G_NORMALTRANS_ucTransType);
	if( ucResult == SUCCESS )
	{
		switch( G_NORMALTRANS_ucTransType )
		{
			case TRANS_VOIDPURCHASE:
			case TRANS_VOIDPREAUTHFINISH:
				ulAmountLimit = G_NORMALTRANS_ulAmount;
				break;
			default:
				ulAmountLimit = 0x3B9AC9FF;
				break;
		}
	}

	/* Input Amount */
	if( ucResult == SUCCESS )
	{
		switch( G_NORMALTRANS_ucTransType )
		{
		case TRANS_QUERYBAL:
			G_NORMALTRANS_ulAmount =0;/*TEST*/
			break;
		case TRANS_VOIDPURCHASE:
		case TRANS_VOIDPREAUTHFINISH:
			break;
		case TRANS_TIPADJUST:
			if((ucResult = UTIL_displayCardNum())!= SUCCESS)
				return ucResult;
			if(G_NORMALTRANS_ucOldTransType == TRANS_OFFPREAUTHFINISH)
			{
                //Os__clr_display(255);
				memset(tmpBuf,0x00,sizeof(tmpBuf));
				UTIL_Form_Montant(tmpBuf,G_NORMALTRANS_ulAmount,2);
				OSMMI_DisplayASCII(0x30,1,10, tmpBuf);
                //Os__GB2312_display(0, 0, (uchar *)"ԭ���:");
                //Os__GB2312_display(1, 0, (uchar *)"�����������:");
			}
			else
			{
				MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
                //Os__GB2312_display(1, 0, (uchar *)"������С�ѽ��:");
			}
			ulAmount=0;
			ucResult=UTIL_InputAmount(2,&ulAmount,1,ulAmountLimit);
			if( ucResult == SUCCESS)
			{
				if(G_NORMALTRANS_ucOldTransType != TRANS_OFFPREAUTHFINISH)
				{
					if( ulAmount>( (G_NORMALTRANS_ulAmount*DataSave0.ConstantParamData.usTipFeePre)/100))
					{
                        //Os__clr_display(1);
                        //Os__clr_display(2);
                        //Os__clr_display(3);
                        //Os__GB2312_display(1, 0, (uchar * )"  ��������Ч");
                        //Os__GB2312_display(2, 0, (uchar * )"   ��С�Ѷ��");
						 UTIL_WaitGetKey(10);
						return( ERR_CANCEL) ;
					}
					util_Printf("һ����С�ѽ��׵���\n");
					G_NORMALTRANS_ulFeeAmount = ulAmount;
					G_RUNDATA_ucAdjustFlag = 0;	/*һ��С�ѽ��׵���*/
				}
				else
				{
					/*���߽��������ͻ�δ���ͣ�С�ѽ����Ϊ��������*/
					if(ulAmount<G_NORMALTRANS_ulAmount)
					{
						G_NORMALTRANS_ulFeeAmount = G_NORMALTRANS_ulAmount-ulAmount;
						G_RUNDATA_ucAdjustFlag = 1;
						/*���ߵ�����С��ԭ���*/
					}
					else
					{
						G_NORMALTRANS_ulFeeAmount = ulAmount-G_NORMALTRANS_ulAmount;
						G_RUNDATA_ucAdjustFlag = 2;
						/*���ߵ��������ԭ���*/
					}
					G_NORMALTRANS_ulAmount = ulAmount;
				}
				if( ucResult == SUCCESS)
					ucResult = UTIL_CheckTransAmount();
			}
			break;
		default:
#ifndef GUI_PROJECT			
			MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
            //Os__GB2312_display(1, 0, (uchar *) "��������:");
			ulAmount=0;
			ucResult=UTIL_InputAmount(2,&ulAmount,1,ulAmountLimit); //��ʾ��������
			G_NORMALTRANS_ulAmount = ulAmount;
#else
//			G_NORMALTRANS_ulAmount=ProUiFace.UiToPro.ulAmount;
			G_NORMALTRANS_ulAmount=1;
			util_Printf("\n =[000000]===============ulAmount [%d]",G_NORMALTRANS_ulAmount);
			ucResult = 0;
			DataSave0.ConstantParamData.ulRefundMaxAmount=10;
#endif
			if(G_NORMALTRANS_ucTransType == TRANS_REFUND)
			{

				if( G_NORMALTRANS_ulAmount>DataSave0.ConstantParamData.ulRefundMaxAmount )
				{
                    //Os__clr_display(1);
                    //Os__clr_display(2);
                    //Os__clr_display(3);
                    //Os__GB2312_display(1, 0, (uchar * )"  ��������Ч");
                    //Os__GB2312_display(2, 0, (uchar * )"   ���˻����");
   				       UTIL_WaitGetKey(10);
					ucResult = ERR_CANCEL;
				}
			}
			if( ucResult == SUCCESS)
				ucResult = UTIL_CheckTransAmount();
			break;
		}
	}
#ifndef GUI_PROJECT
	if(ucResult==SUCCESS&&(G_NORMALTRANS_ucTransType==TRANS_REFUND
		||G_NORMALTRANS_ucTransType==TRANS_CREDITSREFUND
		||G_NORMALTRANS_ucTransType==TRANS_PREAUTHSETTLE))
	{
		memset( aucAmountBuf, 0, sizeof(aucAmountBuf));
		UTIL_Form_Montant(&aucAmountBuf[5],G_NORMALTRANS_ulAmount,2);
        //Os__clr_display(255);
		memcpy(aucAmountBuf,"��",6);
		MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
        //Os__GB2312_display(1, 0, aucAmountBuf);
        //Os__GB2312_display(3, 0, (uchar *)"�밴[ȷ��]��ȷ��");
		ucKey =Os__xget_key();
		if(ucKey!=KEY_ENTER)
			ucResult =ERR_CANCEL;
	}
#endif
	return ucResult;
}
unsigned char UTIL_InputMobileAmount(void)
{
	unsigned long ulAmount,ulAmountLimit;
	unsigned char ucResult;

	switch (G_NORMALTRANS_ucTransType)
	{
		case TRANS_CUPMOBILE:
			ulAmountLimit = 0x3B9AC9FF;
			break;
		case TRANS_VOIDCUPMOBILE:
			ulAmountLimit = G_NORMALTRANS_ulAmount;
			break;
		default:
			break;
	}
	switch (G_NORMALTRANS_ucTransType)
	{
		case TRANS_CUPMOBILE:
			MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
            //Os__GB2312_display(1, 0, (uchar *) "��������:");
			ulAmount=0;
			ucResult=UTIL_InputAmount(2,&ulAmount,1,ulAmountLimit); //��ʾ��������
			G_NORMALTRANS_ulAmount = ulAmount;
			if( ucResult == SUCCESS)
				ucResult = UTIL_CheckTransAmount();
			break;
		default:
			break;
	}
	return (ucResult);
}

unsigned char UTIL_displayCardNum(void)
{
	unsigned char ucResult ;
	unsigned char tmpBuf[30];
	unsigned char dispBuf[30];
	unsigned char dispBufCardNo[20];
	unsigned short cardNumLen;

	while(1)
	{
		MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);

		memset(tmpBuf,0x00,sizeof(tmpBuf));
		memset(dispBuf,0x00,sizeof(dispBuf));

		util_Printf("��ѯ�ƶ����ѽ���:\n");
		util_Printf("������:%d\n",G_NORMALTRANS_ucSourceAccLen);
   	    if(G_NORMALTRANS_ucSourceAccLen%2)
   	    {
   	    	bcd_asc(tmpBuf,G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen+1);
   	    }
   	    else
   	    {
   	    	bcd_asc(tmpBuf,G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen);
   	    }
		memset(dispBufCardNo,0x00,sizeof(dispBufCardNo));
		memcpy(dispBufCardNo,tmpBuf,G_NORMALTRANS_ucSourceAccLen);
		cardNumLen=strlen((char *)tmpBuf);
		util_Printf("\ndispBufCardNo = %s \n",dispBufCardNo);
//-----------------------------------------------------------------------
        //Os__GB2312_display(1,0,(unsigned char *)"ԭ����:");
   	memcpy(ProUiFace.ProToUi.aucLine2,"ԭ����:",strlen((char*)"ԭ����:"));
	
util_Printf(".1.......UTIL_displayCardNum............... dispBuf= %s \n",dispBuf);
#ifndef GUI_PROJECT
		memset(dispBuf,0x00,sizeof(dispBuf));
		memcpy( dispBuf, dispBufCardNo, cardNumLen);
		OSMMI_DisplayASCII(0x30,4,0, dispBuf);
#endif
util_Printf(".2.......UTIL_displayCardNum............... dispBuf= %s \n",dispBuf);
				
   	memcpy(ProUiFace.ProToUi.aucLine3,dispBufCardNo,strlen((char*)dispBufCardNo));


		//------------------
		memset(tmpBuf,0x00,sizeof(tmpBuf));
		memset(dispBuf,0x00,sizeof(dispBuf));
		memcpy( dispBuf, "VOUCHER NO:", 11);
util_Printf(".3.......UTIL_displayCardNum............... dispBuf= %s \n",dispBuf);
		
		long_asc(tmpBuf, 6, &G_NORMALTRANS_ulOldTraceNumber);

util_Printf("G_NORMALTRANS_ulOldTraceNumber = %s \n",tmpBuf);
		
		memcpy( &dispBuf[11], tmpBuf, 16);
#ifndef GUI_PROJECT		
		OSMMI_DisplayASCII(0x30,5,0, dispBuf);
#endif
   	memcpy(ProUiFace.ProToUi.aucLine4,dispBuf,strlen((char*)dispBuf));		
		//------------------
		memset(tmpBuf,0x00,sizeof(tmpBuf));
		memset(dispBuf,0x00,sizeof(dispBuf));
		memcpy( dispBuf, "AMOUNT:", 7);
		util_Printf("�ֽ������� [%02x] ԭ���� [%02x] \n",G_NORMALTRANS_ucTransType,G_NORMALTRANS_ucOldTransType);
		if((G_NORMALTRANS_ucTransType == TRANS_UNDOOFF)
			&&(G_NORMALTRANS_ucOldTransType == TRANS_TIPADJUST)
			||(G_NORMALTRANS_ucOldTransType == TRANS_TIPADJUSTOK)
			)
			UTIL_Form_Montant(tmpBuf,G_NORMALTRANS_ulFeeAmount,2);
		else
			UTIL_Form_Montant(tmpBuf,G_NORMALTRANS_ulAmount,2);

		util_Printf("G_NORMALTRANS_ulAmount = [%s] С�� = [%s]\n",tmpBuf,tmpBuf);
		memcpy( &dispBuf[7], tmpBuf, 12);
#ifndef GUI_PROJECT				
		OSMMI_DisplayASCII(0x30,6,0, dispBuf);
#endif
memcpy(ProUiFace.ProToUi.aucLine5,dispBuf,strlen((char*)dispBuf));
		//------------------
		memset(tmpBuf,0x00,sizeof(tmpBuf));
		memset(dispBuf,0x00,sizeof(dispBuf));
		memcpy( dispBuf, "REF.NO:",7 );
		memcpy(tmpBuf, G_NORMALTRANS_aucOldRefNumber, TRANS_REFNUMLEN);
		util_Printf("G_NORMALTRANS_aucOldRefNumber = %s \n",tmpBuf);
		memcpy( &dispBuf[7], tmpBuf, TRANS_REFNUMLEN);
#ifndef GUI_PROJECT		
		OSMMI_DisplayASCII(0x30,7,0, dispBuf);
#endif
memcpy(ProUiFace.ProToUi.aucLine6,dispBuf,strlen((char*)dispBuf));
ProUiFace.ProToUi.uiLines=6;		
		
		switch(MSG_WaitKey(60))
		{
			case KEY_ENTER:
				ucResult = SUCCESS;
				break;
			case KEY_CLEAR:
				ucResult = ERR_CANCEL;
				break;
			case ERR_APP_TIMEOUT:
				ucResult = ERR_CANCEL;
				break;
			default:
				continue;
		}
		break;
	}
	return ucResult;
}

unsigned char UTIL_InputEncryptPIN(unsigned char * pucEnPIN,unsigned char * pucPinLen)
{
	unsigned char ucResult=0;
	unsigned char aucPIN[13];
	unsigned char aucAmountBuf[20];
	unsigned char aucDispYUAN[15];


	util_Printf("\n.0007.001.Trans_Process..........[%02x].\n",ucResult);

	if(G_NORMALTRANS_ucTransType==TRANS_EC_CASHLOAD||G_NORMALTRANS_ucTransType==TRANS_EC_UNASSIGNLOAD)
		return SUCCESS;

	util_Printf("\n.0099.001.001.UTIL_InputEncryptPIN.G_EXTRATRANS_ucInputPINLen..........[%02x][%d].\n",ucResult,G_EXTRATRANS_ucInputPINLen);
		
	if(G_EXTRATRANS_ucInputPINLen)
		return SUCCESS;
		util_Printf("\n.0007.003.Trans_Process..........[%02x].\n",ucResult);
	
	memset( aucAmountBuf, 0, sizeof(aucAmountBuf));
	memset( aucDispYUAN, 0, sizeof(aucDispYUAN));
	UTIL_Form_Montant(&aucAmountBuf[5],G_NORMALTRANS_ulAmount,2);
	memcpy(aucAmountBuf,"��",6);
	G_RUNDATA_ucQInputPinFlag =1;
	while(1)
	{
#ifndef GUI_PROJECT
		memset(aucPIN,0,sizeof(aucPIN));
        //Os__clr_display(255);
		util_Printf("DataSave0.ConstantParamData.Pinpadflag = %02x\n",DataSave0.ConstantParamData.Pinpadflag);
		if(DataSave0.ConstantParamData.Pinpadflag ==1)
		{
			//---POS DISPLAY
			Os__light_on_pp();
			Os__clr_display_pp(255);

			MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
			if(G_NORMALTRANS_ucTransType!=TRANS_QUERYBAL)
			{
                //Os__GB2312_display(1, 0, aucAmountBuf);
				Os__GB2312_display_pp(0, 0, aucAmountBuf);
			}else if(G_NORMALTRANS_ucTransType == TRANS_QUERYBAL)
			{
				Os__clr_display_pp(255);
				Os__GB2312_display_pp(0, 1, (uchar * )" (����ѯ)");
			}
			Os__GB2312_display_pp(1, 0, (uchar *) "��������������:");
            //Os__GB2312_display(2, 0, (uchar *) "��ֿ�����������");
            //Os__GB2312_display(3, 0, (uchar * )"�������밴ȷ�ϼ�");
		}
		else
		{
			if(G_NORMALTRANS_ucTransType!=TRANS_QUERYBAL)
			{
                //Os__GB2312_display(0, 0, aucAmountBuf);
                //Os__GB2312_display(1, 0, (uchar *) "��������������:");
                //Os__GB2312_display(2, 0, (uchar *) "��ֿ�����������");
                //Os__GB2312_display(3, 0, (uchar * )"�������밴ȷ�ϼ�");
			}else
			{
				MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
                //Os__GB2312_display(1, 0, (uchar * )"��������������:");
                //Os__GB2312_display(2, 0, (uchar *) "��ֿ�����������");
                //Os__GB2312_display(3, 0, (uchar * )"�������밴ȷ�ϼ�");
			}
		}
#endif
		if(DataSave0.ConstantParamData.Pinpadflag ==1)
		{
			ucResult = EMV_UTIL_Input_PP();
			util_Printf("ucResult1 = %02x\n", ucResult);
			UTIL_PINPADDispLOGO();
			util_Printf("ucResult2 = %02x\n", ucResult);
		}
		else
		{
			ucResult = EMV_UTIL_Input();
		}
		UTIL_PINPADDispLOGO();
		util_Printf("ucResult3 = %02x\n", ucResult);
		if(ucResult == ERR_CANCEL||ucResult == SUCCESS||ucResult == ERR_END||ucResult ==EMVERROR_BYPASS)
			break;
	}
	memcpy(pucEnPIN ,  G_EXTRATRANS_aucPINData, G_EXTRATRANS_ucInputPINLen);
	*pucPinLen = G_EXTRATRANS_ucInputPINLen;
	return(ucResult);
}
unsigned char UTIL_InputEncryptPIN_EC(unsigned char * pucEnPIN,unsigned char * pucPinLen)
{
	unsigned char ucResult,uiI;
	unsigned char aucPIN[13];
	unsigned char aucAmountBuf[20];
	unsigned char aucDispYUAN[15];
	unsigned char ucTemPinLen,aucTemPin[8],aucTemCardPan[6];

	memset( aucAmountBuf, 0, sizeof(aucAmountBuf));
	memset( aucDispYUAN, 0, sizeof(aucDispYUAN));
	//UTIL_Form_Montant(&aucAmountBuf[5],G_NORMALTRANS_ulAmount,2);
	//memcpy(aucAmountBuf,"���룺",6);

	while(1)
	{
		memset(aucPIN,0,sizeof(aucPIN));
        //Os__clr_display(255);
		MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
	util_Printf("DataSave0.ConstantParamData.Pinpadflag = %02x\n",DataSave0.ConstantParamData.Pinpadflag);

	if(DataSave0.ConstantParamData.Pinpadflag ==1)
	{
		//if((!ReaderSupport)||(ReaderType!=READER_SAND))
		{
			Os__light_on_pp();
			Os__clr_display_pp(255);
		}
		MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
        //Os__GB2312_display(1, 0, aucAmountBuf);
		//Os__GB2312_display_pp(0, 0, aucAmountBuf);
		//if((!ReaderSupport)||(ReaderType!=READER_SAND))
			Os__GB2312_display_pp(0, 0, (uchar *) "��������������");
        //Os__GB2312_display(2, 0, (uchar *) "��ֿ�����������");
        //Os__GB2312_display(3, 0, (uchar * )"�������밴ȷ�ϼ�");
	}
	else
	{
        //Os__GB2312_display(0, 0, aucAmountBuf);
        //Os__GB2312_display(1, 0, (uchar *) "��������������:");
        //Os__GB2312_display(2, 0, (uchar *) "��ֿ�����������");
        //Os__GB2312_display(3, 0, (uchar * )"�������밴ȷ�ϼ�");
	}

	memset(aucTemPin,0,sizeof(aucTemPin));
	memset(aucTemCardPan,0,sizeof(aucTemCardPan));
	ucTemPinLen =G_EXTRATRANS_ucInputPINLen;
	memcpy(aucTemPin,G_EXTRATRANS_aucPINData,8);
	memcpy(aucTemCardPan,G_NORMALTRANS_aucCardPan,6);
	util_Printf("\n===============TEMP==================\n");
	for(uiI=0;uiI<8;uiI++)
		util_Printf("%02x ",G_EXTRATRANS_aucPINData[uiI]);
	util_Printf("\nCandPan");
	for(uiI=0;uiI<6;uiI++)
		util_Printf("%02x ",G_NORMALTRANS_aucCardPan[uiI]);
	memcpy(G_NORMALTRANS_aucCardPan,G_NORMALTRANS_aucCardPan_UnAssign,6);
	util_Printf("\nG_EXTRATRANS_ucInputPINLen%d",G_EXTRATRANS_ucInputPINLen);
	if(DataSave0.ConstantParamData.Pinpadflag ==1)
	{

		ucResult = EMV_UTIL_Input_PP();

		util_Printf("ucResult1 = %02x\n", ucResult);
		UTIL_PINPADDispLOGO();
		util_Printf("ucResult2 = %02x\n", ucResult);
	}
	else
	{
		ucResult = EMV_UTIL_Input();
	}
	G_EXTRATRANS_ucInputPINLen_UnAssign =G_EXTRATRANS_ucInputPINLen;
	memcpy(G_EXTRATRANS_aucPINData_UnAssign,G_EXTRATRANS_aucPINData,8);
	G_EXTRATRANS_ucInputPINLen =ucTemPinLen;
	memcpy(G_EXTRATRANS_aucPINData,aucTemPin,8);
	memcpy(G_NORMALTRANS_aucCardPan,aucTemCardPan,6);
	util_Printf("\n===============TEMP-end==================\n");
	for(uiI=0;uiI<8;uiI++)
		util_Printf("%02x ",G_EXTRATRANS_aucPINData[uiI]);
	util_Printf("\nCandPan2");
	for(uiI=0;uiI<6;uiI++)
		util_Printf("%02x ",G_NORMALTRANS_aucCardPan[uiI]);
  util_Printf("\nG_EXTRATRANS_ucInputPINLen==%d",G_EXTRATRANS_ucInputPINLen);
  util_Printf("\n===============UnAssign==================\n");
	for(uiI=0;uiI<8;uiI++)
		util_Printf("%02x ",G_EXTRATRANS_aucPINData_UnAssign[uiI]);
	util_Printf("\nCandPan2");
	for(uiI=0;uiI<6;uiI++)
		util_Printf("%02x ",G_NORMALTRANS_aucCardPan_UnAssign[uiI]);
  util_Printf("\nG_EXTRATRANS_ucInputPINLen%d",G_EXTRATRANS_ucInputPINLen_UnAssign);

	UTIL_PINPADDispLOGO();
	util_Printf("ucResult3 = %02x\n", ucResult);
	if(ucResult == ERR_CANCEL||ucResult == SUCCESS||ucResult == ERR_END)
		break;
	}
	//memcpy(pucEnPIN ,  G_EXTRATRANS_aucPINData, G_EXTRATRANS_ucInputPINLen);
	//*pucPinLen = G_EXTRATRANS_ucInputPINLen;
	return(ucResult);
}

//#define EMV_UTIL_Input_PP_test
unsigned char EMV_UTIL_Input_PP(void)
{
	unsigned char aucInitPAN[17];
	unsigned char ucResult;
	unsigned char aucPIN[13];
	unsigned char aucCardPan[9];
	unsigned char aucPINBuf[16];

	unsigned char ucArrayIndex;

	while(1)
	{
		memset(aucPIN,0,sizeof(aucPIN));
		memset(aucPINBuf,0,sizeof(aucPINBuf));
		memset(aucCardPan,0,sizeof(aucCardPan));
		memset(aucInitPAN,0,sizeof(aucInitPAN));
		ucResult = UTIL_Input_pp(1,false,4,12,'P',aucPIN);
		if( ucResult  == KEY_ENTER )
		{
			G_EXTRATRANS_ucInputPINLen = strlen((char *)aucPIN);
			ucResult = SUCCESS;
			if( (G_EXTRATRANS_ucInputPINLen>0)
		              &&(G_EXTRATRANS_ucInputPINLen<4))
			{
	          		continue;
			}
			if(G_EXTRATRANS_ucInputPINLen != 0)
			{
				memset(aucPINBuf,'F',sizeof(aucPINBuf));
				aucPINBuf[0] = '0';
				aucPINBuf[1] = G_EXTRATRANS_ucInputPINLen+'0';

				memcpy(&aucPINBuf[2],aucPIN,G_EXTRATRANS_ucInputPINLen);
#ifdef EMV_UTIL_Input_PP_test
				util_Printf("\nInputPin=%s\n",aucPINBuf);
#endif
				asc_hex(aucPIN,8,aucPINBuf,16);
				util_Printf("\n����PIN->���ܽ�������:%02x\n",G_NORMALTRANS_ucTransType);
				if ( G_NORMALTRANS_ucTransType !=TRANS_MOBILEAUTH
				   &&G_NORMALTRANS_ucTransType !=TRANS_VOIDMOBILEAUTH)
				{
					memset(aucCardPan,0,sizeof(aucCardPan));
					memcpy(&aucCardPan[2],G_NORMALTRANS_aucCardPan,6);
					#ifdef EMV_UTIL_Input_PP_test
					bcd_asc(aucInitPAN,aucCardPan,16);
					aucInitPAN[16] = 0;
					util_Printf("InputPan=%s\n",aucInitPAN);
					#endif
					gz_xor(aucCardPan,aucPIN,8);
					#ifdef EMV_UTIL_Input_PP_test
					hex_asc(aucInitPAN,aucPIN,16);
					aucInitPAN[16] = 0;
					util_Printf("XorPinData=%s\n",aucInitPAN);
					util_Printf("\nCandPan");
					for(uiI=0;uiI<6;uiI++)
					util_Printf("%02x ",G_NORMALTRANS_aucCardPan[uiI]);
					#endif
				}
			/******************��ʱʹ���������ܷ�ʽfante**************/
				ucArrayIndex = KEYARRAY_GOLDENCARDSH;

				util_Printf("ucArrayIndex = %02x \n ",ucArrayIndex);
				if(DataSave0.ConstantParamData.ENCRYPTTYPEParam==ENCRYPTTYPE_SINGLE)
				{
					util_Printf("-----ENCRYPTTYPE_SINGLE----\n");
					ucResult = PINPAD_47_Encrypt8ByteSingleKey(ucArrayIndex,
							KEYINDEX_GOLDENCARDSH_PINKEY,aucPIN,G_EXTRATRANS_aucPINData);
				}else
				if(DataSave0.ConstantParamData.ENCRYPTTYPEParam==ENCRYPTTYPE_DOUBLE)
				{
					util_Printf("-----ENCRYPTTYPE_DOUBLE----\n");
					ucResult = PINPAD_47_Encrypt8ByteSingleKey(ucArrayIndex,
							KEYINDEX_GOLDENCARDSH_PINKEY,
							aucPIN,aucPIN);
					ucResult = PINPAD_47_Crypt8ByteSingleKey(ucArrayIndex,
							KEYINDEX_GOLDENCARDSH_PINKEY2,
							aucPIN,aucPIN);
					ucResult = PINPAD_47_Encrypt8ByteSingleKey(ucArrayIndex,
							KEYINDEX_GOLDENCARDSH_PINKEY,
							aucPIN,G_EXTRATRANS_aucPINData);
				}
#ifdef EMV_UTIL_Input_PP_test
				{
					int i;
					util_Printf("G_EXTRATRANS_aucPINData = ");
					for(i=0;i<TRANS_PINDATALEN;i++)
						util_Printf("%02x ",G_EXTRATRANS_aucPINData[i]);
					util_Printf("\n");
				}
#endif
				ucResult = SUCCESS;
				break;
			}else
			{
				if(G_NORMALTRANS_euCardSpecies==CARDSPECIES_EMV)
					ucResult = EMVERROR_BYPASS;
				else
					ucResult =SUCCESS;
				break;
			}
		}
		else
		{
			util_Printf("break\n");
			break;
		}
	}
	return ucResult;
}


unsigned char EMV_UTIL_Input(void)
{
	unsigned char aucInitPAN[17];
	unsigned char ucResult;
	unsigned char aucPIN[13];
	unsigned char aucCardPan[9];
	unsigned char aucPINBuf[17];
	unsigned char ucArrayIndex;


	util_Printf("\n\n\n EMV_UTIL_Input ............\n ");

	memset(aucPIN,0,sizeof(aucPIN));
	memset(aucPINBuf,0,sizeof(aucPINBuf));
	memset(aucCardPan,0,sizeof(aucCardPan));
	memset(aucInitPAN,0,sizeof(aucInitPAN));

	while(1)
	{
#ifndef GUI_PROJECT
		ucResult = UTIL_Input(2,true,4,12,'P',aucPIN,NULL);
#else
		util_Printf("\n ProUiFace.UiToPro.uiPwLen =%d\n",ProUiFace.UiToPro.uiPwLen);
		util_Printf("\n ProUiFace.UiToPro.aucPasswd =%s\n",ProUiFace.UiToPro.aucPasswd);
		if(ProUiFace.UiToPro.uiPwLen > 12) ucResult = ERR_END;
		else{
			ucResult  = KEY_ENTER;
#ifndef GUI_TTS_DATA
			memcpy(aucPIN, "123456",6);
#else
			memcpy(aucPIN, ProUiFace.UiToPro.aucPasswd,ProUiFace.UiToPro.uiPwLen);
#endif			
		util_Printf("\n EMV_UTIL_Input.aucPasswd =%s\n",aucPIN);
		}
#endif
		if(ucResult  == KEY_ENTER )
		{
			G_EXTRATRANS_ucInputPINLen = strlen((char *)aucPIN);
			ucResult = SUCCESS;
			if( (G_EXTRATRANS_ucInputPINLen>0)
		              &&(G_EXTRATRANS_ucInputPINLen<4)
		         )
			{
	          		continue;
			}
			if(G_EXTRATRANS_ucInputPINLen != 0)
			{
				memset(aucPINBuf,0,sizeof(aucPINBuf));
				memset(aucPINBuf,'F',16);
				aucPINBuf[0] = '0';
				aucPINBuf[1] = G_EXTRATRANS_ucInputPINLen+'0';

				memcpy(&aucPINBuf[2],aucPIN,G_EXTRATRANS_ucInputPINLen);
#ifdef EMV_UTIL_Input_PP_test
				util_Printf("\nInputPin=%s\n",aucPINBuf);
#endif
				util_Printf("\n\n\n 20120616 EMV_UTIL_Input .....aucPINBuf.......[%s]\n ",aucPINBuf);
				asc_hex(aucPIN,8,aucPINBuf,16);
				util_Printf("\n����PIN->���ܽ�������:%02x\n",G_NORMALTRANS_ucTransType);
				if ( G_NORMALTRANS_ucTransType !=TRANS_MOBILEAUTH
				   &&G_NORMALTRANS_ucTransType !=TRANS_VOIDMOBILEAUTH)
				{
					memset(aucCardPan,0,sizeof(aucCardPan));
					memcpy(&aucCardPan[2],G_NORMALTRANS_aucCardPan,6);

					#ifdef EMV_UTIL_Input_PP_test
					bcd_asc(aucInitPAN,aucCardPan,16);
					aucInitPAN[16] = 0;
					util_Printf("InputPan=%s\n",aucInitPAN);
					#endif

					gz_xor(aucCardPan,aucPIN,8);

					#ifdef EMV_UTIL_Input_PP_test
					hex_asc(aucInitPAN,aucPIN,16);
					aucInitPAN[16] = 0;
					util_Printf("XorPinData=%s\n",aucInitPAN);
					#endif
				}

				/******************��ʱʹ���������ܷ�ʽfante**************/

				ucArrayIndex = KEYARRAY_GOLDENCARDSH;

				util_Printf("ucArrayIndex = %02x \n ",ucArrayIndex);
				if(DataSave0.ConstantParamData.ENCRYPTTYPEParam==ENCRYPTTYPE_SINGLE)
				{
					util_Printf("-----ENCRYPTTYPE_SINGLE----\n");
					ucResult = PINPAD_47_Encrypt8ByteSingleKey(ucArrayIndex,
							KEYINDEX_GOLDENCARDSH_PINKEY,aucPIN,G_EXTRATRANS_aucPINData);
				}else
				if(DataSave0.ConstantParamData.ENCRYPTTYPEParam==ENCRYPTTYPE_DOUBLE)
				{
					util_Printf("-----ENCRYPTTYPE_DOUBLE----\n");
					ucResult = PINPAD_47_Encrypt8ByteSingleKey(ucArrayIndex,
							KEYINDEX_GOLDENCARDSH_PINKEY,
							aucPIN,aucPIN);
					ucResult = PINPAD_47_Crypt8ByteSingleKey(ucArrayIndex,
							KEYINDEX_GOLDENCARDSH_PINKEY2,
							aucPIN,aucPIN);
					ucResult = PINPAD_47_Encrypt8ByteSingleKey(ucArrayIndex,
							KEYINDEX_GOLDENCARDSH_PINKEY,
							aucPIN,G_EXTRATRANS_aucPINData);
				}
#ifdef EMV_UTIL_Input_PP_test
				{
					int i;
					util_Printf("G_EXTRATRANS_aucPINData = ");
					for(i=0;i<TRANS_PINDATALEN;i++)
						util_Printf("%02x ",G_EXTRATRANS_aucPINData[i]);
					util_Printf("\n");
				}
#endif
				break;
			}else
			{
				if(G_NORMALTRANS_euCardSpecies==CARDSPECIES_EMV)
					ucResult = EMVERROR_BYPASS;
				else
					ucResult =SUCCESS;
				break;
			}
		}
		else
		{
			break;
		}
	}
	return ucResult;
}

void gz_xor(unsigned char *a, unsigned char *b, char lg)
{
	while (lg--)
		*(b++) ^= *(a++);
}

unsigned char  UTIL_GetMenu_Value(unsigned char ucTransType,unsigned short uiIndex,
		unsigned char *pucFlag,unsigned char (*pfnProc)(void),SELMENU *Menu)
{
	if(Menu->ucMenuCnt>MAXMENUDISPLINE)
		return(ERR_NOTPROC);
	if( (pucFlag !=NULL)&&(*pucFlag == 0) )
		return(ERR_NOTPROC);
	Menu->DispMenu[Menu->ucMenuCnt].ucTransType=ucTransType;
	Menu->DispMenu[Menu->ucMenuCnt].uiDispIndex=uiIndex;
	Menu->DispMenu[Menu->ucMenuCnt].pfnProc=pfnProc;
	Menu->ucMenuCnt++;
}

unsigned char OSUTIL_InputIPV4Addr(
                unsigned int uiTimeout,
                unsigned char ucFont,
                unsigned char ucRow,
                unsigned char *pucIPV4Addr)
{
    unsigned char ucResult;
    unsigned char aucDisp[20];
    unsigned char aucIPV4Addr[4];
    unsigned char ucIPV4AddrLen;
    unsigned char aucBuf[5];
    unsigned char ucBufLen;
    unsigned char ucCursorRow;
    unsigned char ucCursorCol;
    unsigned char ucRedraw;
    unsigned char ucClear=0;

    DRV_OUT *pdKey;   //DRVOUT *pdKey;       cj
    unsigned char ucKey;
    unsigned char ucEnd;

    ucResult = SUCCESS;       //OSERR_SUCCESS;    cj
    memset(aucDisp,0,sizeof(aucDisp));
    memset(aucIPV4Addr,0,sizeof(aucIPV4Addr));
    ucIPV4AddrLen = 0;
    memset(aucBuf,0,sizeof(aucBuf));
    ucBufLen = 0;
//    util_Printf("\n=====aucIPV4Addr[0]======\n",&aucIPV4Addr);
    if( uiTimeout )
    {
        Os__timer_start(&uiTimeout );  //OSTIMER_Start(&uiTimeout);  cj
    }else
    {
        uiTimeout = 1;
    }
    ucRedraw = FALSE;
    ucEnd = FALSE;
    if( pucIPV4Addr )
    {
        memcpy(aucIPV4Addr,pucIPV4Addr,4);
        ucRedraw = TRUE;

    }
 //   util_Printf("----1----aucIPV4Addr=%s\n",aucIPV4Addr);
    ucResult = SUCCESS;     //OSERR_SUCCESS;   cj
    do
    {
        if( ucRedraw == TRUE)
        {
            memset(&aucDisp[0],0,sizeof(aucDisp));
            sprintf((char *)aucDisp,"%3d.%3d.%3d.%3d",
                aucIPV4Addr[0],aucIPV4Addr[1],aucIPV4Addr[2],aucIPV4Addr[3]);
            //Os__display(1,0,&aucDisp[0]);    //OSMMI_DisplayASCII(ucFont,ucRow,0,&aucDisp[0]);  //// cj
            if( ucIPV4AddrLen < 4 )
            {
                ucCursorCol = (ucIPV4AddrLen*4+2)*6;
            }else
            {
                ucCursorCol = (3*4+2)*6;
            }
            ucCursorRow = ucRow*8+7;
     // util_Printf("=====chen00======\n");
         //   HALLCD_Display8DotInRow(0x00,ucCursorRow,ucCursorCol,0xFC);     //cj
     // util_Printf("=====chen01======\n");
            ucRedraw = FALSE;
        }
        pdKey = Os__get_key();       // OSMMI_GetKey();      cj

        do{
        }while(  (pdKey->gen_status == DRV_RUNNING)
               &&(uiTimeout)
               );

        if (uiTimeout == 0)
        {
            Os__abort_drv(drv_mmi);  //OSDRV_Abort(HALDRVID_MMI);
            ucResult = ERR_CANCEL;             //OSERR_CANCEL;   cj
            break;
        }else
        {
            ucKey = pdKey->xxdata[1];
        }
   //     util_Printf("\n ucKey %02x",ucKey);
        switch(ucKey)
        {
        case KEY_CLEAR:
            if( ucIPV4AddrLen )
            {
                ucIPV4AddrLen = 0;
                memset(aucIPV4Addr,0,sizeof(aucIPV4Addr));
                memset(aucBuf,0,sizeof(aucBuf));
                ucBufLen = 0;
                ucRedraw = TRUE;
            }else
            {
                ucEnd = TRUE;
                ucResult = ERR_CANCEL;   //OSERR_CANCEL;   cj
            }
            break;
        case KEY_00_PT:
            if( ucIPV4AddrLen < 4)
            {
                if( !OSUTIL_CheckIPAddrField(aucBuf,ucBufLen,&aucIPV4Addr[ucIPV4AddrLen]) )
                {
                    ucIPV4AddrLen ++;
                    if( ucIPV4AddrLen >= 4 )
                    {
                        ucIPV4AddrLen = 3;
                    }
                    ucBufLen = 0;
                    memset(aucBuf,0,sizeof(aucBuf));
                    ucRedraw = TRUE;
                }
            }else
            {
                Os__beep();
            }
            break;
        case KEY_BCKSP:
        	if (!ucClear)
            {
                ucIPV4AddrLen=3;
                ucClear=1;
            }
            if( ucBufLen )
            {
                ucBufLen --;
                aucBuf[ucBufLen] = 0;
                if( !OSUTIL_CheckIPAddrField(aucBuf,ucBufLen,&aucIPV4Addr[ucIPV4AddrLen]) )
                {
                    ucRedraw = TRUE;
                }
            }else
            {
                if( ucIPV4AddrLen )
                {
                    aucIPV4Addr[ucIPV4AddrLen] = 0;
                    ucIPV4AddrLen --;
                    sprintf((char *)aucBuf,"%d",aucIPV4Addr[ucIPV4AddrLen]);
                    ucBufLen = strlen((char *)aucBuf);
                    ucRedraw = TRUE;
                }else
                {
                    aucIPV4Addr[ucIPV4AddrLen] = 0;
                    Os__beep();
                }
            }
            break;
        case KEY_ENTER:
            ucEnd = TRUE;
            ucResult=KEY_ENTER;
            break;
        default :
            if(  (ucKey >= '0')
               &&(ucKey <= '9')
              )
            {
                aucBuf[ucBufLen++] = ucKey;
                if( ucIPV4AddrLen < 4 )
                {
                    if( !OSUTIL_CheckIPAddrField(aucBuf,ucBufLen,&aucIPV4Addr[ucIPV4AddrLen]) )
                    {
                        ucRedraw = TRUE;
                    }
                }else
                {
                    OSMMI_Beep();
                }
                ucRedraw = TRUE;
                if( ucBufLen >= 3 )
                {
                    memset(aucBuf,0,sizeof(aucBuf));
                    ucBufLen = 0;
                    if( ucIPV4AddrLen < 3 )
                    {
                        ucIPV4AddrLen ++;
                    }
                }
            }
            break;
        }
        if( ucEnd == TRUE)
            break;
    }while(1);

//util_Printf("======util====== ---0--- pucIPV4Addr=%s\n",pucIPV4Addr);

    if( ucResult==KEY_ENTER )
    {
        if( pucIPV4Addr )
        {

            memcpy(pucIPV4Addr,aucIPV4Addr,4);
//util_Printf("----2----aucIPV4Addr=\n",&aucIPV4Addr);
    //        util_Printf("ucResult1 %02x",ucResult);
        }
    }
//util_Printf("======util====== ---1--- pucIPV4Addr=%s\n",pucIPV4Addr);
    return(ucResult);
}

unsigned char OSUTIL_CheckIPAddrField(
                unsigned char *pucIP,
                unsigned char ucIPLen,
                unsigned char *pucIPField)
{
    unsigned short uiIPField;

    if(ucIPLen > 4 )
    {
        return(ERR_CANCEL);    //return(OSERR_BADIP);  cj
    }
    if( strlen((char *)pucIP) > ucIPLen )
    {
        return(ERR_CANCEL);    //return(OSERR_BADIP);  cj
    }
    uiIPField = CONV_StrLong(pucIP);
    if( uiIPField > 255 )
    {
        return(ERR_CANCEL);    //return(OSERR_BADIP);  cj
    }
    *pucIPField = uiIPField;

    return(SUCCESS);    //return(OSERR_SUCCESS);   cj
}

/*��������Կ*/
#define KEY_TEST
unsigned char UTIL_Single_StoreNewKey(void)
{
	unsigned char ucResult;
	unsigned char ucArrayIndex;
	unsigned char aucHexData[25];
	unsigned char aucDataBuf[9];
	unsigned char aucBuf[9];
	unsigned char ucKeyIndex;
	short iLen,usI;



	ucResult = ISO8583_GetBitValue(62,aucHexData,&iLen,sizeof(aucHexData));
	if( ucResult == SUCCESS)
	{
		if( iLen != 24 )
		{
			return(ERR_ISO8583_INVALIDLEN);
		}
	}
	util_Printf("\n-0001.ucResult:%02x.",ucResult);

	util_Printf("\n------------single Encrypt  00-------------------\n");
	for(usI=0;usI<24;usI++)
	{
		if (!((usI+1)%8))
			util_Printf("\n");
		util_Printf("%02x",aucHexData[usI]);
	}
	util_Printf("\n");
	util_Printf("Key Len = %02x\n",iLen);

	memset(aucDataBuf,0,sizeof(aucDataBuf));
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucDataBuf,aucHexData,8);
	memcpy(aucBuf,&aucHexData[12],8);

	//---------------------------------------------
	util_Printf("ConstantParamData.ucUseKeyIndex=%02x\n",DataSave0.ConstantParamData.ucUseKeyIndex);
	ucKeyIndex = DataSave0.ConstantParamData.ucUseKeyIndex  ;

	ucArrayIndex = KEYARRAY_GOLDENCARDSH;

	util_Printf("ucArrayIndex = %02x \n ",ucArrayIndex);
	util_Printf("ucKeyIndex = %02x \n ",ucKeyIndex);

	ucResult = PINPAD_42_LoadSingleKeyUseSingleKey(
		ucArrayIndex,ucKeyIndex,
		KEYINDEX_GOLDENCARDSH_PINKEY,aucDataBuf);

	util_Printf("\n-0002.ucResult:%02x.\n",ucResult);
	if(ucResult == SUCCESS)
	{
 		ucResult = PINPAD_42_LoadSingleKeyUseSingleKey(
				ucArrayIndex,ucKeyIndex,
				KEYINDEX_GOLDENCARDSH_MACKEY,aucBuf);
	}
	util_Printf("\n-0003.ucResult:%02x.\n",ucResult);	
	if(ucResult == SUCCESS)
	{
		memset(aucDataBuf,0,sizeof(aucDataBuf));
		ucResult = PINPAD_47_Encrypt8ByteSingleKey(
					ucArrayIndex,KEYINDEX_GOLDENCARDSH_PINKEY,
						aucDataBuf,aucDataBuf);
	}
	util_Printf("\n-0004.ucResult:%02x.\n",ucResult);
#ifdef KEY_TEST
util_Printf("------------single Encrypt  01-------------------\n");


		for(usI=0;usI<4;usI++)
			util_Printf("%02x",aucDataBuf[usI]);
		util_Printf("\n");
		for(usI=8;usI<12;usI++)
			util_Printf("%02x",aucHexData[usI]);
		util_Printf("\n");
#endif
	if(ucResult == SUCCESS)
	{
		memcpy(aucBuf,&aucHexData[8],4);
		if(memcmp(aucDataBuf,aucBuf,4) != 0)
			ucResult = ERR_CHECKSECRET;
	}
		util_Printf("\n-0005.ucResult:%02x.\n",ucResult);
	if(ucResult == SUCCESS)
	{
		memset(aucDataBuf,0,sizeof(aucDataBuf));
		ucResult = PINPAD_47_Encrypt8ByteSingleKey(
					ucArrayIndex,KEYINDEX_GOLDENCARDSH_MACKEY,
						aucDataBuf,aucDataBuf);
	}
		util_Printf("\n-0006.ucResult:%02x.\n",ucResult);
#ifdef KEY_TEST
util_Printf("------------single Encrypt  02-------------------\n");
		for(usI=0;usI<4;usI++)
			util_Printf("%02x",aucDataBuf[usI]);
		util_Printf("\n");
		for(usI=20;usI<24;usI++)
			util_Printf("%02x",aucHexData[usI]);
		util_Printf("\n");
#endif

	if(ucResult == SUCCESS)
	{
		memcpy(aucBuf,&aucHexData[20],4);
		if(memcmp(aucDataBuf,aucBuf,4) != 0)
			ucResult = ERR_CHECKSECRET;
	}

util_Printf("------------UTIL_Single_StoreNewKey ucResult:[%02x]------------------\n",ucResult);

	return(ucResult);
}


unsigned char UTIL_Double_StoreNewKey(void)
{

	unsigned char ucResult;
	unsigned char ucArrayIndex;
	unsigned char ucKeyIndex;
	unsigned char aucHexData[41];
	unsigned char aucDataBuf[9];
	unsigned char aucDataBuf2[9];
	unsigned char aucDataPINCHACK[9];
	unsigned char aucDataMACCHACK[9];
	unsigned char aucBuf[9];
	short iLen,usI;
	int i;



	ucResult = ISO8583_GetBitValue(62,aucHexData,&iLen,sizeof(aucHexData));
	if( ucResult == SUCCESS)
	{
		if( iLen != 40 )
		{
			return(ERR_ISO8583_INVALIDLEN);
		}
	}

/*
����˫������Կ�㷨��ǰ20���ֽ�ΪPIN�Ĺ�����Կ�����ģ�
��20���ֽ�ΪMAC�Ĺ�����Կ�����ġ�
*/

	util_Printf("\n------------double Encrypt  00-------------------\n");
	for(usI=0;usI<40;usI++)
	{
		if(!((usI+1)%8))
			util_Printf("\n");
		util_Printf("%02x",aucHexData[usI]);
	}
	util_Printf("\n");
	util_Printf("Key Len = %02x\n",iLen);


	memset(aucDataBuf,0,sizeof(aucDataBuf));
	memset(aucDataBuf2,0,sizeof(aucDataBuf2));
	memset(aucDataPINCHACK,0,sizeof(aucDataPINCHACK));
	memset(aucBuf,0,sizeof(aucBuf));

	memcpy(aucDataBuf,aucHexData,8);
	memcpy(aucDataBuf2,&aucHexData[8],8);
	memcpy(aucBuf,&aucHexData[20],8);

	util_Printf("ConstantParamData.ucUseKeyIndex=%02x\n",DataSave0.ConstantParamData.ucUseKeyIndex);
	ucKeyIndex = DataSave0.ConstantParamData.ucUseKeyIndex  ;

	ucArrayIndex = KEYARRAY_GOLDENCARDSH;

#ifdef TEST
{

	util_Printf("ucArrayIndex = %02x \n",ucArrayIndex);

	util_Printf("PINKEY =  ");
	for(i=0;i<8;i++)
		util_Printf("%02x",aucDataBuf[i]);
	util_Printf("\nPINKEY2 = ");
	for(i=0;i<8;i++)
		util_Printf("%02x",aucDataBuf2[i]);
	util_Printf("\nMACKEY =  ",aucBuf);
	for(i=0;i<8;i++)
		util_Printf("%02x",aucBuf[i]);
	util_Printf("\n");
}
#endif
	//------------------PIN 1--------------------------------
	ucResult = PINPAD_47_Crypt8ByteSingleKey(
				ucArrayIndex,ucKeyIndex,
					aucDataBuf,aucDataBuf);
	util_Printf("\nPINKEY ---1= ");
	for(i=0;i<8;i++)
		util_Printf("%02x",aucDataBuf[i]);
	ucResult = PINPAD_47_Encrypt8ByteSingleKey(
				ucArrayIndex,ucKeyIndex+3,
					aucDataBuf,aucDataBuf);
	util_Printf("\nPINKEY ---2= ");
	for(i=0;i<8;i++)
		util_Printf("%02x",aucDataBuf[i]);
	//------------------PIN 2--------------------------------
	ucResult = PINPAD_47_Crypt8ByteSingleKey(
				ucArrayIndex,ucKeyIndex,
					aucDataBuf2,aucDataBuf2);
	util_Printf("\nPINKEY2 --1= ");
	for(i=0;i<8;i++)
		util_Printf("%02x",aucDataBuf2[i]);
	ucResult = PINPAD_47_Encrypt8ByteSingleKey(
				ucArrayIndex,ucKeyIndex+3,
					aucDataBuf2,aucDataBuf2);
	util_Printf("\nPINKEY2 --2= ");
	for(i=0;i<8;i++)
		util_Printf("%02x",aucDataBuf2[i]);
	//------------------MAC--------------------------------
	ucResult = PINPAD_47_Crypt8ByteSingleKey(
				ucArrayIndex,ucKeyIndex,
					aucBuf,aucBuf);
	util_Printf("\nMACKEY ---1= ");
	for(i=0;i<9;i++)
		util_Printf("%02x ",aucBuf[i]);
	ucResult = PINPAD_47_Encrypt8ByteSingleKey(
				ucArrayIndex,ucKeyIndex+3,
					aucBuf,aucBuf);
	util_Printf("\nMACKEY ---2= ");
	for(i=0;i<9;i++)
		util_Printf("%02x ",aucBuf[i]);

	//--------------------------------------------------
			ucResult = PINPAD_42_LoadSingleKeyUseSingleKey(
				ucArrayIndex,ucKeyIndex,
				KEYINDEX_GOLDENCARDSH_PINKEY,aucDataBuf);
			ucResult = PINPAD_42_LoadSingleKeyUseSingleKey(
				ucArrayIndex,ucKeyIndex,
				KEYINDEX_GOLDENCARDSH_PINKEY2,aucDataBuf2);
			if(ucResult == SUCCESS)
			{
		 		ucResult = PINPAD_42_LoadSingleKeyUseSingleKey(
						ucArrayIndex,ucKeyIndex,
						KEYINDEX_GOLDENCARDSH_MACKEY,aucBuf);
			}

/*
"PIN������Կ"ǰ16���ֽ������ģ���4���ֽ���checkvalue��
ǰ16���ֽڽ�����ĺ󣬶�16����ֵ0��˫������Կ�㷨��
ȡ�����ǰ��λ��checkvalue ��ֵ�Ƚ�Ӧ����һ�µģ�
*/
	if(ucResult == SUCCESS)
	{
		memset(aucDataPINCHACK,0,sizeof(aucDataPINCHACK));
		ucResult = PINPAD_47_Encrypt8ByteSingleKey(
					ucArrayIndex,KEYINDEX_GOLDENCARDSH_PINKEY,
						aucDataPINCHACK,aucDataPINCHACK);
		ucResult = PINPAD_47_Crypt8ByteSingleKey(
					ucArrayIndex,KEYINDEX_GOLDENCARDSH_PINKEY2,
						aucDataPINCHACK,aucDataPINCHACK);
		ucResult = PINPAD_47_Encrypt8ByteSingleKey(
					ucArrayIndex,KEYINDEX_GOLDENCARDSH_PINKEY,
						aucDataPINCHACK,aucDataPINCHACK);

	}
#ifdef TEST
		util_Printf("\n------------Double Encrypt  01--------PIN-----------\n");
		util_Printf("PIN chack data just = ");
		for(usI=0;usI<4;usI++)
			util_Printf("%02x",aucDataPINCHACK[usI]);
		util_Printf("\nPIN chack data old  = ");
		for(usI=16;usI<20;usI++)
			util_Printf("%02x",aucHexData[usI]);
		util_Printf("\n");
#endif
	if(ucResult == SUCCESS)
	{
		util_Printf("----------PingKey -----ok ok ok!\n");
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,&aucHexData[16],4);
		if(memcmp(aucDataPINCHACK,aucBuf,4) != 0)
		{
			ucResult = ERR_CHECKSECRET;
		}
	}
/*
"MAC������Կ"ǰ8���ֽ������ģ���8���ֽ��Ƕ������㣬
��4���ֽ���checkvalue��ǰ8���ֽڽ�����ĺ�
��8����ֵ0����������Կ�㷨��
ȡ�����ǰ��λ��checkvalue ��ֵ�Ƚ�Ӧ����һ�µ�
*/
	if(ucResult == SUCCESS)
	{
		memset(aucDataMACCHACK,0,sizeof(aucDataMACCHACK));
		ucResult = PINPAD_47_Encrypt8ByteSingleKey(
					ucArrayIndex,KEYINDEX_GOLDENCARDSH_MACKEY,
						aucDataMACCHACK,aucDataMACCHACK);
	}
#ifdef TEST
		util_Printf("------------Double Encrypt  02--------MAC-----------\n");
		util_Printf("MAC chack data just = ");
		for(usI=0;usI<4;usI++)
			util_Printf("%02x",aucDataMACCHACK[usI]);
		util_Printf("\nMAC chack data old  = ");
		for(usI=36;usI<40;usI++)
			util_Printf("%02x",aucHexData[usI]);
		util_Printf("\n");
#endif
	if(ucResult == SUCCESS)
	{
		util_Printf("----------MAC Key -----ok ok ok!\n");
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,&aucHexData[36],4);
		if(memcmp(aucDataMACCHACK,aucBuf,4) != 0)
		{
			ucResult = ERR_CHECKSECRET;
		}
	}
	return(ucResult);
}

unsigned char UTIL_CalcGoldenCardSHMAC_Single(
				unsigned char *pucInData,
				unsigned short uiInLen,
				unsigned char *pucOutMAC)
{
	unsigned char ucResult;
	unsigned char aucHexData[9];
	unsigned char aucAscData[17];
	unsigned char *pucPtr;
	unsigned short uiI,uiJ;
	unsigned short uiLen;
	unsigned char  ucArrayIndex;

	pucPtr = pucInData;

#ifdef TEST_MAC
	util_Printf("��ʼ����:\n");
	for (ucI=0;ucI<uiInLen;ucI++)
		util_Printf("%02x ",pucPtr[ucI]);
	util_Printf("\n");
#endif

	ucArrayIndex = KEYARRAY_GOLDENCARDSH;

	memset(aucHexData,0,sizeof(aucHexData));

	for(uiI=0;uiI<uiInLen;uiI += 8)
	{
		uiLen = min(8,pucInData+uiInLen-pucPtr);
		for(uiJ=0;uiJ<uiLen;uiJ++)
		{
			aucHexData[uiJ] ^= *(pucPtr+uiJ);
		}
		pucPtr += 8;
	}

#ifdef TEST_MAC
	util_Printf("ÿ8λ��������:\n");
	for (ucI=0;ucI<16;ucI++)
		util_Printf("%02x ",aucHexData[ucI]);
	util_Printf("\n");
#endif

	hex_asc(aucAscData,aucHexData,16);
	aucAscData[16] = 0;

#ifdef TEST_MAC
	util_Printf("ת��16λ������:\n");
	for (ucI=0;ucI<16;ucI++)
		util_Printf(" %02x",aucAscData[ucI]);
	util_Printf("\n");
#endif

	memset(aucHexData,0,sizeof(aucHexData));
	ucResult = PINPAD_47_Encrypt8ByteSingleKey(
					ucArrayIndex,
					KEYINDEX_GOLDENCARDSH_MACKEY,
					aucAscData,
					aucHexData);

	if( ucResult != SUCCESS)
		return(ucResult);

#ifdef TEST_MAC
	util_Printf("DES���ܺ�����:\n");
	for (ucI=0;ucI<8;ucI++)
		util_Printf("%02x ",aucHexData[ucI]);
	util_Printf("\n");

	util_Printf("��8λ����:\n");
	for (ucI=0;ucI<8;ucI++)
		util_Printf("%02x ",aucAscData[ucI+8]);
	util_Printf("\n");
#endif

	for(uiI=0;uiI<8;uiI++)
	{
		aucHexData[uiI] ^= aucAscData[8+uiI];
	}

#ifdef TEST_MAC
	util_Printf("�ٴ���������:\n");
	for (ucI=0;ucI<8;ucI++)
		util_Printf("%02x ",aucHexData[ucI]);
	util_Printf("\n");
#endif

	ucResult = PINPAD_47_Encrypt8ByteSingleKey(
					ucArrayIndex,
					KEYINDEX_GOLDENCARDSH_MACKEY,
					aucHexData,
					aucHexData);

#ifdef TEST_MAC
	util_Printf("�ٴ�DES���ܺ�����:\n");
	for (ucI=0;ucI<8;ucI++)
		util_Printf("%02x ",aucHexData[ucI]);
	util_Printf("\n");
#endif

	if( ucResult != SUCCESS)
		return(ucResult);

	hex_asc(aucAscData,aucHexData,16);

#ifdef TEST_MAC
	util_Printf("���ת��16λ������:\n");
	for (ucI=0;ucI<16;ucI++)
		util_Printf(" %02x",aucAscData[ucI]);
	util_Printf("\n");
#endif

	memcpy(pucOutMAC,aucAscData,8);
	return(SUCCESS);
}

void UTIL_SetDefault_State(unsigned char Flag)
{
    unsigned char ucI,ucJ;
    unsigned int ulBatchNumber;
    unsigned char ucInitCasherNo=5;
    unsigned char aucCasherNo[CASH_CASHIERNOLEN+1],aucCasherPass[CASH_CASHIERPASSLEN+1];
    unsigned char initpass[CASH_MAXSUPERPASSWDLEN+1];
    unsigned char aucModemParam[10]={0,0,8,6,115,8,1,3,0,0};

    if (!Flag)
    {
        util_Printf("UTIL_SetDefault_State()\n");
        memset(&DataSave0,0,sizeof(DATASAVEPAGE0));
        memset(&DataSave1,0,sizeof(DATASAVEPAGE1));

        XDATA_ClearAllAPPFile();
        UTIL_DeletaAllEMVfileData();
        UTIL_ClearEMVInterfaceData();
    }
//	TERMSETTING_LoadDefaultSetting();

	//�汾��Ϣ
	memcpy(DataSave0.ConstantParamData.APP_Project,CURRENT_PRJ,sizeof(CURRENT_PRJ));
	memcpy(DataSave0.ConstantParamData.SOFTWARE_Version,SOFTWARE_VER,sizeof(SOFTWARE_VER));
	memcpy(DataSave0.ConstantParamData.HARDWARE_Version,HARDWARE_VER,sizeof(HARDWARE_VER));

	#if PS400
	//����
	Os__saved_set(&DataSave0.ConstantParamData.ucCommMode,3,1);
	Os__saved_set(&DataSave0.ConstantParamData.prnflag,1,1);
	Os__saved_set(&DataSave0.ConstantParamData.prnflagNew,0,1);
	Os__saved_set(&DataSave0.ConstantParamData.prnflagOld,0,1);
	Os__saved_set(&DataSave0.ConstantParamData.prnflagZD,0,1);
	Os__saved_set(&DataSave0.ConstantParamData.linenum,0x16,1);
	Os__saved_set(&DataSave0.ConstantParamData.font,0,1);
	Os__saved_set(&DataSave0.ConstantParamData.printtimes,2,1);
	Os__saved_set(&DataSave0.ConstantParamData.ucHeatPrintFormat,1,1);
	memcpy(DataSave0.ConstantParamData.aucPrintType,"\x01\x01",2);
	#else
	//���
	Os__saved_set(&DataSave0.ConstantParamData.prnflag,0,1);
	Os__saved_set(&DataSave0.ConstantParamData.prnflagNew,0,1);
	Os__saved_set(&DataSave0.ConstantParamData.prnflagOld,0,1);
	Os__saved_set(&DataSave0.ConstantParamData.prnflagZD,1,1);
	Os__saved_set(&DataSave0.ConstantParamData.linenum,0x01,1);
	Os__saved_set(&DataSave0.ConstantParamData.font,1,1);
	Os__saved_set(&DataSave0.ConstantParamData.printtimes,1,1);
	memcpy(DataSave0.ConstantParamData.aucPrintType,"\x02\x01",2);
	#endif

	DataSave0.ConstantParamData.uclujiaomc='0';
	

	Os__saved_set(&DataSave0.ConstantParamData.BackLightFlag,0,1);//����
	DataSave0.ConstantParamData.ucPrint =0x31;//�Ƿ��ӡ����
	DataSave0.ConstantParamData.ucInstallment='0';
	DataSave0.ConstantParamData.ucMobileFlag='0';
	DataSave0.ChangeParamData.ucTerminalLockFlag = 0;
	DataSave0.ConstantParamData.ucPrnErrTicket = '0';
	DataSave0.ConstantParamData.ucUnSaleSwipe='1';
	DataSave0.ConstantParamData.ucUnAuthFinSwipe = '1';
	DataSave0.ConstantParamData.ucLoadEMVParam='1';
	DataSave0.ConstantParamData.ucVoidInputPin = '1';
	DataSave0.ConstantParamData.ucPREAUTHFINISHInputPin = '1';
	DataSave0.ConstantParamData.ucDefaultTransParam=TRANS_PURCHASE;
	DataSave0.ConstantParamData.ENCRYPTTYPEParam=ENCRYPTTYPE_SINGLE;
	DataSave0.ConstantParamData.ucENCFlag =0x01;//TMS
	DataSave0.ConstantParamData.ucMagPreauthFinish='1';
	DataSave0.ConstantParamData.ucLoadFlag = 1;
	DataSave0.ConstantParamData.uiReceiveTimeout = 30;
	DataSave0.ConstantParamData.ucDialRetry = 3 ;
	DataSave0.ConstantParamData.ucSelfLogoff = '0';
	DataSave0.ConstantParamData.ucOFFLineSendType = '1';
	DataSave0.ConstantParamData.ucTransRetry=3;
	DataSave0.ConstantParamData.ucMagPreauthFinish = '1';
	DataSave0.ConstantParamData.uICCInfo = '0';
	DataSave0.ConstantParamData.ucUnManagePwd='1';
	DataSave0.ConstantParamData.ucUseLogonDisp = '0';
	DataSave0.ConstantParamData.ucDisAutoSettleFlag= '0';	/*�Զ������־*/
	DataSave0.ConstantParamData.ucDisAutoTiming= '0';		/*��ʱ�Զ������־*/
	DataSave0.ConstantParamData.ucEncryptDataFlag = '0';	//�����Ƿ����
	DataSave0.ConstantParamData.ucRePAuthority='0';
	DataSave0.ConstantParamData.ucForeignCard = 0x30;
	DataSave0.ConstantParamData.ucInputCardNo = 0x30; /*0x32-->0x30*/
	DataSave0.ConstantParamData.ucKEYMODE ='1';
	DataSave0.ConstantParamData.ucDisplay = '1';
	DataSave0.ConstantParamData.ucPrintCardNo=0x31;
	DataSave0.ConstantParamData.ucPBeforeSix=0x30;
	DataSave0.ConstantParamData.ucCardtype = 0x32;   //Ĭ��Ϊ��ӡ���⿨����ʱ���ؿ���
	DataSave0.ConstantParamData.ucDisAuth = 0x30;
	DataSave0.ConstantParamData.ucSwiptEvent = 0x31;
	DataSave0.ConstantParamData.ucCashFlag = 0x32;
	DataSave0.ConstantParamData.ucTMSFlag =0x30;
	DataSave0.ConstantParamData.ucUndoOff = 0x30;
	DataSave0.ConstantParamData.ucXorTipFee = 0;
	DataSave0.ConstantParamData.ucList = 0x30;
	DataSave0.ConstantParamData.ucCNOREN = '1';
	DataSave0.ConstantParamData.ucQuickUndo = '0';
	DataSave0.ConstantParamData.ucSelIP = 1;
	DataSave0.ConstantParamData.ucORDERFLAG = '0';
	DataSave0.ConstantParamData.ucSendOrder='0';
	DataSave0.ConstantParamData.ucMBFEFlag='0';
	DataSave0.ConstantParamData.ucLogistics='0';
	DataSave0.ConstantParamData.ucBatchErrTicket='0';
	DataSave0.ConstantParamData.ucCollectFlag='0';
	DataSave0.ConstantParamData.ucSendCollectFlag='0';
	DataSave0.ConstantParamData.ucEmptySettle ='0';
	DataSave0.ConstantParamData.ucInforFlag ='0';
	hex_asc(DataSave0.ConstantParamData.aucKEKRow,"\x0E",2);
	hex_asc(DataSave0.ConstantParamData.aucKEKLine,"\x24",2);
	DataSave0.ConstantParamData.uiDialWaitTime = asc_long((unsigned char *)"30",2);
	DataSave0.ConstantParamData.uiCommSpeed = asc_long((unsigned char *)"9600",4);

	DataSave0.ChangeParamData.ulUnTouchTransLimit = 5000000;//2009-9-7 20:14cbf
	DataSave0.ChangeParamData.bECSupport =true;//2009-9-7 20:16cbf
	ReaderSupport = true;
	ReaderType = READER_SAND;
	#if PS100
	DataSave0.ConstantParamData.ucKEYCOMM = PARAM_DOWNKEYCOMM_HDLC;
          #else
          DataSave0.ConstantParamData.ucKEYCOMM = PARAM_DOWNKEYCOMM_GPRS;
          #endif
#ifdef EMVTEST
	memcpy(DataSave0.ConstantParamData.aucGlobalTransEnable,"\xFF\xE0",2);//Ĭ��֧�����н���
#else
	memcpy(DataSave0.ConstantParamData.aucGlobalTransEnable,"\x00\x00",2);
#endif

	ulBatchNumber = 300;
	Os__saved_copy((unsigned char *)&ulBatchNumber,(unsigned char *)&TRANS_MAXNB,sizeof(unsigned int));
	Os__saved_set(&DataSave0.ChangeParamData.ucSuperLogonFlag,0,1);/*״̬��Ϊ��������*/

	//����5��Ĭ�Ϲ�Ա
	for(ucI=1;ucI<=ucInitCasherNo;ucI++)
	{
		ucJ = ucI -1 ;
		memcpy(aucCasherPass ,"0000" ,CASH_CASHIERPASSLEN);
		char_asc(aucCasherNo, CASH_CASHIERNOLEN,&ucI);
		memcpy(&DataSave0.Cashier_SysCashier_Data.aucCashierNo[ucJ],aucCasherNo,CASH_CASHIERNOLEN);
		memcpy(&DataSave0.Cashier_SysCashier_Data.aucCashierPass[ucJ],aucCasherPass,CASH_CASHIERPASSLEN);
	}

	//����Ĭ�����ܺź�����
	Os__saved_copy((unsigned char*)"00",DataSave0.Cashier_SysCashier_Data.aucSYSCashierNo,CASH_SYSCASHIERNOLEN);
	Os__saved_copy((unsigned char*)"123456",DataSave0.Cashier_SysCashier_Data.aucSYSCashierPass,CASH_SYSCASHIERPASSLEN);
	Os__saved_set(&DataSave0.Cashier_SysCashier_Data.ucSYSCashierExitFlag,0,1);

	//�绰����
	Os__saved_copy((unsigned char *)"4008200358.",//33034500
				DataSave0.ConstantParamData.aucHostTelNumber1,10);
	Os__saved_copy((unsigned char *)"66694500.",
				DataSave0.ConstantParamData.aucHostTelNumber2,9);
	Os__saved_copy((unsigned char *)"66674500.",
				DataSave0.ConstantParamData.aucHostTelNumber3,9);

	Os__saved_copy((unsigned char *)"123456.",
				DataSave0.ConstantParamData.aucHostTelNumber4,7);
	Os__saved_copy((unsigned char *)"\x04\x01",
				DataSave0.ConstantParamData.aucTpdu,2);


	Os__saved_copy((unsigned char *)"54485159.",
				DataSave0.ConstantParamData.aucHostTel1,9);
	Os__saved_copy((unsigned char *)"54485159.",
				DataSave0.ConstantParamData.aucHostTel2,9);
	Os__saved_copy((unsigned char *)"\x60\x03\x08\x00\x00",
				DataSave0.ConstantParamData.aucKEYTpdu,5);

	//����Ĭ��ϵͳ����Ա�ź�����
	memset(initpass,0,sizeof(initpass));
	memcpy( DataSave0.Cashier_SysCashier_Data.aucSuperNo, "99", CASH_MAXSUPERNO);
	memcpy(initpass,"\x39\x31\x38\x32\x37\x33\x37\x38",CASH_MAXSUPERPASSWDLEN);
	Os__saved_copy(initpass,(unsigned char *)DataSave0.Cashier_SysCashier_Data.aucSuperPassword,CASH_MAXSUPERPASSWDLEN);
	Os__saved_set(&DataSave0.Cashier_SysCashier_Data.ucSYSCashierExitFlag,1,1);
	Os__saved_set(&DataSave0.ChangeParamData.ucCashierLogonFlag,0,1);

	/*��һ��װ��ϵͳĬ��ÿ�춨ʱ����ʱ��Ϊ�賿0��*/
	memcpy(DataSave0.ChangeParamData.aucSettleCycle,"\x30\x30\x30\x30",4);
	memcpy(DataSave0.Cashier_SysCashier_Data.aucSuperPassWord,"838055",6);//"\x38\x33\x38\x30\x35\x35"

	/*=======GPRS IP========*/
	//��IP
	memcpy(&DataSave0.ConstantParamData.ulHostIPAddress1, (unsigned char *)"\xAC\x10\x0A\x47", 4);
	memcpy(&DataSave0.ConstantParamData.ulHostIPAddress2, (unsigned char *)"\xAC\x10\x0A\x48", 4);
	memcpy(&DataSave0.ConstantParamData.ulHostIPAddress3, (unsigned char *)"\xAC\x10\x0A\x49", 4);
	//����IP
	memcpy(&DataSave0.ConstantParamData.ulHostIPGPRS1, (unsigned char *)"\xAC\x10\x0A\x3D", 4);
	memcpy(&DataSave0.ConstantParamData.ulHostIPGPRS2, (unsigned char *)"\xAC\x10\x0A\x3E", 4);
	memcpy(&DataSave0.ConstantParamData.ulHostIPGPRS3, (unsigned char *)"\xAC\x10\x0A\x3F", 4);

	/*=======MIS�̻� IP========*/
	//��IP
	memcpy(&DataSave0.ConstantParamData.ulHostIPBack1,(unsigned char *)"\x90\x10\x23\x15", 4);
	memcpy(&DataSave0.ConstantParamData.ulHostIPBack2,(unsigned char *)"\x90\x10\x23\x16", 4);
	memcpy(&DataSave0.ConstantParamData.ulHostIPBack3,(unsigned char *)"\x90\x10\x23\x17", 4);

	//����IP
	memcpy(&DataSave0.ConstantParamData.ulHostIPMIS1,(unsigned char *)"\x90\x10\x22\x15", 4);
	memcpy(&DataSave0.ConstantParamData.ulHostIPMIS2,(unsigned char *)"\x90\x10\x22\x16", 4);
	memcpy(&DataSave0.ConstantParamData.ulHostIPMIS3,(unsigned char *)"\x90\x10\x22\x17", 4);

	memcpy(DataSave0.ConstantParamData.aucAPN,(unsigned char*)"MPOS-CUPSH.SH",13);

	DataSave0.ConstantParamData.uiHostPort=asc_long((unsigned char *)"006000",6);
	DataSave0.ConstantParamData.uiHostBackPort=asc_long((unsigned char *)"006000",6);
	DataSave0.ConstantParamData.uiHostKEYPort =asc_long((unsigned char *)"009080",6);
	memcpy(DataSave0.ConstantParamData.aucKEYAPN,(unsigned char*)"CMNET",5);
	memcpy(&DataSave0.ConstantParamData.ulHostKEYIP,(unsigned char *)"\x74\xE4\xEA\x23", 4);

   Os__saved_copy(aucModemParam,(uchar *)&DataSave0.ConstantParamData.ModemParam,10);

	XDATA_Write_Vaild_File(DataSaveConstant);
	XDATA_Write_Vaild_File(DataSaveChange);
	XDATA_Write_Vaild_File(DataSaveCashier);
	XDATA_Write_Vaild_File(DataSaveTransInfo);
}

unsigned char UTIL_CheckProjAndVer(void)
{
	if ( memcmp(CURRENT_PRJ,DataSave0.ConstantParamData.APP_Project,sizeof(CURRENT_PRJ))
		|| memcmp(SOFTWARE_VER,DataSave0.ConstantParamData.SOFTWARE_Version,sizeof(SOFTWARE_VER))
		 )
	{
		util_Printf("--------�������°汾-------\n");
		UTIL_SetDefault_State(true);
	}
	return(SUCCESS);
}

unsigned char UITL_Delete_Reversal_Flag(void)
{
	unsigned char ucResult;

	ucResult=SUCCESS;
	Os__saved_set((unsigned char *)(&DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid),
				0,sizeof(unsigned char));
	memset(&DataSave0.Trans_8583Data.ReversalISO8583Data,0x00,sizeof(REVERSALISO8583));

	ucResult = XDATA_Write_Vaild_File(DataSaveTrans8583);
	if (ucResult!=SUCCESS)
	{
		MSG_DisplayErrMsg(ucResult);
		ucResult = ERR_END;
	}
	if (!ucResult)
	{
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char*)"����ɹ�");
	}
	else
	{
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char*)"���ʧ��");
	}
	UTIL_GetKey(10);
	return SUCCESS;
}
#if 0
void UTIL_SetYunNaPaper(void)
{
	unsigned char ucPreKey;
	unsigned char ucPaperFormat;

	while(1)
	{
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char *)"--�״�ֽ������--");
        //Os__GB2312_display(1,0,(unsigned char *)"1.������");
        //Os__GB2312_display(2,0,(unsigned char *)"2.�ն���");
        //Os__GB2312_display(3,0,(unsigned char *)"3.��˫������");
		ucPreKey = UTIL_GetKey(15);
		switch(ucPreKey)
		{
			case '1':
				ucPaperFormat = PAPER_KM;
				break;
			case '2':
				ucPaperFormat = PAPER_PE;
				break;
			case '3':
				ucPaperFormat = PAPER_XSBN;
				break;
			default:
				break;
		}
		if (ucPreKey >='1' && ucPreKey <= '3')
			break;
	}
    //Os__clr_display(255);
    //Os__GB2312_display(0,0,(unsigned char *)"--�״�ֽ������--");
    //Os__GB2312_display(1,0,(unsigned char *)"����Ϊ��");
	switch(ucPaperFormat)
	{
		case PAPER_KM:
            //Os__GB2312_display(2,2,(unsigned char *)"������ʽ");
			break;
		case PAPER_PE:
            //Os__GB2312_display(2,2,(unsigned char *)"�ն���ʽ");
			break;
		case PAPER_XSBN:
            //Os__GB2312_display(2,1,(unsigned char *)"��˫���ɸ�ʽ");
			break;
		default:
            //Os__GB2312_display(2,0,(unsigned char *)"  δ����ֽ�Ÿ�ʽ");
			break;
	}
	Os__saved_set(&DataSave0.ConstantParamData.prnflagZD,0,1);
	Os__saved_set(&DataSave0.ConstantParamData.prnflag,0,1);
	Os__saved_set(&DataSave0.ConstantParamData.ucPRNPAPER,ucPaperFormat,1);
	Os__saved_set(&DataSave0.ConstantParamData.prnflagNew,0,1);
	Os__saved_set(&DataSave0.ConstantParamData.prnflagOld,0,1);
	Os__saved_set(&DataSave0.ConstantParamData.linenum,0x08,1);
	Os__saved_set(&DataSave0.ConstantParamData.font,0x01,1);
	Os__saved_set(&DataSave0.ConstantParamData.printtimes,1,1);
	XDATA_Write_Vaild_File(DataSaveConstant);
	util_Printf("����ֽ��:%02x\n",DataSave0.ConstantParamData.ucPRNPAPER);
	UTIL_WaitGetKey(15);
}
#endif
unsigned char UTIL_SetPrinter_Heat(void)
{
	unsigned char Key,ucPrnflag_Format;

    //Os__clr_display(255);
    //Os__GB2312_display(0,0,(unsigned char *)"--������ʽ����--");
    //Os__GB2312_display(1,0,(unsigned char *)"1:��׼��ʽ");

	while(1)
	{
		Key=UTIL_WaitGetKey(30);
		if(Key=='1')
		{
			ucPrnflag_Format=PRINTER_HEAT_FORMAT_1;
			break;
		}else
		if(Key==KEY_CLEAR||Key==99)
			return(ERR_CANCEL);
	}

    //Os__clr_display(255);
    //Os__GB2312_display(0,0,(unsigned char *)"--������ʽ����--");
    //Os__GB2312_display(1,0,(unsigned char *)"����Ϊ��");
	switch(ucPrnflag_Format)
	{
		case PRINTER_HEAT_FORMAT_1:
            //Os__GB2312_display(2,2,(unsigned char *)"  ��׼��ʽ");
			break;
		default:
            //Os__GB2312_display(2,1,(unsigned char *)"  δ���ø�ʽ");
			break;
	}

	Os__saved_set(&DataSave0.ConstantParamData.prnflag,1,1);
	Os__saved_set(&DataSave0.ConstantParamData.prnflagNew,0,1);
	Os__saved_set(&DataSave0.ConstantParamData.prnflagOld,0,1);
	Os__saved_set(&DataSave0.ConstantParamData.linenum,0x16,1);
	Os__saved_set(&DataSave0.ConstantParamData.prnflagZD,0,1);
	Os__saved_set(&DataSave0.ConstantParamData.font,0,1);
	DataSave0.ConstantParamData.ucHeatPrintFormat = ucPrnflag_Format;
	/*Add 08-8-27 for TMS*/
	if(ucPrnflag_Format ==0x01)
	{
		memcpy(DataSave0.ConstantParamData.aucPrintType,"\x01\x01",2);
	}
	else if(ucPrnflag_Format ==0x02)
	{
		memcpy(DataSave0.ConstantParamData.aucPrintType,"\x01\x02",2);
	}
	else if(ucPrnflag_Format ==0x03)
	{
		memcpy(DataSave0.ConstantParamData.aucPrintType,"\x01\x03",2);
	}
	else if(ucPrnflag_Format ==0x04)
	{
		memcpy(DataSave0.ConstantParamData.aucPrintType,"\x01\x04",2);
	}
	/*End*/
	XDATA_Write_Vaild_File(DataSaveConstant);
	UTIL_WaitGetKey(30);
	return(SUCCESS);
}

unsigned char UTILE_SetPage(unsigned char ucUCHAR)
{
  	unsigned char ucPage,ucPageBuf[2];

    //Os__clr_display(255);
    //Os__GB2312_display(0,0,(unsigned char *)"���ô�ӡҳ��");
    //Os__GB2312_display(1,0,(unsigned char *)"(1-9)");
#if 0
	if (ucUCHAR == true)
	{
        Os__saved_set(&DataSave0.ConstantParamData.printtimes,2,1);
        XDATA_Write_Vaild_File(DataSaveConstant);
	}
	else if (ucUCHAR == false)
	{
        Os__saved_set(&DataSave0.ConstantParamData.printtimes,1,1);
        XDATA_Write_Vaild_File(DataSaveConstant);
	}
#endif
	memset( ucPageBuf, 0, sizeof(ucPageBuf));
	ucPageBuf[0] = DataSave0.ConstantParamData.printtimes+0x30;
    //Os__GB2312_display(2,0,(unsigned char *)ucPageBuf);
	while(1)
	{
		memset( ucPageBuf, 0, sizeof(ucPageBuf));
		ucPage=UTIL_GetKey(15);
		if(ucPage >=0x31&&ucPage<=0x39)
		{
			ucPageBuf[0] = ucPage;
			DataSave0.ConstantParamData.printtimes = ucPageBuf[0]-0x30;
			XDATA_Write_Vaild_File(DataSaveConstant);
		}
		else if(ucPage==KEY_ENTER)
		{
			break;
		}
        //Os__GB2312_display(2,0,(unsigned char *)ucPageBuf);
        //Os__GB2312_display(3,0,(unsigned char *)"��ȷ�ϼ����");
	}
	return(SUCCESS);
}
unsigned char UTIL_SetPrinter(void)
{
    unsigned char ucPage;
    unsigned char ucResult;

    while(1)
    {
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(uchar *)"��ǰΪ:");
        //Os__GB2312_display(1,0,(uchar *)"1.����");
        //Os__GB2312_display(3,0,(uchar *)"��Ϊ:");

        if(DataSave0.ConstantParamData.prnflag)
        {
            //Os__GB2312_display(0,6,(unsigned char *)"����");
        }
        else
        {
            //Os__GB2312_display(0,6,(unsigned char *)"δ����");
        }

        ucPage = UTIL_GetKey(15);
        if (ucPage == '1')
        {
            ucResult = UTIL_SetPrinter_Heat();

            if (ucResult == SUCCESS)
            {
                UTILE_SetPage(true);
                return(SUCCESS);
            }
        }
        else if (ucPage == KEY_CLEAR)
        {
            return(SUCCESS);
        }
    }
    return(SUCCESS);
}
unsigned char UTIL_SetPinpad(void)
{
    unsigned char ch;
MIMA:   //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char *)"��ǰ����");
        //Os__GB2312_display(1,0,(unsigned char *)"1.�� 2.��");
        //Os__GB2312_display(2,0,(unsigned char *)"����Ϊ:");
        //if (DataSave0.ConstantParamData.Pinpadflag==1)
            //Os__GB2312_display(0,6,(unsigned char *)"��");
        //else
            //Os__GB2312_display(0,6,(unsigned char *)"��");
        ch=UTIL_GetKey(15);
        if(ch=='1')
        {
            //Os__GB2312_display(2,6,(unsigned char *)"��");
            ch=UTIL_GetKey(15);
            if(ch==KEY_ENTER)
            {
                Os__saved_set(&DataSave0.ConstantParamData.Pinpadflag,1,1);
                Os__saved_set(&Pinpad_flag,1,1);
                XDATA_Write_Vaild_File(DataSaveConstant);
                //Os__clr_display(255);
                //Os__GB2312_display(0,0,(unsigned char *)"���������");
                //Os__GB2312_display(1,0,(unsigned char *)"����Ϊ:   ��");
                //Os__GB2312_display(2,0,(unsigned char *)"����PINPAD");
							  //if((!ReaderSupport)||(ReaderType!=READER_SAND))
							  {
						                Os__clr_display_pp(255);
						                Os__GB2312_display_pp(0,0,( unsigned char *)     "��ѡ��");
						                Os__display_pp(0,6,(unsigned char *)"PINPAD");
						                Os__GB2312_display_pp(1,0,( unsigned char *) " �밴ȷ��");
							  }
//	         			if((ReaderSupport)&&(ReaderType==READER_SAND))
//		 								return SUCCESS;
                if(UTIL_GetKey_pp(20) != 99)
                {
                    //Os__clr_display(255);
                    //Os__GB2312_display(0,0,( unsigned char *) "PINPAD ������ȷ");
                    ch = UTIL_GetKey(10);
								DispPINPAD_AD();
                    return SUCCESS;
                }else
                {
                    //Os__clr_display(255);
                    //Os__GB2312_display(0,0,( unsigned char *) "PINPAD ���Դ���");
                    ch = UTIL_GetKey(15);
                    return SUCCESS;
                }
               }else   return ERR_CANCEL;// goto MIMA;
        }
        else if(ch=='2')
            {
                //Os__GB2312_display(2,6,(unsigned char *)"��");
                    ch=UTIL_GetKey(15);
                    if(ch==KEY_ENTER)
                {
                    Os__saved_set(&DataSave0.ConstantParamData.Pinpadflag,2,1);
                    Os__saved_set(&Pinpad_flag,1,1);
                    XDATA_Write_Vaild_File(DataSaveConstant);
                    //Os__clr_display(255);
                    //Os__GB2312_display(0,0,(unsigned char *)"���������");
                    //Os__GB2312_display(1,0,(unsigned char *)"����Ϊ:   ��");
                    ch=UTIL_GetKey(15);
										//if((!ReaderSupport)||(ReaderType!=READER_SAND))
												Os__clr_display_pp(255);
                    return SUCCESS;
                }else   return ERR_CANCEL;//goto MIMA;
                }else if(ch==KEY_CLEAR)  return ERR_CANCEL;
                    else  goto MIMA;
//util_Printf("CFG_ConstantParamPinpad of pinpadflag=%d\n",DataSave0.ConstantParamData.Pinpad_flag);
}

unsigned char UTIL_Backlight(void)//FANGBO MODIFY DATE 080623
{
	unsigned char ucKey;

    //Os__clr_display(255);
    //Os__GB2312_display(0,0,(unsigned char *)"�Ƿ�������");
    //Os__GB2312_display(1,0,(unsigned char *)"0����   1����");
    //Os__GB2312_display(3,0,(unsigned char *)"��ǰ:");
    //if(DataSave0.ConstantParamData.BackLightFlag == 0)
        //{ //Os__GB2312_display(3,3,(unsigned char *)"��");}
    //else if(DataSave0.ConstantParamData.BackLightFlag == 1)
        //{ //Os__GB2312_display(3,3,(unsigned char *)"��");}

	while(1)
	{
		ucKey = UTIL_GetKey(30);
		if(ucKey=='1'||ucKey=='0'||ucKey==KEY_CLEAR)
			break;
	}
	if(ucKey=='1')
		Os__saved_set(&DataSave0.ConstantParamData.BackLightFlag,1,1);
	else
		Os__saved_set(&DataSave0.ConstantParamData.BackLightFlag,0,1);
	XDATA_Write_Vaild_File(DataSaveConstant);
	return (SUCCESS);
}

unsigned char UTIL_GetKey_pp(unsigned char wait_sec)
{
    unsigned int Timeout;
    DRV_OUT *KBoard;

    Timeout=wait_sec*100;
    Os__timer_start(&Timeout);
    Os__xdelay(1);
    KBoard=Os__get_key_pp();
    while ((Timeout!=0)&&(KBoard->gen_status==DRV_RUNNING));
    Os__timer_stop(&Timeout);
    if(Timeout==0)
    {
        Os__abort_drv(drv_mmi);
		Os__xdelay(10);
        return  99;
    }
    return KBoard->xxdata[1];
}


unsigned char UTIL_ClearTrans(void)
{
	unsigned char KEY;
	unsigned char ucResult;


	/*�������*/
	if(DataSave0.ChangeParamData.ucTicketPrintFlag==0)
	{
        //Os__clr_display(255);
        //Os__GB2312_display(0, 0, (uchar *) "�Ƿ�ȷ���������?");
        //Os__GB2312_display(2, 0, (uchar *) "���-[ȷ��]");
        //Os__GB2312_display(3, 0, (uchar *) "�˳�-[ȡ��]");
		KEY = UTIL_GetKey(10);
	}
	else
	{
		KEY=KEY_ENTER;
	}
	if(KEY == KEY_ENTER)
	{
		Os__saved_set((unsigned char *)&DataSave0.ConstantParamData.ucFunctStep,
			0,sizeof(unsigned char));
		Os__saved_set((unsigned char *)&DataSave0.ConstantParamData.aucSettleFlag,
							0,sizeof(unsigned char));
		Os__saved_set(&DataSave0.ChangeParamData.ucCashierLogonFlag,0,1);

		Os__saved_set((unsigned char *)&DataSave0.ChangeParamData.uiTotalNumber,0,
			sizeof(unsigned int));
		memset(&DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid,0x00,sizeof(unsigned char));
		memset(&DataSave0.Trans_8583Data.ReversalISO8583Data,0x00,sizeof(REVERSALISO8583));
	    Os__saved_set((unsigned char*)&DataSave0.ChangeParamData.uiEMVICLogNum,0x00,sizeof(unsigned short));
		Os__saved_set((unsigned char*)&DataSave0.ChangeParamData.ucScriptInformValid,0x00,sizeof(unsigned char));

		Os__saved_copy((unsigned char *)&DataSave0.TransInfoData.TransTotal,
				(unsigned char *)&DataSave0.TransInfoData.LastTransTotal,
				sizeof(TRANSTOTAL));
		Os__saved_copy((unsigned char *)&DataSave0.TransInfoData.ForeignTransTotal,
				(unsigned char *)&DataSave0.TransInfoData.LastForeignTransTotal,
				sizeof(TRANSTOTAL));
		Os__saved_set((unsigned char *)&DataSave0.TransInfoData.ForeignTransTotal,
					0,sizeof(TRANSTOTAL));
		Os__saved_set((unsigned char *)&DataSave0.TransInfoData.TransTotal,
					0,sizeof(TRANSTOTAL));
		Os__saved_set((unsigned char *)DataSave0.TransInfoData.auiTransIndex,
					0,sizeof(unsigned short)*TRANS_MAXNB);
		Os__saved_set((unsigned char *)&DataSave0.TransInfoData.TransDetail,
					0,sizeof(TRANSDETAIL)*6);


		ucResult = XDATA_Write_Vaild_File(DataSaveConstant);
		 if(ucResult==SUCCESS)
			ucResult = XDATA_Clear_SaveTrans_File();
		if(ucResult==SUCCESS)
			ucResult =UTIL_DeleteAllData((PUCHAR)ERRICCTRANS);
		if(ucResult==SUCCESS)
			ucResult =UTIL_DeleteAllData((PUCHAR)TRANSLOG);
		if(ucResult==SUCCESS)
			ucResult = XDATA_Write_Vaild_File(DataSaveChange);
		if(ucResult==SUCCESS)
			ucResult = XDATA_Write_Vaild_File(DataSaveCashier);
		if(ucResult==SUCCESS)
			ucResult = XDATA_Write_Vaild_File(DataSaveTransInfo);
		if(ucResult==SUCCESS)
			ucResult = XDATA_Write_Vaild_File(DataSaveTrans8583);
		if (ucResult != SUCCESS)
		{
			MSG_DisplayErrMsg(ucResult);
		}
		if(ucResult==SUCCESS)
		{
            //Os__clr_display(255);
            //Os__GB2312_display(0, 0, (uchar *) "��������ɹ�!");
		}else
		{
            //Os__clr_display(255);
            //Os__GB2312_display(0, 0, (uchar *) "�������ʧ��!");
		}
		if(DataSave0.ChangeParamData.ucTicketPrintFlag==0)
			UTIL_GetKey(50);
	}

	return SUCCESS ;
}

unsigned char UTIL_ClearCollectData(void)
{
	unsigned char KEY;

    //Os__clr_display(255);
    //Os__GB2312_display(0, 0, (uchar *) "�Ƿ�����ɼ�����");
    //Os__GB2312_display(2, 0, (uchar *) "���-[ȷ��]");
    //Os__GB2312_display(3, 0, (uchar *) "�˳�-[ȡ��]");
	KEY = UTIL_GetKey(10);
	if (KEY!=KEY_ENTER)
		return(SUCCESS);

	XDATA_Clear_CollectTrans_File();
    //Os__clr_display(255);
    //Os__GB2312_display(0, 0, (uchar *) "����ɹ�!");
	UTIL_GetKey(5);
}


unsigned char UTIL_CheckSettlePrint(void)
{
	unsigned char ucResult;
	ucResult = SUCCESS;

	if(DataSave0.ChangeParamData.ucTicketPrintFlag)
	{
		/***��ӡ����ͳ�Ƶ�***/
		PRINT_List(NORMAL_PRINT_FLAG|PRINT_SETTLE_FLAG);
		/*��ӡ����δ���͵��ѻ���ϸ*/
		PRINT_OfflineDetial();
		UTIL_ClearTrans();
		Os__saved_set(&DataSave0.ChangeParamData.ucTicketPrintFlag,0,1);
		ucResult =XDATA_Write_Vaild_File(DataSaveChange);
		if(ucResult!=SUCCESS)
			return(ucResult);
	}
	return(ucResult);
}

unsigned char UITL_Delete_Script_Flag(void)
{
	unsigned char ucResult;

	ucResult=SUCCESS;
	Os__saved_set((unsigned char *)(&DataSave0.ChangeParamData.ucScriptInformValid),
				0,sizeof(unsigned char));

	ucResult=XDATA_Write_Vaild_File(DataSaveChange);
	if (ucResult != SUCCESS)
	{
		MSG_DisplayErrMsg(ucResult);
	}
	if(!ucResult)
	{
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char*)"����ɹ�");
	}
	else
	{
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char*)"���ʧ��");
	}

	UTIL_GetKey(10);
	return SUCCESS;
}

unsigned char UTIL_ClearMemery(void)
{
	unsigned char KEY;

	/*�������*/
    //Os__clr_display(255);
    //Os__GB2312_display(0, 0, (uchar *) "�Ƿ�ȷ������ڴ�?");
    //Os__GB2312_display(2, 0, (uchar *) "���-[ȷ��]");
    //Os__GB2312_display(3, 0, (uchar *) "�˳�-[ȡ��]");

	KEY = UTIL_GetKey(10);

	if(KEY == KEY_ENTER)
	{
		UTIL_SetDefault_State(false);  //Set Default val

        //Os__clr_display(255);
        //Os__GB2312_display(0, 0, (uchar *) "�ڴ�����ɹ�!");

		UTIL_GetKey(5);

	}
	return SUCCESS ;

}


void UTIL_TestDispStr(unsigned char * pucInBuf ,unsigned int uiLen ,unsigned char * DispInfo, unsigned char flag)
{
	int uiI;

	if(flag )
		util_Printf("\n---------------------%s---------------------\n",DispInfo);
	else
		util_Printf("\n%s",DispInfo);
	for(uiI=0;uiI<uiLen;uiI++)
	{
		util_Printf("%02x ",pucInBuf[uiI]);
	}
	if(flag )
		util_Printf("\n---------------------------------------------\n");
	else
		util_Printf("\n");

}

//#define DispEMVTransResult_For_English
void UTIL_DispTransResult(TRANSRESULT	enTransResult)
{
        //Os__clr_display(255);
		switch(enTransResult)
		{
		case OFFLINEAPPROVED:
			#ifndef DispEMVTransResult_For_English
            //Os__GB2312_display(1,0,"    �ѻ��ɹ�");
            //Os__GB2312_display(2,0,"    ���ڴ�ӡ...");
			#else
            //Os__GB2312_display(2,0,"OFFLINE APPROVED");
			#endif
			break;
		case OFFLINEDECLINED:
			#ifndef DispEMVTransResult_For_English
            //Os__GB2312_display(1,0,"    ����ʧ��");
            //Os__GB2312_display(2,0,"    �ѻ��ܾ�");
			#else
            //Os__GB2312_display(2,0,"OFFLINE DECLINED");
			#endif
			UTIL_GetKey(30);
			break;

		case ONLINEAPPROVED:
			#ifndef DispEMVTransResult_For_English
            //Os__GB2312_display(1,0,"    �����ɹ�");
            //Os__GB2312_display(2,0,"    ���ڴ�ӡ...");
			#else
            //Os__GB2312_display(2,0,"ONLINE APPROVED");
			#endif
			break;
		case ONLINEDECLINED:
			#ifndef DispEMVTransResult_For_English
            //Os__GB2312_display(1,0,"    ����ʧ��");
            //Os__GB2312_display(2,0,"    �����ܾ�");
			#else
            //Os__GB2312_display(2,0,"ONLINE DECLINED");
			#endif
			UTIL_GetKey(30);
			break;
		case UNABLEONLINE_OFFLINEAPPROVED:
			#ifndef DispEMVTransResult_For_English
            //Os__GB2312_display(1,0,"    ����ʧ��");
            //Os__GB2312_display(2,0,"    �ѻ��ɹ�");
            //Os__GB2312_display(3,0,"    ���ڴ�ӡ...");
			#else
            //Os__GB2312_display(2,0,"OFFLINE APPROVED");
			#endif
			break;
		case UNABLEONINE_OFFLINEDECLINED:
			#ifndef DispEMVTransResult_For_English
            //Os__GB2312_display(1,0,"    ����ʧ��");
            //Os__GB2312_display(2,0,"    �ѻ��ܾ�");
			#else
            //Os__GB2312_display(2,0,"OFFLINE DECLINED");
			#endif
			UTIL_GetKey(30);
			break;
		default:
			#ifndef DispEMVTransResult_For_English
            //Os__GB2312_display(2,0,"����ȷ�Ľ��׽��");
			#else
            //Os__GB2312_display(2,2,"UNKNOWN");
			#endif
			UTIL_GetKey(30);
			break;
		}

}

void UTIL_SavEMVTransDate(void)
{
	unsigned char aucDateTime[16];

	memset( aucDateTime, 0, sizeof(aucDateTime));

	Os__read_date(aucDateTime);
	memcpy(TransReqInfo.aucTransDate,"20",2);
	memcpy(&TransReqInfo.aucTransDate[2],&aucDateTime[4],2);
	memcpy(&TransReqInfo.aucTransDate[4],&aucDateTime[2],2);
	memcpy(&TransReqInfo.aucTransDate[6],&aucDateTime[0],2);
	Os__read_time_sec(TransReqInfo.aucTransTime );
	srand(CONV_AscLong(TransReqInfo.aucTransDate,sizeof(TransReqInfo.aucTransDate))
				+CONV_AscLong(TransReqInfo.aucTransTime,sizeof(TransReqInfo.aucTransTime)));
	asc_bcd(G_NORMALTRANS_aucDate, TRANSDATELEN/2,  TransReqInfo.aucTransDate, TRANSDATELEN);
	asc_bcd(G_NORMALTRANS_aucTime, TRANSTIMELEN/2,  TransReqInfo.aucTransTime, TRANSTIMELEN);
}

unsigned char UTIL_CheckTransAmount(void)
{
	ULONG_C51	ulTotalAmount,ulTotalAmountForeign,ulAmountLimit;
	UCHAR ucAmountLimit[CFG_MAXAMOUNTLEN] ;
	TRANSTOTAL	*pTransTotal;
	TRANSTOTAL	*pTransTotalForeign;

	ulAmountLimit =0xEE6B2800;
	long_asc(ucAmountLimit,CFG_MAXAMOUNTLEN,&ulAmountLimit);

	util_Printf("DataSave0.TransInfoData.TransTotal = %dl\n",DataSave0.TransInfoData.TransTotal);
	util_Printf("DataSave0.TransInfoData.ForeignTransTotal = %dl\n",DataSave0.TransInfoData.ForeignTransTotal);
	pTransTotal = &DataSave0.TransInfoData.TransTotal;
	pTransTotalForeign = &DataSave0.TransInfoData.ForeignTransTotal;

	switch(G_NORMALTRANS_ucTransType)
	{
		case TRANS_PURCHASE:
		case TRANS_PREAUTHFINISH:
		case TRANS_OFFPREAUTHFINISH:
		case TRANS_PREAUTHSETTLE:
		case TRANS_OFFPURCHASE:
		case TRANS_CUPMOBILE:
			ulTotalAmount = ulAmountLimit - pTransTotal->ulDebitAmount ;
			ulTotalAmountForeign = ulAmountLimit - pTransTotalForeign->ulDebitAmount ;

			if(ulTotalAmount < G_NORMALTRANS_ulAmount
				|| ulTotalAmountForeign < G_NORMALTRANS_ulAmount )
			{
				return ERR_CHECKAOUNMT;
			}
			break;
		case TRANS_VOIDCUPMOBILE:
		case TRANS_VOIDPURCHASE:
		case TRANS_VOIDOFFPURCHASE:
		case TRANS_VOIDPREAUTHFINISH:
		case TRANS_REFUND:
			ulTotalAmount = ulAmountLimit - pTransTotal->ulCreditAmount ;
			ulTotalAmountForeign = ulAmountLimit - pTransTotalForeign->ulCreditAmount ;
			if(ulTotalAmount < G_NORMALTRANS_ulAmount
				|| ulTotalAmountForeign < G_NORMALTRANS_ulAmount )
				return ERR_CHECKAOUNMT;
			break;
		case TRANS_TIPADJUST:
			if(G_RUNDATA_ucAdjustFlag!=1)
			{
				ulTotalAmount = ulAmountLimit - pTransTotal->ulDebitAmount ;
				ulTotalAmountForeign = ulAmountLimit - pTransTotalForeign->ulDebitAmount ;
				if(ulTotalAmount < G_NORMALTRANS_ulFeeAmount
					|| ulTotalAmountForeign < G_NORMALTRANS_ulFeeAmount )
					return ERR_CHECKAOUNMT;
			}

			if(G_RUNDATA_ucAdjustFlag == 0)	/*����һ�������ӵ������ܽ��*/
			{
				ulTotalAmount = ulAmountLimit - pTransTotal->ulPurchaseAmount ;
				ulTotalAmountForeign = ulAmountLimit - pTransTotalForeign->ulPurchaseAmount ;
				if(ulTotalAmount < G_NORMALTRANS_ulFeeAmount
					|| ulTotalAmountForeign < G_NORMALTRANS_ulFeeAmount )
					return ERR_CHECKAOUNMT;
			}else
			{
				/*�����߽����ۼӵ������ۼ�����*/
				if(G_RUNDATA_ucAdjustFlag != 1)
				{
					ulTotalAmount = ulAmountLimit - pTransTotal->ulOfflineAmount ;
					ulTotalAmountForeign = ulAmountLimit - pTransTotalForeign->ulOfflineAmount ;
					if(ulTotalAmount < G_NORMALTRANS_ulFeeAmount
						|| ulTotalAmountForeign < G_NORMALTRANS_ulFeeAmount )
						return ERR_CHECKAOUNMT;
				}
			}

			break;
		case TRANS_PREAUTH:
		case TRANS_PREAUTHADD:
			ulTotalAmount = ulAmountLimit - pTransTotal->ulAuthAmount ;
			ulTotalAmountForeign = ulAmountLimit - pTransTotalForeign->ulAuthAmount ;

			if(ulTotalAmount < G_NORMALTRANS_ulAmount
				|| ulTotalAmountForeign < G_NORMALTRANS_ulAmount )
				return ERR_CHECKAOUNMT;
			break;
		default:
			break;
	}
	return SUCCESS;
}

void UTIL_SavEMVTransData(void)
{
	unsigned char aucDateTime[16];

	memset(&TransReqInfo,0x00,sizeof(TRANSREQINFO));
	memset( aucDateTime, 0, sizeof(aucDateTime));
	//�������ڡ�ʱ�䡢���������������(GOODS)�����նˡ��̻����̻����ơ����۵����뷽ʽ(0x02)���Ƿ�ǿ������
#ifdef GUI_TTS_DATA
	memcpy(TransReqInfo.aucTransDate,"20120621",8);
	memcpy(TransReqInfo.aucTransTime,"103510",6);
#else
	Os__read_date(aucDateTime);
	memcpy(TransReqInfo.aucTransDate,"20",2);
	memcpy(&TransReqInfo.aucTransDate[2],&aucDateTime[4],2);
	memcpy(&TransReqInfo.aucTransDate[4],&aucDateTime[2],2);
	memcpy(&TransReqInfo.aucTransDate[6],&aucDateTime[0],2);
	Os__read_time_sec(TransReqInfo.aucTransTime );
#endif
	srand(CONV_AscLong(TransReqInfo.aucTransDate,sizeof(TransReqInfo.aucTransDate))
			+CONV_AscLong(TransReqInfo.aucTransTime,sizeof(TransReqInfo.aucTransTime)));
	asc_bcd(G_NORMALTRANS_aucDate, TRANSDATELEN/2,  TransReqInfo.aucTransDate, TRANSDATELEN);
	asc_bcd(G_NORMALTRANS_aucTime, TRANSTIMELEN/2,  TransReqInfo.aucTransTime, TRANSTIMELEN);

	TransReqInfo.uiRandNum= rand();
	switch(G_NORMALTRANS_ucTransType)
	{
		case TRANS_EC_ASSIGNLOAD:
		case TRANS_EC_UNASSIGNLOAD:
		case TRANS_EC_CASHLOAD:
			TransReqInfo.enTransType = TRANSFER;
			break;
		case TRANS_PREAUTH:
			TransReqInfo.enTransType = PREAUTHOR;
			break;
		default:
			TransReqInfo.enTransType = GOODS;
			break;
	}
	switch(G_NORMALTRANS_ucTransType)
	{
		case TRANS_EC_ASSIGNLOAD:
		case TRANS_EC_UNASSIGNLOAD:
		case TRANS_EC_CASHLOAD:
			TransReqInfo.uiAmount = G_NORMALTRANS_ulAmount;
			break;
		default:
			TransReqInfo.uiAmount = G_NORMALTRANS_ulAmount;
			break;
	}
	memcpy(TransReqInfo.aucTerminalID,ConstParam.aucTerminalID,TERMINALIDLEN);
	memcpy(TransReqInfo.aucMerchantID,ConstParam.aucMerchantID,MERCHANTIDLEN);
	memcpy(TransReqInfo.aucMerchantName,ConstParam.aucMerchantName,MERCHANTNAMELEN);
	TransReqInfo.ucEntryMode = 0x02;		//9F39  ���۵����뷽ʽ
	if(G_NORMALTRANS_ucTransType == TRANS_PURCHASE/*&&I_OffSale*/)//���ѽ�����֧������ʱ�����Բ�ǿ��������������ǿ������
		TransReqInfo.bForceOnline = false;
	else
		TransReqInfo.bForceOnline=TRUE;
	//TransReqInfo.bForceOnline = false;
	util_Printf("\nTransReqInfo.bForceOnline=abc=%d",TransReqInfo.bForceOnline);
}


unsigned char UTIL_DisplyTransType(unsigned char ucInTransType ,unsigned char Line ,unsigned char Col ,unsigned char ucClrFlag ,unsigned char ucClrLine)
{

    if(ucClrFlag)	//Os__clr_display(ucClrLine);

	switch(G_NORMALTRANS_ucTransType)
	{
		case TRANS_PURCHASE:
            //Os__GB2312_display(0, 2, (uchar * )" (����)");
   			break;
		case TRANS_VOIDPURCHASE:
            //Os__GB2312_display(0, 1, (uchar * )" (���ѳ���)");
			break;
		case TRANS_PREAUTH:
            //Os__GB2312_display(0, 2, (uchar * )"(Ԥ��Ȩ)");
   			break;
   		case TRANS_VOIDPREAUTH:
            //Os__GB2312_display(0, 1, (uchar * )"(Ԥ��Ȩ����)");
			break;
		case TRANS_VOIDPREAUTHFINISH:
            //Os__GB2312_display(0, 0, (uchar * )"(Ԥ��Ȩ��ɳ���)");
			break;
		case TRANS_PREAUTHFINISH:
            //Os__GB2312_display(0, 0, (uchar * )"(Ԥ��Ȩ�������)");
			break;
		case TRANS_REFUND:
            //Os__GB2312_display(0, 2, (uchar * )" (�˻�)");
			break;
		case TRANS_TIPADJUST:
		case TRANS_TIPADJUSTOK:
            //Os__GB2312_display(0, 1, (uchar * )" (���ߵ���)");
			break;
		case TRANS_OFFPREAUTHFINISH:
            //Os__GB2312_display(0, 1, (uchar * )" (���߽���)");
			break;
		case TRANS_UNDOOFF:
            //Os__GB2312_display(0, 1, (uchar * )" (���߳���)");
			break;
		case TRANS_PREAUTHSETTLE:
            //Os__GB2312_display(0, 0, (uchar * )"(Ԥ��Ȩ�������)");
			break;
		case TRANS_OFFPURCHASE:
            //Os__GB2312_display(0, 1, (uchar * )" (�ѻ�����)");
			break;
		case TRANS_VOIDOFFPURCHASE:
            //Os__GB2312_display(0, 0, (uchar * )" (�ѻ����ѳ���)");
			break;
		case TRANS_QUERYBAL:
            //Os__GB2312_display(0, 1, (uchar * )" (����ѯ)");
			break;
		case TRANS_PREAUTHADD:
            //Os__GB2312_display(0, 1, (uchar * )"(׷��Ԥ��Ȩ)");
			break;
		case TRANS_DEPOSIT:
            //Os__GB2312_display(0, 2, (uchar * )" (Ȧ��)");
			break;
		default:
            //Os__GB2312_display(0, 0, (uchar * )" ");
			break;
	}
	return (SUCCESS);
}

unsigned char UTIL_SavPrinterParamToPOS(unsigned char ucFlag)
{
    unsigned char ch;

	ch = ucFlag;

        if(ch=='1') //------ ����------------
        {
                Os__saved_set(&DataSave0.ConstantParamData.prnflag,1,1);
                Os__saved_set(&DataSave0.ConstantParamData.prnflagNew,0,1);
                Os__saved_set(&DataSave0.ConstantParamData.prnflagOld,0,1);
                Os__saved_set(&DataSave0.ConstantParamData.linenum,0x16,1);
		  Os__saved_set(&DataSave0.ConstantParamData.font,1,1);
                Os__saved_set(&DataSave0.ConstantParamData.printtimes,2,1);

        }
        else
	 if(ch=='2') //------ ���------------
        {
                Os__saved_set(&DataSave0.ConstantParamData.prnflag,0,1);
                Os__saved_set(&DataSave0.ConstantParamData.prnflagNew,0,1);
                Os__saved_set(&DataSave0.ConstantParamData.prnflagOld,0,1);
                Os__saved_set(&DataSave0.ConstantParamData.linenum,0x15,1);
                Os__saved_set(&DataSave0.ConstantParamData.font,1,1);
                Os__saved_set(&DataSave0.ConstantParamData.printtimes,1,1);

        }
        else
	 if(ch=='3') //------ �״���ֽ------
        {
                Os__saved_set(&DataSave0.ConstantParamData.prnflag,0,1);
                Os__saved_set(&DataSave0.ConstantParamData.prnflagNew,0,1);
                Os__saved_set(&DataSave0.ConstantParamData.prnflagOld,1,1);
                Os__saved_set(&DataSave0.ConstantParamData.linenum,0x08,1);
                Os__saved_set(&DataSave0.ConstantParamData.font,0x01,1);
                Os__saved_set(&DataSave0.ConstantParamData.printtimes,1,1);
        }
	 else
        if(ch=='4') //------ �״���ֽ------
        {
                Os__saved_set(&DataSave0.ConstantParamData.prnflag,0,1);
                Os__saved_set(&DataSave0.ConstantParamData.prnflagNew,1,1);
                Os__saved_set(&DataSave0.ConstantParamData.prnflagOld,0,1);
                Os__saved_set(&DataSave0.ConstantParamData.linenum,0x08,1);
                Os__saved_set(&DataSave0.ConstantParamData.font,0x01,1);
                Os__saved_set(&DataSave0.ConstantParamData.printtimes,1,1);
        }
	return (SUCCESS);
}


unsigned char GetINCardName(unsigned char ucFlag ,unsigned char *temp,unsigned char *aucPrintBuf)
{
	if (!memcmp(temp,"0001",4))
	{
		memcpy(aucPrintBuf,"������",8);
		return 0;
	}
	if (!memcmp(temp,"0102",4))
	{
		memcpy(aucPrintBuf,"��������",8);
		return 0;
	}
	if (!memcmp(temp,"0103",4))
	{
		memcpy(aucPrintBuf,"ũҵ����",8);
		return 0;
	}

	if (!memcmp(temp,"0104",4))
	{
		memcpy(aucPrintBuf,"�й�����",8);
		return 0;
	}
	if (!memcmp(temp,"0105",4))
	{
		memcpy(aucPrintBuf,"��������",8);
		return 0;
	}
	if (!memcmp(temp,"0100",4))
	{
		memcpy(aucPrintBuf,"�й�������������",16);
		return 0;
	}
	if (!memcmp(temp,"0301",4))
	{
		memcpy(aucPrintBuf,"��ͨ����",8);
		return 0;
	}
	if (!memcmp(temp,"0302",4))
	{
		memcpy(aucPrintBuf,"��������",8);
		return 0;
	}
	if (!memcmp(temp,"0303",4))
	{
		memcpy(aucPrintBuf,"�������",8);
		return 0;
	}
	if (!memcmp(temp,"0304",4))
	{
		memcpy(aucPrintBuf,"��������",8);
		return 0;
	}
	if (!memcmp(temp,"0305",4))
	{
		memcpy(aucPrintBuf,"��������",8);
		return 0;
	}
	if (!memcmp(temp,"0306",4))
	{
		memcpy(aucPrintBuf,"�㷢����",8);
		return 0;
	}
	if (!memcmp(temp,"0307",4))
	{
		memcpy(aucPrintBuf,"�չ",6);
		return 0;
	}
	if (!memcmp(temp,"0308",4))
	{

		memcpy(aucPrintBuf,"��������",8);
		return 0;
	}
	if (!memcmp(temp,"0309",4))
	{
		memcpy(aucPrintBuf,"��ҵ����",8);
		return 0;
	}
	if (!memcmp(temp,"0310",4))
	{

		memcpy(aucPrintBuf,"�ַ�����",8);
		return 0;
	}

	if (!memcmp(temp,"0311",4)
	 || (!memcmp(temp,"0401",4)))
	{
		memcpy(aucPrintBuf,(unsigned char *)"�Ϻ�����",8);
		return 0;
	}

	//Add by lote 2008-03-17
	if (!memcmp(temp,"6501",4)
		||(!memcmp(temp,"0402",4))
		||(!memcmp(temp,"1401",4))
		)//"�Ϻ�ũ��"�Ѹ���Ϊ�Ϻ�ũ����ҵ���У�����Ϻ�ũ������
	{
		//strcpy((char *)aucPrintBuf,"�Ϻ�ũ����ҵ����");
		memcpy(aucPrintBuf,"�Ϻ�ũ��",8);
		return 0;
	}
	if (!memcmp(temp,"0408",4)||!memcmp(temp,"6408",4))
	{
		memcpy(aucPrintBuf,"��������",8);
		return 0;
	}

	if (!memcmp(temp,"0410",4))
	{
		memcpy(aucPrintBuf,"ƽ������",8);
		return 0;
	}
	if (!memcmp(temp,"0316",4))
	{
		memcpy(aucPrintBuf,"��������",8);
		return 0;
	}
	if (!memcmp(temp,"0317",4))
	{
		memcpy(aucPrintBuf,"��������",8);
		return 0;
	}
	if (!memcmp(temp,"0403",4))
	{
		memcpy(aucPrintBuf,"��������",8);
		return 0;
	}
	if (!memcmp(temp,"0417",4))
	{
		memcpy(aucPrintBuf,"ʢ������",8);
		return 0;
	}
	if (!memcmp(temp,"0420",4))
	{
		memcpy(aucPrintBuf,"��������",8);
		return 0;
	}
	if (!memcmp(temp,"0423",4))
	{
		memcpy(aucPrintBuf,"��������",8);
		return 0;
	}
	if (!memcmp(temp,"0434",4))
	{
		memcpy(aucPrintBuf,"�������",8);
		return 0;
	}
	if (!memcmp(temp,"4802",4))
	{
		//strcpy((char *)aucPrintBuf,"�����������ֹ�˾");
		memcpy(aucPrintBuf,"��������",8);
		return 0;
	}
	//add 09-07-01
	if (!memcmp(temp,"0414",4)
		||!memcmp(temp,"0507",4)
		)
	{
		memcpy(aucPrintBuf,"��������",8);
		return 0;
	}
	if (!memcmp(temp,"1410",4))
	{
		memcpy(aucPrintBuf,"�����ź�",8);
		return 0;
	}
	if (!memcmp(temp,"0432",4))
	{
		memcpy(aucPrintBuf,"�˲�����",8);
		return 0;
	}
	if (!memcmp(temp,"0529",4))
	{
		memcpy(aucPrintBuf,"�差����",8);
		return 0;
	}
	//End
	/******************************End*******************************/
	if(ucFlag==1)
	{
		if(G_NORMALTRANS_ucCardType == TRANS_CARDTYPE_INTERNAL)
		{
			memcpy(aucPrintBuf,temp,8);
		}
		else
		{
			memcpy(aucPrintBuf,temp,8);
		}
	}else
	if(ucFlag==2)
	{
		memcpy(aucPrintBuf,temp,11);
	}
	return 0;

}
void UTIL_ClearEMVInterfaceData(void)
{

	memset(&ICTransInfo , 0 ,sizeof(ICTRANSINFO));

	memset(AIDParam , 0 ,sizeof(AIDParam));

	ucTermCAPKNum=0;

	ucTermAIDNum=0;
	memset(AppListTerm , 0 ,sizeof(AppListTerm));

	ucCAPKNum=0;
	memset(CAPKInfo , 0 ,sizeof(CAPKInfo));

	SupportAIDNum=0;
	memset(SupportAID , 0 ,sizeof(SupportAID));
}

void UTIL_PINPADDispLOGO(void)
{
	if(DataSave0.ConstantParamData.Pinpadflag ==1)
	{
		//if((!ReaderSupport)||(ReaderType!=READER_SAND))
		{
			Os__clr_display_pp(255);
			Os__light_on_pp();
			//Os__GB2312_display_pp(0,0,(uchar *)"   ɼ������ͨ");
			Os__GB2312_display_pp(0,0,(uchar *)"    ��ӭʹ��");
		}
	}
}


unsigned char UITL_Input_Rate(void)
{
	unsigned char aucBuf[40],ucTemp;
	ULONG_C51	ulTemp;

	memset(aucBuf,0,sizeof(aucBuf));
    //Os__clr_display(255);
    //Os__GB2312_display(0, 0, (uchar *) "����ҿ���");
	long_asc(aucBuf,3,&DataSave0.ConstantParamData.ulRMBRate);
	if (UTIL_Input(1,true,3,3,'N',aucBuf,0) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}else
	{
		ulTemp = (unsigned long)asc_long(aucBuf,3);
		Os__saved_copy((unsigned char *)&ulTemp,
			(unsigned char *)&DataSave0.ConstantParamData.ulRMBRate,sizeof(unsigned long));
	}

	memset(aucBuf,0,sizeof(aucBuf));
    //Os__clr_display(255);
    //Os__GB2312_display(0, 0, (uchar *) "VISA����");
	long_asc(aucBuf,3,&DataSave0.ConstantParamData.ulVISARate);
	if (UTIL_Input(1,true,3,3,'N',aucBuf,0) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}else
	{
		ulTemp = (unsigned long)asc_long(aucBuf,3);
		Os__saved_copy((unsigned char *)&ulTemp,
			(unsigned char *)&DataSave0.ConstantParamData.ulVISARate,sizeof(unsigned long));
	}

	memset(aucBuf,0,sizeof(aucBuf));
    //Os__clr_display(255);
    //Os__GB2312_display(0, 0, (uchar *) "MASTER����");
	long_asc(aucBuf,3,&DataSave0.ConstantParamData.ulMASTERRate);
	if (UTIL_Input(1,true,3,3,'N',aucBuf,0) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}else
	{
		ulTemp = (unsigned long)asc_long(aucBuf,3);
		Os__saved_copy((unsigned char *)&ulTemp,
			(unsigned char *)&DataSave0.ConstantParamData.ulMASTERRate,sizeof(unsigned long));
	}

	memset(aucBuf,0,sizeof(aucBuf));
    //Os__clr_display(255);
    //Os__GB2312_display(0, 0, (uchar *) "JCB����");
	long_asc(aucBuf,3,&DataSave0.ConstantParamData.ulJCBRate);
	if (UTIL_Input(1,true,3,3,'N',aucBuf,0) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}else
	{
		ulTemp = (unsigned long)asc_long(aucBuf,3);
		Os__saved_copy((unsigned char *)&ulTemp,
			(unsigned char *)&DataSave0.ConstantParamData.ulJCBRate,sizeof(unsigned long));
	}

	memset(aucBuf,0,sizeof(aucBuf));
    //Os__clr_display(255);
    //Os__GB2312_display(0, 0, (uchar *) "DINNER����");
	long_asc(aucBuf,3,&DataSave0.ConstantParamData.ulDINERRate);
	if (UTIL_Input(1,true,3,3,'N',aucBuf,0) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}else
	{
		ulTemp = (unsigned long)asc_long(aucBuf,3);
		Os__saved_copy((unsigned char *)&ulTemp,
			(unsigned char *)&DataSave0.ConstantParamData.ulDINERRate,sizeof(unsigned long));
	}

	memset(aucBuf,0,sizeof(aucBuf));
    //Os__clr_display(255);
    //Os__GB2312_display(0, 0, (uchar *) "AMEX����");
	long_asc(aucBuf,3,&DataSave0.ConstantParamData.ulAMEXRate);
	if (UTIL_Input(1,true,3,3,'N',aucBuf,0) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}else
	{
		ulTemp = (unsigned long)asc_long(aucBuf,3);
		Os__saved_copy((unsigned char *)&ulTemp,
			(unsigned char *)&DataSave0.ConstantParamData.ulAMEXRate,sizeof(unsigned long));
	}

	memset(aucBuf,0,sizeof(aucBuf));
    //Os__clr_display(255);
    //Os__GB2312_display(0, 0, (uchar *) "�����Ƿ��ӡ");
    //Os__GB2312_display(3, 0, (uchar *) "1:��ӡ  0:����ӡ ");
	aucBuf[0] = DataSave0.ConstantParamData.ucPrint;
	if (UTIL_Input(1,true,1,1,'N',aucBuf,0) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}else
	{
		ucTemp = aucBuf[0];
		Os__saved_copy(&ucTemp,
			&DataSave0.ConstantParamData.ucPrint,sizeof(unsigned char));
	}
	XDATA_Write_Vaild_File(DataSaveConstant);
	return SUCCESS;

}


unsigned char UTIL_BANK_Process(void)
{
    unsigned char ch,ucresult;
    unsigned char BANKID[8][18] =
    {
        {"��ͨ�����Ϻ�����\0"},
        {"���������Ϻ�����\0"},
        {"���������Ϻ�����\0"},
        {"��ҵ�����Ϻ�����\0"},

        {"���������Ϻ�����\0"},
        {"ũҵ�����Ϻ�����\0"},
        {"�չ�Ϻ�����\0"},
        {"��������Ϻ�����\0"},


    };
    unsigned char BANKID1[][18]=//ADD BY JIANG.GS
    {
        {"�й������Ϻ�����\0"},//ADD BY JIANG.GS
        {"���������Ϻ�����\0"},
        {"�Ϻ�����\0"},
        {"���������Ϻ�����\0"},

        {"�㷢�����Ϻ�����\0"},
        {"���������Ϻ�����\0"},
        {"�ַ�����\0"},
        {"�Ϻ�ũ����ҵ����\0"},//{"ũ��������\0"},
    };

	unsigned char BANKID2[][18]=
	{
		{"���������Ϻ�����\0"},
		{"��������\0"},
	};

   //Os__clr_display(255);
   //Os__GB2312_display(0, 0, (uchar *) "��������������");
    Os__saved_set(DataSave0.ConstantParamData.Top, 0x00, Display_MAXNB);

    util_Printf("\n cflag == %02x \n",cflag);
    util_Printf("\n BankID == %02x \n",DataSave0.ConstantParamData.BankID);

    if(cflag==0)
    	Os__saved_copy((unsigned char *)BANKID[(DataSave0.ConstantParamData.BankID-1)],DataSave0.ConstantParamData.Top,16);
    else if(cflag==1)
    	Os__saved_copy((unsigned char *)BANKID1[(DataSave0.ConstantParamData.BankID-1)],DataSave0.ConstantParamData.Top,16);
	else
		Os__saved_copy((unsigned char *)BANKID2[(DataSave0.ConstantParamData.BankID-1)],DataSave0.ConstantParamData.Top,16);

    Os__saved_set(&DataSave0.ChangeParamData.ucCashierLogonFlag,0,1);

    XDATA_Write_Vaild_File(DataSaveConstant);
	XDATA_Write_Vaild_File(DataSaveChange);

    //Os__GB2312_display( 1,0,(unsigned char *)DataSave0.ConstantParamData.Top);
    //Os__GB2312_display( 2,0,(unsigned char *)"�Ƿ����ظ�����Կ");
    //Os__GB2312_display( 3,0,(unsigned char *)"   ��ȷ��");

    ch = UTIL_GetKey(30);
    if (ch == KEY_ENTER)
    {
    	if(cflag==0)
    		ucresult = KEY_Load_Default_Key();
    	else if(cflag==1)
    		ucresult = KEY_Load_Default_Key1();
		else
			ucresult = KEY_Load_Default_Key2();

         XDATA_Write_Vaild_File(DataSaveConstant);

        return ucresult;
    }else
    return ERR_CANCEL;
}


unsigned char UTIL_BANK_OTHERS(void)
{
 	unsigned char ucResult=SUCCESS;
	SELMENU ManagementMenu;
	memset(&ManagementMenu,0,sizeof(SELMENU));

	UTIL_GetMenu_Value(NULL,	BANK_CHINABANK,	NULL,	UTIL_BANK_Process,&ManagementMenu);
	UTIL_GetMenu_Value(NULL,	BANK_MINSHENG, 		NULL,	UTIL_BANK_Process,&ManagementMenu);
	UTIL_GetMenu_Value(NULL,	BANK_SHANGHAI, 		NULL,	UTIL_BANK_Process,&ManagementMenu);
	UTIL_GetMenu_Value(NULL,	BANK_ZHAOSHANG,	NULL,	UTIL_BANK_Process,&ManagementMenu);

	UTIL_GetMenu_Value(NULL,	BANK_GUANGFA,		NULL,	UTIL_BANK_Process,&ManagementMenu);
	UTIL_GetMenu_Value(NULL,     BANK_ZHONGXIN,         NULL,   	UTIL_BANK_Process,&ManagementMenu);
	UTIL_GetMenu_Value(NULL,     BANK_PUFA,        		NULL,   	UTIL_BANK_Process,&ManagementMenu);
	UTIL_GetMenu_Value(NULL,     BANK_COUNTRY,      	NULL,   	UTIL_BANK_Process,&ManagementMenu);

	//UTIL_GetMenu_Value(NULL,     BANK_POSTSAVE,      	NULL,   	UTIL_BANK_Process,&ManagementMenu);
	UTIL_GetMenu_Value(NULL,    BANK_OTHERS,      		NULL,   	UTIL_OTHERBANK_OTHERS,&ManagementMenu);

   	ucResult = SUCCESS;

	 if((ucResult == SUCCESS)
	 	&&(DataSave0.ChangeParamData.ucCashierLogonFlag!=CASH_LOGONFLAG) )
	 {
	 	SetBankflag = 1;
        	cflag=1;
	    	ucResult = UTIL_DisplayMenu(&ManagementMenu);
		SetBankflag = 0;
	       if( ucResult != SUCCESS&&ucResult != ERR_CANCEL&&ucResult != ERR_END)
	       {
	            MSG_DisplayErrMsg(ucResult);
	       }
	 }
   	 return(SUCCESS);

}
unsigned char UTIL_OTHERBANK_OTHERS(void)
{
 	unsigned char ucResult=SUCCESS;
	SELMENU ManagementMenu;
	memset(&ManagementMenu,0,sizeof(SELMENU));


	UTIL_GetMenu_Value(NULL,    BANK_NINGBO,      	NULL,   	UTIL_BANK_Process,&ManagementMenu);
	UTIL_GetMenu_Value(NULL,    BANK_POSTSAVE,      NULL,   	UTIL_BANK_Process,&ManagementMenu);

   	ucResult = SUCCESS;

	 if((ucResult == SUCCESS)
	 	&&(DataSave0.ChangeParamData.ucCashierLogonFlag!=CASH_LOGONFLAG) )
	 {
	 	SetBankflag = 1;
        	cflag=2;
	    	ucResult = UTIL_DisplayMenu(&ManagementMenu);
		SetBankflag = 0;
	       if( ucResult != SUCCESS&&ucResult != ERR_CANCEL&&ucResult != ERR_END)
	       {
	            MSG_DisplayErrMsg(ucResult);
	       }
	 }
   	 return(SUCCESS);

}
/*
��̬����
*/
void UTIL_GetDynamicPSW(unsigned char DynamicBuf[])
{
	unsigned char aucBuf1[20],Date1[20];
	unsigned long Year,month,day,uldate;

	memset( aucBuf1,0,sizeof(aucBuf1) );
	memset( Date1,0,sizeof(Date1) );
	Os__read_date(aucBuf1);

	util_Printf("\n***********��ȡ����************\n");
	util_Printf("aucBuf=%s\n",aucBuf1);

	memcpy( &Date1[0],&aucBuf1[4],2 );
	memcpy( &Date1[2],&aucBuf1[2],2 );
	memcpy( &Date1[4],&aucBuf1[0],2 );

	util_Printf("Date1=%s\n",Date1);

	Year = asc_long( &Date1[0],2);
	month = asc_long( &Date1[2],2);
	day = asc_long( &Date1[4],2);
	uldate = asc_long( &Date1[0],6);

	util_Printf("\n1111111111111111111\n");
	util_Printf("Year=%d\n",Year);
	util_Printf("month=%d\n",month);
	util_Printf("day=%d\n",day);
	util_Printf("11uldate=%d\n",uldate);

    uldate*=day;
	util_Printf("22uldate=%d\n",uldate);
    long_asc(DynamicBuf, 6, &uldate);
	util_Printf("DynamicBuf=%s\n",DynamicBuf);
	return;
}

unsigned char  UTIL_LoadMasterKey(unsigned char ucKeyIndex , unsigned char * pucInBuf , unsigned short usInLen)
{
	unsigned char ucResult;
	unsigned char aucAscData[33];
	unsigned char aucAscData1[17];
	unsigned char aucAscData2[17];
	unsigned char aucHexData[9];
	unsigned char aucHexData1[9];
	unsigned char ucArrayIndex;

	memset(aucAscData,0,sizeof(aucAscData));
	memset(aucAscData1,0,sizeof(aucAscData1));
	memset(aucAscData2,0,sizeof(aucAscData2));
	memset(aucHexData,0,sizeof(aucHexData));
	memset(aucHexData1,0,sizeof(aucHexData1));

	ucArrayIndex = KEYARRAY_GOLDENCARDSH;

	memcpy(aucAscData, pucInBuf, usInLen);
	memcpy(aucAscData1, aucAscData, 16);
	memcpy(aucAscData2, &aucAscData[16], 16);

	asc_hex(aucHexData,8,aucAscData1,16);
	asc_hex(aucHexData1,8,aucAscData2,16);

	util_Printf("ucKeyIndex = %02x\n",ucKeyIndex);
	UTIL_TestDispStr(aucAscData,usInLen,(UCHAR *)"Master key asc = ",NULL);
	UTIL_TestDispStr(aucHexData,8,(UCHAR *)"Master key hex 1= ",NULL);
	UTIL_TestDispStr(aucHexData1,8,(UCHAR *)"Master key hex 2= ",NULL);

	ucResult = PINPAD_47_Crypt8ByteSingleKey(0x0E,
			KEYINDEX_GOLDENCARDSH_KEK,aucHexData,aucHexData);
	if(ucResult )	return(ucResult);
	UTIL_TestDispStr(aucHexData,8,(UCHAR *)" Cry Master key hex 1= ",NULL);

	ucResult = PINPAD_47_Encrypt8ByteSingleKey(0x0E,
					KEYINDEX_GOLDENCARDSH_KEK+1,aucHexData,aucHexData);
	if(ucResult )	return(ucResult);
		UTIL_TestDispStr(aucHexData,8,(UCHAR *)"Enc Master key hex 2= ",NULL);

	ucResult = PINPAD_47_Crypt8ByteSingleKey(0x0E,
			KEYINDEX_GOLDENCARDSH_KEK,aucHexData,aucHexData);
	if(ucResult )	return(ucResult);
	UTIL_TestDispStr(aucHexData,8,(UCHAR *)" Cry Master key hex 3= ",NULL);
		if(ucResult )	return(ucResult);

	ucResult = PINPAD_47_Encrypt8ByteSingleKey(ucArrayIndex,
					ucKeyIndex,aucHexData,aucHexData);
	if(ucResult )	return(ucResult);
	ucResult = PINPAD_42_LoadSingleKeyUseSingleKey(ucArrayIndex,
					ucKeyIndex,ucKeyIndex,aucHexData);

	ucResult = PINPAD_47_Crypt8ByteSingleKey(0x0E,
			KEYINDEX_GOLDENCARDSH_KEK,aucHexData1,aucHexData1);
	UTIL_TestDispStr(aucHexData1,8,(UCHAR *)" Cry Master key hex 21= ",NULL);
	if(ucResult )	return(ucResult);
	ucResult = PINPAD_47_Encrypt8ByteSingleKey(0x0E,KEYINDEX_GOLDENCARDSH_KEK+1,aucHexData1,aucHexData1);
	if(ucResult )	return(ucResult);
		UTIL_TestDispStr(aucHexData1,8,(UCHAR *)"Enc Master key hex 22= ",NULL);

	ucResult = PINPAD_47_Crypt8ByteSingleKey(0x0E,
			KEYINDEX_GOLDENCARDSH_KEK,aucHexData1,aucHexData1);
	if(ucResult )	return(ucResult);
	UTIL_TestDispStr(aucHexData1,8,(UCHAR *)"Cry Master key hex 23= ",NULL);

	ucResult = PINPAD_47_Encrypt8ByteSingleKey(ucArrayIndex,
					ucKeyIndex+3,aucHexData1,aucHexData1);
	if(ucResult )	return(ucResult);
	ucResult = PINPAD_42_LoadSingleKeyUseSingleKey(ucArrayIndex,
					ucKeyIndex+3,ucKeyIndex+3,aucHexData1);
	if(ucResult )	return(ucResult);
}

unsigned char UTIL_PackTMData(void)
{
		unsigned char aucBuf[512];
		unsigned char aucLeftKeyBuf[32],aucRightKeyBuf[32],aucTempBuf[32];
		unsigned char ucResult,aucTempBuf1[32],aucTempBuf2[32],aucHexData[512];
		unsigned char ucKeyIndex;
		unsigned short uiLen,uiI;
		unsigned short SendMsgID,RecvMsgID;
		short iLen;

		util_Printf("��������:[%02x]\n",G_NORMALTRANS_ucTransType);
#ifdef tCOMMPRINT
		memset(aucBuf,0,sizeof(aucBuf));
		memset(aucBuf,' ',sizeof(aucBuf));
		memcpy(aucBuf,"��������:",9);
		hex_asc(&aucBuf[9],&G_NORMALTRANS_ucTransType,2);
		hex_asc(&aucBuf[13],&ucComm,1);
		PRINT_GB2312_xprint(aucBuf,0x1D);
#endif
		ISO8583_Clear();

		//Set MsgID
		ISO8583_SetMsgID(800);

		//Bit 2
		memset(aucBuf,0,sizeof(aucBuf));
		switch(G_NORMALTRANS_ucTransType)
		{
			case TRANS_OPERATOR:
				util_Printf("����Ա����:");
				for (uiI=0;uiI<16;uiI++)
				{
					util_Printf("%02x",G_RUNDATA_aucOperaPan[uiI]);
				}
				util_Printf("\n");

				memcpy(aucBuf,G_RUNDATA_aucOperaPan,16);
				ucResult = ISO8583_SetBitValue(2,aucBuf,16);
				util_Printf("Bit 2 ucResult = [%02x]\n",ucResult);
				break;
			default:
				break;
		}

		//Bit 3
		memset(aucBuf,0,sizeof(aucBuf));
		switch(G_NORMALTRANS_ucTransType)
		{
			case TRANS_OPERATOR:
				memcpy(aucBuf,"\x95\x00\x00",3);
				ISO8583_SetBitHexValue(3,aucBuf,3);
				break;
			case TRANS_UPTMKEY:
				memcpy(aucBuf,"\x08\x00\x00",3);
				ISO8583_SetBitHexValue(3,aucBuf,3);
				break;
			default:
				break;
		}

		//Bit 11
		memset(aucBuf,0,sizeof(aucBuf));
		long_asc(aucBuf,6,&xDATA_Change.ulTraceNumber);
		ISO8583_SetBitValue(11,aucBuf,6);
		UTIL_IncreaseTraceNumber();

		//Bit 22
		memset(aucBuf,0,sizeof(aucBuf));
		switch(G_NORMALTRANS_ucTransType)
		{
			case TRANS_OPERATOR:
				memcpy(aucBuf,"00",2);
				ISO8583_SetBitValue(22,aucBuf,3);
				break;
			default:
				break;
		}

		//Bit 27
		memset(aucBuf,0,sizeof(aucBuf));
		switch(G_NORMALTRANS_ucTransType)
		{
		case TRANS_OPERATOR:
			memcpy(aucBuf,"04",2);
			ISO8583_SetBitValue(26,aucBuf,2);
			memset(aucBuf,0,sizeof(aucBuf));
			memcpy(aucBuf,"57",2);
			ISO8583_SetBitValue(27,aucBuf,2);
			break;
		case TRANS_UPTMKEY:
			//memcpy(aucBuf,"04",2);
			//ISO8583_SetBitValue(26,aucBuf,2);
			break;
		default:
			break;
		}

	//Bit 41&42&44
	if (xDATA_Constant.aucTerminalID[0]==0x00||xDATA_Constant.aucMerchantID[0]==0x00)
	    {
                Os__light_on();
                //Os__clr_display(255);
                //Os__GB2312_display(0,0,(uchar *)" �̻��Ż��ն˺�");
                //Os__GB2312_display(1,2,(uchar *)" δ����");
                MSG_WaitKey(3);
                return (ERR_END);
	    }
	ISO8583_SetBitValue(41,xDATA_Constant.aucTerminalID,8);
	ISO8583_SetBitValue(42,xDATA_Constant.aucMerchantID,15);

	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,"00001",5);
	ISO8583_SetBitValue(44,aucBuf,5);

	//Bit 47 & 52
	if (G_NORMALTRANS_ucTransType == TRANS_OPERATOR)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,"02",2);
		ISO8583_SetBitValue(47,aucBuf,2);

		memset(aucBuf,0,sizeof(aucBuf));
		memset(aucBuf,' ',sizeof(aucBuf));
		//memcpy(aucBuf,"\x30\x30\x30\x30\x30\x30\x20\x20",8);
		//uiLen= sizeof(G_RUNDATA_aucOperaPanKey)-1;
		memcpy(aucBuf,G_RUNDATA_aucOperaPanKey,6);
		util_Printf("����Ա����:");
		for (uiI=0;uiI<8;uiI++)
		{
			util_Printf("%02x",aucBuf[uiI]);
		}
		util_Printf("\n");

		memset(aucLeftKeyBuf,0,sizeof(aucLeftKeyBuf));
		memset(aucRightKeyBuf,0,sizeof(aucRightKeyBuf));

		memset(aucTempBuf,0,sizeof(aucTempBuf));
		memset(aucTempBuf1,0,sizeof(aucTempBuf1));
		memset(aucTempBuf2,0,sizeof(aucTempBuf2));

		memcpy(aucLeftKeyBuf,"\xC0\xEE\xD3\xF1\xB4\xA8\xD0\xB4",8);
		memcpy(aucRightKeyBuf,"\xB5\xC4\xCE\xA1\xBF\xB5\xBF\xA8",8);

		des(aucBuf,aucTempBuf,aucLeftKeyBuf);
		desm1(aucTempBuf,aucTempBuf1,aucRightKeyBuf);
		des(aucTempBuf1,aucTempBuf2,aucLeftKeyBuf);

		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,aucTempBuf2,8);

		ISO8583_SetBitHexValue(52,aucBuf,8);
	}
	if (G_NORMALTRANS_ucTransType == TRANS_UPTMKEY)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,xDATA_Constant.aucMerchantID,15);
		memcpy(&aucBuf[15],xDATA_Constant.aucTerminalID,8);
		ISO8583_SetBitValue(60,aucBuf,strlen((char*)aucBuf));
	}
    /* Save SendISO to Check Return Data */
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.Trans_8583Data.SendISO8583Data);
	ucResult = XDATA_Write_Vaild_File(DataSaveTrans8583);
	if (ucResult != SUCCESS)
	{
		MSG_DisplayErrMsg(ucResult);
		return(ERR_CANCEL);
	}
	SendMsgID = (unsigned short)bcd_long(ISO8583Data.aucMsgID,ISO8583_MSGIDLEN*2);
	util_Printf("SendMsgID=%d\n",SendMsgID);
	ucResult = ISO8583_PackData(ISO8583Data.aucCommBuf,&ISO8583Data.uiCommBufLen,
					ISO8583_MAXCOMMBUFLEN);
    /* SendReceive with host */
	uiLen = sizeof(ISO8583Data.aucCommBuf);
	ISO8583_DumpData();

	util_Printf("\nISO Len=%d\n",ISO8583Data.uiCommBufLen);
#if 1
	//Start to connect Net & Send Trans Data
	ucResult = COMMS_SendReceiveKEY(ISO8583Data.aucCommBuf,ISO8583Data.uiCommBufLen,
					ISO8583Data.aucCommBuf,&uiLen);
	util_Printf("ucResult 1=[%02x]\n",ucResult);
	if( ucResult != SUCCESS)
	{
		return(ucResult);
	}
	ISO8583Data.uiCommBufLen = uiLen;
	ucResult =  ISO8583_UnpackData(ISO8583Data.aucCommBuf,ISO8583Data.uiCommBufLen);
	util_Printf("\n-++++++++++++++++-Receive-++++++++++++++++-\n");
	ISO8583_DumpData();
	util_Printf("\n-++++++++++++++++-End-++++++++++++++++-\n");
	util_Printf("ucResult 2 =[%02x]\n",ucResult);
	if( ucResult != SUCCESS)
	{
		return(ucResult);
	}
	/* Check response data valid */
	RecvMsgID = (unsigned short)bcd_long(ISO8583Data.aucMsgID,ISO8583_MSGIDLEN*2);
	util_Printf("SID:[%d]\nRID:[%d]\n",SendMsgID,RecvMsgID);
	if (G_NORMALTRANS_ucTransType != TRANS_UPTMKEY)
	{
		if( RecvMsgID != SendMsgID)
		{
			util_Printf("ucResult 3 =[%02x]\n",ucResult);
			return (ERR_UNKNOWNTRANTYPE);
		}
	}
	ucResult = ISO8583_CheckResponseValid();
	util_Printf("CRV:[%02x]\n",ucResult);
	if (ucResult == ERR_HOSTCODE)
	{
	       KEY_COMMS_FinComm();
	       UTIL_Beep();
		memset(aucHexData,0,sizeof(aucHexData));
		memset(aucBuf,0,sizeof(aucBuf));
		iLen=0;
		ucResult = ISO8583_GetBitValue(55,aucHexData,&iLen,sizeof(aucHexData));
		if (!ucResult)
		{
			memcpy(aucBuf,aucHexData,iLen);
            //Os__clr_display(255);
            //Os__GB2312_display(1,0,aucBuf);
			UTIL_WaitGetKey(5);
			return(ERR_END);
		}
	}
	if (!ucResult && G_NORMALTRANS_ucTransType == TRANS_UPTMKEY)
	{
		util_Printf("��Կ��װ�ɹ�\n");
		memset(aucHexData,0,sizeof(aucHexData));
		ucResult = ISO8583_GetBitValue(61,aucHexData,&iLen,sizeof(aucHexData));

		util_Printf("61����������[%d]:\n",iLen);
		for (uiI=0;uiI<iLen;uiI++)
		{
			if (!(uiI+1)%10)
				util_Printf("\n");
			util_Printf("[%02x] ",aucHexData[uiI]);
		}
		util_Printf("\n");

		//memset(aucBuf,0,sizeof(aucBuf));
		//memset(aucKeyBuf,0,sizeof(aucKeyBuf));
		//asc_hex(aucBuf,iLen/2,aucHexData,iLen);
		memcpy(&ucKeyIndex,&DataSave0.ConstantParamData.ucUseKeyIndex,1);
		util_Printf("����:%x\n",ucKeyIndex);
		//util_Printf("asc_hex��������[%d]:\n",iLen);
		//for (uiI=0;uiI<iLen/2;uiI++)
		//{
		//	if (!(uiI+1)%10)
		//		util_Printf("\n");
		//	util_Printf("[%02x] ",aucBuf[uiI]);
		//}
		//util_Printf("\n");
		ucResult = UTIL_LoadMasterKey(ucKeyIndex,aucHexData,iLen);
		if( ucResult != SUCCESS)
		{
			Os__saved_set(&DataSave0.ChangeParamData.ucKeyRight,1,1);
			XDATA_Write_Vaild_File(DataSaveChange);
			return(ERR_CANCEL);
		}
		util_Printf("��Կ��װ�ɹ�\n");
		Os__saved_set(&DataSave0.ChangeParamData.ucKeyRight,0,1);
		XDATA_Write_Vaild_File(DataSaveChange);
	}
#endif
	return (ucResult);
}

unsigned char UTIL_InputOper(void)
{
    //Os__clr_display(255);
    //Os__GB2312_display(0, 0, (uchar * )"���������Ա��:");
	memset(G_RUNDATA_aucOperaPan,0,sizeof(G_RUNDATA_aucOperaPan));
	if( UTIL_Input(1,true,16,16,'N',G_RUNDATA_aucOperaPan,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}

    //Os__clr_display(255);
    //Os__GB2312_display(0, 0, (uchar * )"���������Ա����:");
	memset(G_RUNDATA_aucOperaPanKey,0,sizeof(G_RUNDATA_aucOperaPanKey));
	if( UTIL_Input(1,true,4,6,'P',G_RUNDATA_aucOperaPanKey,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	KEY_COMMS_PreComm();
	return(SUCCESS);
}

unsigned char UTIL_UpDataTM(unsigned char ucFlag)
{
	unsigned char ucResult;

	ucResult = SUCCESS;
	if (ucFlag)
	{
		if (xDATA_Constant.ucKEYCOMM == PARAM_DOWNKEYCOMM_COMM)
		{
            //Os__clr_display(255);
            //Os__GB2312_display(1,0,(uchar *)"�ù��ܲ�֧�ִ���");
            //Os__GB2312_display(2,0,(uchar *)" ��ѡ������ͨѶ");
			UTIL_WaitGetKey(2);
			return(ERR_CANCEL);
		}
		ucResult = UTIL_InputOper();
	}
	if (!ucResult)
	{
		//����Ա������֤
		G_NORMALTRANS_ucTransType = TRANS_OPERATOR;
		ucResult = UTIL_PackTMData();
		util_Printf("����Ա��֤:[%02x]\n",ucResult);
	}
	if (!ucResult)
	{
        //Os__clr_display(255);
        //Os__GB2312_display(0,1,(uchar *)"����Ա����֤");
        //Os__GB2312_display(1,1,(uchar *)"׼����Կ����");
		UTIL_WaitGetKey(2);
	}

	if (!ucResult)
	{
		//��Կ����
		G_NORMALTRANS_ucTransType = TRANS_UPTMKEY;
		ucResult = UTIL_PackTMData();
		util_Printf("��Կ������֤:[%02x]\n",ucResult);
	}
	COMMS_FinComm();
	if (!ucResult)
	{
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(uchar *)"��Կ���سɹ�");
		UTIL_WaitGetKey(3);
	}

	return (ucResult);
}
unsigned char UTIL_UpDataInitCom(void)
{
	unsigned char ucKey,ucResult;

    //Os__clr_display(255);
    //Os__GB2312_display(0,0,(unsigned char *)"��ѡ��ͨѶ���ڣ�");
    //Os__GB2312_display(1,0,(unsigned char *)"[1]---����1 ");
    //Os__GB2312_display(2,0,(unsigned char *)"[2]---����2");

	while(!(ucKey=='1' ||ucKey=='2'||ucKey==KEY_CLEAR))
		ucKey = Os__xget_key();
	ucComm = 0;
	switch(ucKey)
	{
		case '1':
			ucComm = COMM1;
			break;
		case '2':
			ucComm = COMM2;
			break;
		case KEY_CLEAR:
			return ERR_CANCEL;
		default:
			break;
	}
	util_Printf("����:(%02x)\n",ucComm);
	if (ucComm == COMM1)
	{
            //Os__clr_display(255);
            //Os__GB2312_display(1,0,(unsigned char *)" �ݲ�֧�ִ���һ");
            //Os__GB2312_display(2,0,(unsigned char *)" ��ѡ������ͨѶ");
        	UTIL_GetKey(5);
        	return (ERR_CANCEL);
	}
	else if (ucComm == COMM2)
	{
		Os__end_com3();
		Os__xdelay(10);
		if( !OSUART_Init2(0x0303,0x0C00,0x0C))	//COM2 speed:9600
		{
			ucResult = SUCCESS;
		}
		else
		{
			ucResult = ERR_COMMINIT;
		}
	}
    	if (!ucResult && DataSave0.ConstantParamData.ucKEYCOMM != PARAM_DOWNKEYCOMM_COMM)
    	{
            util_Printf("\n����ͨѶ\n");
            DataSave0.ConstantParamData.ucKEYCOMM  = PARAM_DOWNKEYCOMM_COMM;
            XDATA_Write_Vaild_File(DataSaveConstant);
    	}
	return (ucResult);
}
unsigned char UTIL_ReceCoMM(unsigned char *pucByte,unsigned int puiTimeout)
{
	unsigned short usResult;
	//unsigned char aucASCBuf[10];

	usResult = Os__rxcar3(puiTimeout);
//	memcpy(aucASCBuf,0,sizeof(aucASCBuf));
//	hex_asc(aucASCBuf,usResult/256,2);
//	PRINT_GB2312_xprint(aucASCBuf,0x1D);
	switch ( usResult/256 )
	{
		case OK :
			*pucByte = usResult % 256;
			return (OK);
		case COM_ERR_TO:
			return (ERR_COMMS_RECVTIMEOUT);
		default :
			return (ERR_COMMS_RECVCHAR);
	}
	return (ERR_COMMS_RECVCHAR);
}
unsigned char UTIL_ReadyAns(unsigned char ucFlag)
{
	unsigned char ucResult,aucBuf[32],ucLRC;
	unsigned short uiLen,uiSendLen;
	unsigned int uiTimeout,uiI;
	unsigned long ulLen;
	unsigned char aucReceBuf[32],ucChar;
	unsigned char aucTimeBuf[32];

	memset(aucBuf,0,sizeof(aucBuf));
	memset(aucReceBuf,0,sizeof(aucReceBuf));
	uiLen=0;
	uiSendLen = 0;
	ucLRC=0;
	aucBuf[0]=0x02;
	if (ucFlag)
	{
		uiLen = 24;
		uiSendLen = uiLen+3;
		short_bcd(&aucBuf[1],2,&uiLen);
		memcpy(&aucBuf[3],"CONNECTOVER",11);
		memcpy(&aucBuf[14],&DataSave0.ConstantParamData.ulHostKEYIP,4);
		memcpy(&aucBuf[18],&DataSave0.ConstantParamData.uiHostKEYPort,6);
		for(uiI=1;uiI<uiLen;uiI++)
		{
			ucLRC ^= aucBuf[uiI];
		}
		aucBuf[11] =0x03;
		memcpy(&aucBuf[12],&ucLRC,1);
	}
	else
	{
		uiLen = 8;
		uiSendLen = uiLen+3;
		short_bcd(&aucBuf[1],2,&uiLen);
		memcpy(&aucBuf[3],"AREYOUOK",8);
		for(uiI=1;uiI<=uiLen+2;uiI++)
		{
		    ucLRC ^= aucBuf[uiI];
		}
		aucBuf[11] =0x03;
		memcpy(&aucBuf[12],&ucLRC,1);
	}
	memcpy(aucReceBuf,"YESIAMOK",8);

#ifdef tCOMMPRINT
	PRINT_GB2312_xprint((unsigned char*)"����ARE U OK",0x1d);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"������:",7);
	hex_asc(&aucPrintBuf[7],aucBuf,(uiSendLen+2)*2);
	PRINT_GB2312_xprint(aucPrintBuf,0x1D);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"LRC:",4);
	hex_asc(&aucPrintBuf[4],&ucLRC,2);
	PRINT_GB2312_xprint(aucPrintBuf,0x1D);
#endif
    //Os__clr_display(255);
    //Os__GB2312_display(0, 0, (uchar *) "���ڳ�ʼ���ɹ�");
    //Os__GB2312_display(1, 0, (uchar *) "���ݷ�����...");
    //Os__GB2312_display(2, 0, (uchar *) "���Ժ�");

	for(uiI=0;uiI<uiSendLen+2;uiI++)
	{
		Os__txcar3(aucBuf[uiI]);
	}

    //Os__clr_display(255);
    //Os__GB2312_display(0, 0, (uchar *) "���ݷ��ͳɹ�");
    //Os__GB2312_display(1, 0, (uchar *) "���ڽ��շ���...");

	uiTimeout = 30*100;
	Os__timer_start(&uiTimeout);
	memset(aucTimeBuf,0x00,sizeof(aucTimeBuf));
	memcpy(aucTimeBuf,"����ʹ��    ��",14);
    //Os__GB2312_display(3,0,(unsigned char*)"   (������)");
        while(1)
        {
            uiI = 30-uiTimeout/100;
            int_asc(aucTimeBuf+9,2,&uiI);
            //Os__GB2312_display(2,0,aucTimeBuf);
            ucResult = UTIL_ReceCoMM(&ucChar,100);
            if(!ucResult)
            {
                break;
            }
            if(!uiTimeout)
                break;
        }
        Os__timer_stop(&uiTimeout);
        if(!uiTimeout)
        {
            Os__end_com3();
            return ERR_COMMS_RECVTIMEOUT;
        }
        if (!ucResult)
        {
            while(ucResult == OK)
            {
                if(ucChar == 0x02)
                {
                    break;
                }
                else
                {
                    Os__xdelay(50);
                    ucResult = UTIL_ReceCoMM(&ucChar,10);
                }
            }
            memset(aucBuf,0,sizeof(aucBuf));
            aucBuf[0] = ucChar;
            ucLRC=0;
            for (uiI=0;uiI<2;uiI++)
            {
                if (!(ucResult = UTIL_ReceCoMM(&ucChar,10)))
                {
                    aucBuf[uiI+1]=ucChar;
                    ucLRC ^=ucChar;
                }
                else
                {
                    return ucResult;
                }
            }
            ulLen = bcd_long(aucBuf,4);
#ifdef tCOMMPRINT
            memset(aucPrintBuf,0,sizeof(aucPrintBuf));
            memcpy(aucPrintBuf,aucBuf,3);
#endif

            memset(aucBuf,0,sizeof(aucBuf));
            for (uiI=0;uiI<ulLen;uiI++)
            {
                if (!(ucResult = UTIL_ReceCoMM(&ucChar,10)))
                {
                    aucBuf[uiI]=ucChar;
                    ucLRC ^=ucChar;
                }
                else
                {
                    return ucResult;
                }
            }

#ifdef tCOMMPRINT
        memcpy(&aucPrintBuf[3],aucBuf,ulLen);
#endif
            for (uiI=0;uiI<2;uiI++)
            {
                if (!(ucResult = UTIL_ReceCoMM(&ucChar,10)))
                {
#ifdef tCOMMPRINT
                    memcpy(&aucPrintBuf[ulLen+3+uiI+1],&ucChar,1);
#endif
                    if (uiI ==0 && ucChar != 0x03)
                        {
                            return ERR_COMMS_PROTOCOL;
                        }
                    if (uiI==1 && ucLRC != ucChar)
                        {
                            return ERR_COMMS_LRC;
                        }
                }
                else
                {
                    return ucResult;
                }
            }
        }
#ifdef tCOMMPRINT
        PRINT_GB2312_xprint((unsigned char*)"����ARE U OK",0x1d);
        PRINT_GB2312_xprint(aucPrintBuf,0x1D);
#endif
    //Os__clr_display(255);
    //Os__GB2312_display(0, 0, (uchar *) "���ݽ��ճɹ�");
    //Os__GB2312_display(1, 0, (uchar *) "���ڴ���...");
	Os__xdelay(2);
	if (memcmp(aucReceBuf,aucBuf,8))
		return (ERR_CANCEL);
        return (SUCCESS);
}
unsigned char UTIL_UpDataMF(void)
{
	unsigned char ucResult;

	ucResult = UTIL_UpDataInitCom();
	util_Printf("����ģ��L1:(%02x)\n",ucResult);


#ifdef tCOMMPRINT
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,"��ʼ�����ڽ��:",15);
	hex_asc(&aucBuf[15],&ucResult,2);
	PRINT_GB2312_xprint(aucBuf,0x1D);
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,"ͨѶ��ʽ:",9);
	hex_asc(&aucBuf[9],&ucComm,1);
	PRINT_GB2312_xprint(aucBuf,0x1D);
#endif


	if (!ucResult)
	{
		if (ucComm == COMM1)
		{
			ucResult = UTIL_UpDataTM(0);
		}
		else if (ucComm == COMM2)
		{
		    ucResult = UTIL_InputOper();
		    if (!ucResult)
			ucResult = UTIL_ReadyAns(0);

			util_Printf("ARE U OK 0:[%02x]\n",ucResult);
			if (!ucResult)
				ucResult = UTIL_UpDataTM(0);
			util_Printf("UpDataTM:[%02x]\n",ucResult);
			if (!ucResult)
				ucResult = UTIL_ReadyAns(1);
			util_Printf("ARE U OK 1:[%02x]\n",ucResult);
		}
	}
	util_Printf("����ģ��L2:(%02x)\n",ucResult);
	return (ucResult);
}
unsigned char UTIL_UpDataKey(void)
{
	unsigned char ucKey,ucResult;
	unsigned char aucBuf[10],ucFlag;

    //Os__clr_display(255);
    //Os__GB2312_display(0,0,(UCHAR*)"��������Կ����Ա");
    //Os__GB2312_display(1,0,(UCHAR*)"����:");

	memset(aucBuf,0,sizeof(aucBuf));
	if( UTIL_Input(2,true,6,6,'P',aucBuf,NULL) != KEY_ENTER )
	{
    	    return(ERR_CANCEL);
	}
	if(memcmp(aucBuf,"372819",6))
	{
		return(MSG_DisplayErrMsg(ERR_SUPERKEY));
	}
#if PS100
    //Os__clr_display(255);
    //Os__GB2312_display(0,0,(UCHAR*)" ��ȷ��PIN����");
    //Os__GB2312_display(1,0,(UCHAR*)"����Կ�洢PIN��");
    //Os__GB2312_display(3,0,(unsigned char *)"[ȷ��]    [ȡ��]");
	while(1)
	{
		ucKey =Os__xget_key();
		if (ucKey==KEY_ENTER)
		{
			DataSave0.ConstantParamData.ucKEYMODE = 0x31;
			XDATA_Write_Vaild_File(DataSaveConstant);
			break;
		}
		else if (ucKey == KEY_CLEAR)
			return(ERR_CANCEL);
	}
#endif

	while(1)
	{
            //Os__clr_display(255);
            //Os__GB2312_display(0,1,(UCHAR*)"Զ����Կ����");
            //Os__GB2312_display(1,0,(UCHAR*)"1.��������");
            //Os__GB2312_display(2,0,(UCHAR*)"2.Զ������");

		ucKey = UTIL_GetKey(30);
		ucFlag = 0;

		switch(ucKey)
		{
		       case '1':
		            ucResult = SEL_SETKEYPARA();
		            ucFlag = 1;
		            break;
			case '2':
				G_NORMALTRANS_euCardSpecies = 0xFF;
				ucResult = UTIL_UpDataTM(1);
				KEY_COMMS_FinComm();
				break;
//			case '3':
//				G_NORMALTRANS_euCardSpecies = 0xFF;
//				ucResult = UTIL_UpDataMF();
//				break;
		       case 99:
			case KEY_CLEAR:
				break;
			default:
				continue;
		}
		if (!ucFlag)
		    break;
	}

	return(ucResult);
}
unsigned char UTIL_Set_BankID(void)
{
	unsigned char ucResult=SUCCESS;
	unsigned char aucDynamicPSW[8],aucBuf[7];
	SELMENU ManagementMenu;
	memset(&ManagementMenu,0,sizeof(SELMENU));

	UTIL_GetMenu_Value(NULL,	BANK_TRAFFICBANK,		NULL,	UTIL_BANK_Process,&ManagementMenu);
	UTIL_GetMenu_Value(NULL,	BANK_CONSTRUCT, 		NULL,	UTIL_BANK_Process,&ManagementMenu);
	UTIL_GetMenu_Value(NULL,	BANK_INDUSCOMMERCE, 	NULL,	UTIL_BANK_Process,&ManagementMenu);
	UTIL_GetMenu_Value(NULL,	BANK_XINGYE,			NULL,	UTIL_BANK_Process,&ManagementMenu);
	UTIL_GetMenu_Value(NULL,	BANK_HUAXIA,			NULL,	UTIL_BANK_Process,&ManagementMenu);
	UTIL_GetMenu_Value(NULL,    BANK_AGRICULTURE,		NULL,   	UTIL_BANK_Process,&ManagementMenu);
	UTIL_GetMenu_Value(NULL,    BANK_SHENGDEVELOP,		NULL,   	UTIL_BANK_Process,&ManagementMenu);
	UTIL_GetMenu_Value(NULL,    BANK_GUANGDA,      		NULL,   	UTIL_BANK_Process,&ManagementMenu);
	UTIL_GetMenu_Value(NULL,    BANK_OTHERS,      		NULL,   	UTIL_BANK_OTHERS,&ManagementMenu);

	ucResult = SUCCESS;

	if((DataSave0.TransInfoData.ForeignTransTotal.uiTotalNb != 0)
		  ||(DataSave0.TransInfoData.TransTotal.uiTotalNb != 0))
	{
        //Os__clr_display(255);
        //Os__GB2312_display(1,0,(uchar *)"   �ն��н���");
        //Os__GB2312_display(2,0,(uchar *)"  ������������");
		UTIL_Beep();
		MSG_WaitKey(50);
		ucResult = ERR_CANCEL;
	}
	if(DataSave0.ChangeParamData.ucCashierLogonFlag==CASH_LOGONFLAG)
	{
        //Os__clr_display(255);
        //Os__GB2312_display(1,0,(uchar *)"   ��Ա��ǩ��");
        //Os__GB2312_display(2,0,(uchar *)"  ������������");
		UTIL_Beep();
		MSG_WaitKey(50);
		ucResult = ERR_CANCEL;
	}
	/*========================================*/
	if(!ucResult)
	{
		memset(aucDynamicPSW,0x00,sizeof(aucDynamicPSW));
		UTIL_GetDynamicPSW(aucDynamicPSW);

		util_Printf("\n��̬����:%s\n",aucDynamicPSW);
		memset(aucBuf,0x00,sizeof(aucBuf));
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(UCHAR*)"������ϵͳ����:");
		if (UTIL_Input(1,true,6,6,'P',aucBuf,0) != KEY_ENTER)
		{
			ucResult = ERR_CANCEL;
		}else
		{
			if( memcmp(aucBuf,aucDynamicPSW,6) )
				ucResult = ERR_CASH_PASS;
		}
	}
	/*===========================================*/
	 if((ucResult == SUCCESS)
	 	&&(DataSave0.ChangeParamData.ucCashierLogonFlag!=CASH_LOGONFLAG) )
	 {
	 	SetBankflag = 1;
        	cflag=0;
	    	ucResult = UTIL_DisplayMenu(&ManagementMenu);
		SetBankflag = 0;
	       if( ucResult != SUCCESS&&ucResult != ERR_CANCEL&&ucResult != ERR_END)
	       {
	            MSG_DisplayErrMsg(ucResult);
	       }
	 }
   if( ucResult==ERR_CASH_PASS)
   {
      MSG_DisplayErrMsg(ucResult);
   }
    return(ucResult);
}

void UTIL_AddTraceNumber(void)
{
	ULONG_C51 ulTraceNumber;

	DataSave0.ChangeParamData.ulTraceNumber += 50;
	ConstParam.uiTraceNumber += 50;
	TransReqInfo.uiTraceNumber = ConstParam.uiTraceNumber ;
	ulTraceNumber = DataSave0.ChangeParamData.ulTraceNumber;
	G_NORMALTRANS_ulTraceNumber = ulTraceNumber;
    	ulTraceNumber ++;
	if( ulTraceNumber > 999999 )
	{
		ulTraceNumber = 1;
	}
	ConstParam.uiTraceNumber = ulTraceNumber;
	Os__saved_copy(	(unsigned char *)&ulTraceNumber,
			(unsigned char *)&DataSave0.ChangeParamData.ulTraceNumber,
			sizeof(ULONG_C51));

	UTIL_WriteConstParamFile(&ConstParam);
	XDATA_Write_Vaild_File(DataSaveChange);
}


unsigned char UITL_SetAutoSettleTime(void)
{
	unsigned char aucSetSettleTime[5],ucBuf[6];
	unsigned short uiHours,uiMinte,uiTime;
	int i;

	util_Printf("��ʱʱ��:\n");
	for(i=0;i<4;i++)
		util_Printf("%02x ",DataSave0.ChangeParamData.aucSettleCycle[i]);
	util_Printf("\n%s\n",DataSave0.ChangeParamData.aucSettleCycle);
	CYcle:
	memset(aucSetSettleTime,0,sizeof(aucSetSettleTime));
	memset(ucBuf,0,sizeof(ucBuf));
	memcpy( ucBuf, &DataSave0.ChangeParamData.aucSettleCycle[0], 2);
	memcpy( &ucBuf[2], (uchar *)":", 1);
	memcpy( &ucBuf[3], &DataSave0.ChangeParamData.aucSettleCycle[2], 2);
    //Os__clr_display(255);
    //Os__GB2312_display(0,0,(uchar *)"�����ս���ʱ��:");
    //Os__GB2312_display(1,0,(uchar *)"ʱ��(HHMM)");
    //Os__GB2312_display(3,0,(uchar *)" ��ǰ:");
    //Os__GB2312_display(3,4,(uchar *)ucBuf);
	if( UTIL_Input(2,true,0,CASH_MAXTime,'N',aucSetSettleTime,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	if(strlen((char *)aucSetSettleTime)==0)
		return(SUCCESS);
	if(strlen((char *)aucSetSettleTime)<4)
		goto CYcle;
	uiHours=(unsigned short)asc_long(aucSetSettleTime, 2);
	uiMinte=(unsigned short)asc_long(&aucSetSettleTime[2],2);
	util_Printf("H=%d,M=%d\n",uiHours,uiMinte);
/*
	if((uiHours>23)||(uiMinte>59))
	{
        //Os__clr_display(255);
        //Os__GB2312_display(1,0,(uchar*)"  ʱ���ʽ����");
        //Os__GB2312_display(2,0,(uchar*)"��[ȷ��]��������");
		ucResult=MSG_WaitKey(30);
		if(ucResult==KEY_ENTER)
		{
			goto CYcle;
		}
		else
		{
			return(ERR_CANCEL);
		}
	}
	else
*/
	{
		memcpy(DataSave0.ChangeParamData.aucSettleCycle,aucSetSettleTime,4);
		XDATA_Write_Vaild_File(DataSaveChange);
		util_Printf("ASC-Cycle=%s\n",&DataSave0.ChangeParamData.aucSettleCycle);
		uiTime=(unsigned short)asc_long(DataSave0.ChangeParamData.aucSettleCycle, 4);
		util_Printf("INT--Cycle=%d\n",uiTime);
	}
	return(SUCCESS);
}


void UTIL_AutoSettleOn(void)
{
	unsigned char DispBuf[17];
	unsigned char aucBuf[11],aucTime[4],aucAreaTime[4];
	unsigned int uiHours,uiMinte,uiSHour,uiSMinte;

	memset(aucBuf,0,sizeof(aucBuf));
	memset(DispBuf,0,sizeof(DispBuf));

	UTIL_READ_DateTimeAndFormat(aucBuf);/*YYMMDDhhmm*/

	memcpy(DispBuf,"20",2);
	memcpy(&DispBuf[2],aucBuf,2);//YY
	memcpy(&DispBuf[4],&aucBuf[2],2);//MM
	memcpy(&DispBuf[6],&aucBuf[4],2);//DD
	memcpy(&DispBuf[8],&aucBuf[6],2);//hh
	memcpy(&DispBuf[10],&aucBuf[8],2);//mm

	util_Printf("aucBuf===%s\n",&aucBuf);
	util_Printf("DispBuf=%s\n",&DispBuf[8]);
	util_Printf("aucSettleCycle=%s\n",&DataSave0.ChangeParamData.aucSettleCycle);

	memset(aucTime,0,sizeof(aucTime));
	memcpy(aucTime,DataSave0.ChangeParamData.aucSettleCycle,4);
	uiHours=(unsigned short)asc_long(aucTime, 2);
	uiMinte=(unsigned short)asc_long(&aucTime[2],2);

	uiSHour=(unsigned short)asc_long(&DispBuf[8], 2);
	uiSMinte=(unsigned short)asc_long(&DispBuf[10],2);
	util_Printf("H=%d,M=%d,sH=%d,sM=%d\n",uiHours,uiMinte,uiSHour,uiSMinte);

	uiMinte = uiMinte+10;
	util_Printf("H33=%d,M44=%d\n",uiHours,uiMinte);
	if (uiMinte>59)
	{
		uiHours++;
		uiMinte = uiMinte-60;
	}

	memset(aucAreaTime,0,sizeof(aucAreaTime));
	int_asc(aucAreaTime,2,&uiHours);
	int_asc(&aucAreaTime[2],2,&uiMinte);

	util_Printf("�趨ʱ��:(ʱ)=%d,(��)=%d\n",uiHours,uiMinte);
	util_Printf("����ʱ��:(ʱ)=%d,(��)=%d [%s]\n",uiSHour,uiSMinte,&aucAreaTime);
	{
		int i;
		util_Printf("����ʱ��:\n");
		for (i=0;i<4;i++)
		{
			util_Printf("%02x ",aucAreaTime[i]);
		}
		util_Printf("\n");
	}
	if((memcmp(&DispBuf[8],DataSave0.ChangeParamData.aucSettleCycle,4))>=0 && (memcmp(&DispBuf[8],aucAreaTime,4)<=0))
	{
		util_Printf("��ʱ����:\n");
		if ( ((DataSave0.TransInfoData.ForeignTransTotal.uiTotalNb != 0)||(DataSave0.TransInfoData.TransTotal.uiTotalNb != 0))
			&& DataSave0.ChangeParamData.ucCashierLogonFlag == CASH_LOGONFLAG)
		{
			SERV_Settle(true);
		}
	}
}


UCHAR UTIL_SaveReaderCAPKFile(void)
{
	UCHAR	ucResult=SUCCESS;
	UCHAR	ucI,ucJ;

	ucResult = INTERFACE_ClearReaderAllPublicPK(); //ɾ���������������й�Կ
	util_Printf("INTERFACE_ClearReaderAllPublicPK-abc-Print--ucResult=%02x\n",ucResult);
	for(ucI=0;!ucResult && ucI<ucTermCAPKNum;ucI++)
	{
		if(/*(memcmp(TermVISAPK[ucI].aucRID,"\xA0\x00\x00\x00\x03",5)==0)  //ֻ֧��VISA��QPBOC�Ĺ�Կ
		||*/( memcmp(TermVISAPK[ucI].aucRID,"\xA0\x00\x00\x03\x33",5)==0))
		{
#if 1
			util_Printf("\n Send To Reader RID:");
			for(ucJ =0;ucJ<5;ucJ++)
				util_Printf("%02x ",TermVISAPK[ucI].aucRID[ucJ]);
#endif
			ucResult = INTERFACE_SetPublicPKToReader(1,1,&TermVISAPK[ucI]);
			util_Printf("INTERFACE_SetPublicPKToReader-abc-ucResult=%02x\n",ucResult);

		}
	}
	return ucResult;

}
UCHAR UTIL_ManualSaveReaderCAPKFile(void)
{
	UCHAR	ucResult=SUCCESS;
	UCHAR	ucI,ucJ;
	VISAPK 	VISA_Load; //����һ���ǽӽṹ��WangAn Add 20090806

	ucResult = INTERFACE_ClearReaderAllPublicPK(); //ɾ���������������й�Կ
	util_Printf("�����������ԿINTERFACE_ClearReaderAllPublicPK-ucResult=%02x\n",ucResult);

	for(ucI=0;!ucResult && ucI<ucTermCAPKNum;ucI++)
	{
		if(/*(memcmp(TermCAPK[ucI].aucRID,"\xA0\x00\x00\x00\x03",5)==0)  //ֻ֧��VISA��QPBOC�Ĺ�Կ
		||*/( memcmp(TermCAPK[ucI].aucRID,"\xA0\x00\x00\x03\x33",5)==0))
		{
			//WangAn Add 20090806

      memset(&VISA_Load,0x00,sizeof(VISAPK));
            //��������POS�Ͷ������ṹ�岻һ�� //WangAn Add 2009-8-14 10:54
      memcpy(VISA_Load.aucRID,TermCAPK[ucI].aucRID,sizeof(VISA_Load.aucRID));
			VISA_Load.ucIndex=TermCAPK[ucI].ucCAPKI;   			   //��֤���Ĺ�Կ����
			VISA_Load.ucHashInd=TermCAPK[ucI].ucHashInd;
			VISA_Load.ucArithInd=TermCAPK[ucI].ucArithInd;
			VISA_Load.ucExponentLen = TermCAPK[ucI].ucExponentLen; //��Կָ��
			memcpy(VISA_Load.aucExponent,TermCAPK[ucI].aucExponent,TermCAPK[ucI].ucExponentLen);

			VISA_Load.ucModulLen = TermCAPK[ucI].ucModulLen;
			memcpy(VISA_Load.aucModul,TermCAPK[ucI].aucModul,VISA_Load.ucModulLen);
			memcpy(VISA_Load.aucCheckSum,TermCAPK[ucI].aucCheckSum,sizeof(TermCAPK[ucI].aucCheckSum));

#if 1
			util_Printf("\nSend To Reader RID:");
			for(ucJ =0;ucJ<5;ucJ++)
				util_Printf("%02x ",VISA_Load.aucRID[ucJ]);

			util_Printf("\nVISA_Load.ucExponentLen=%d\n",VISA_Load.ucExponentLen);
			for(ucJ =0;ucJ<VISA_Load.ucExponentLen;ucJ++)
				util_Printf("%02x ",VISA_Load.aucExponent[ucJ]);
			util_Printf("\nVISA_Load.ucIndex=%02x",VISA_Load.ucIndex);
			util_Printf("\nVISA_Load.ucHashInd=%02x",VISA_Load.ucHashInd);
			util_Printf("\nVISA_Load.ucArithInd=%02x",VISA_Load.ucArithInd);

			util_Printf("\nVISA_Load.ucModulLen=%d",VISA_Load.ucModulLen);
			util_Printf("\nVISA_Load.aucModul:");
			for(ucJ =0;ucJ<VISA_Load.ucModulLen;ucJ++)
				util_Printf("%02x ",VISA_Load.aucModul[ucJ]);
			util_Printf("\nVISA_Load.aucCheckSum Len=%d",sizeof(TermCAPK[ucI].aucCheckSum));
			util_Printf("\n");
			for(ucJ =0;ucJ<sizeof(TermCAPK[ucI].aucCheckSum);ucJ++)
				util_Printf("%02x ",VISA_Load.aucCheckSum[ucJ]);
#endif
			ucResult = INTERFACE_SetPublicPKToReader(1,1,&VISA_Load);
			util_Printf("\nINTERFACE_SetPublicPKToReader--ucResult=%02x\n",ucResult);
		}
	}
	return ucResult;
}
unsigned char UTIL_SetReader(void)
{
	unsigned char ucChar,ucResult=SUCCESS;

	if( ucResult == SUCCESS)
  {
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(uchar *)"ѡ��Ӧ�����");
        //if(ReaderSupport)
            //Os__GB2312_display(1,0,(uchar *)"��ǰѡ��:PBOC");
        //else
            //Os__GB2312_display(1,0,(uchar *)"��ǰѡ��:EMV");
        //Os__GB2312_display(3,0,(uchar *)"1.PBOC     2.EMV");
		while(1)
		{
			ucChar=Os__xget_key();
			if(ucChar=='1')
			{
				ReaderSupport = true;
				XDATA_Write_Vaild_File(DataSaveChange);
				break;
			}
			else if(ucChar=='2')
			{
				ReaderSupport = false;
				XDATA_Write_Vaild_File(DataSaveChange);
				break;
			}else if(ucChar==KEY_CLEAR)
			{
				return ERR_CANCEL;
			}else if(ucChar==KEY_ENTER)
			{
				break;
			}
			else
				continue;
		}
  }
  if(!ReaderSupport) return SUCCESS;
	if( ucResult == SUCCESS)
  {
#if 0
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(uchar *)"����������");
		if(ReaderType==READER_HONGBAO)
            //Os__GB2312_display(1,0,(uchar *)"��ǰѡ��:HONGBAO");
		else
		if(ReaderType==READER_SAND)
            //Os__GB2312_display(1,0,(uchar *)"��ǰѡ��:SAND");
		else
            //Os__GB2312_display(1,0,(uchar *)"��ǰѡ��:δ����");
		#if SANDREADER
        //Os__GB2312_display(3,0,(uchar *)"1.HONGBAO 2.SAND");
		#else
        //Os__GB2312_display(3,0,(uchar *)"1.HONGBAO");
#endif
		#endif
		while(1)
		{
			//ucChar=Os__xget_key();
			ucChar='2';
			if(ucChar=='1')
			{
				ReaderType =READER_HONGBAO;
				ucResult = File_CheckExistFile((unsigned char*)INTERFACEBASICCONFIGDATA,sizeof(BASICCONFIG));
				util_Printf("\nFile_CheckExistFile=HONGBAO1=%d",ucResult);
				if( ucResult == SUCCESS )
					ucResult = File_CheckExistFile((unsigned char*)INTERFACEKEYDATA,sizeof(INTERFACEKEYFILE));

				if( ucResult == SUCCESS )
					OnEve_INTERFACE_Init();
				util_Printf("\nFile_CheckExistFile=HONGBAO2=%d",ucResult);
				XDATA_Write_Vaild_File(DataSaveChange);
				break;
			}
			#if SANDREADER
			else if(ucChar=='2')
			{
				ReaderType =READER_SAND;
				XDATA_Write_Vaild_File(DataSaveChange);
				break;
			}
			#endif
			else if(ucChar==KEY_CLEAR)
			{
				return ERR_CANCEL;
			}else if(ucChar==KEY_ENTER)
			{
				if((ReaderType==READER_HONGBAO)||(ReaderType==READER_SAND))
					break;
			}
			else
				continue;
		}
  }
    if(ucResult==SUCCESS)
    {
        if(ReaderType==READER_HONGBAO)
        {
            ucResult =SEL_ReaderMenu_HongBao();
        }
    }
    return(SUCCESS);
}
unsigned char UTIL_InPutMobileNum(void)
{
	MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
	if(DataSave0.ConstantParamData.Pinpadflag ==1)
	{
        //Os__GB2312_display(1, 0, (uchar * )"��ͻ����������");
        //Os__GB2312_display(2, 0, (uchar * )"�������ֻ�����  ");
		while(1)
		{
			memset(G_NORMALTRANS_aucMobileNUM,0,sizeof(G_NORMALTRANS_aucMobileNUM));
			Os__clr_display_pp(255);
			Os__GB2312_display_pp(0, 0, (uchar * )"�������ֻ�����:");
			if( UTIL_Input_pp(1,true,11,11,'N',G_NORMALTRANS_aucMobileNUM) != KEY_ENTER)
				return(ERR_CANCEL);

			util_Printf("�ֻ�����Len:=%d\n",strlen((char*)G_NORMALTRANS_aucMobileNUM));
			if (strlen((char*)G_NORMALTRANS_aucMobileNUM))
				break;
			else
			{
				Os__clr_display_pp(255);
				Os__GB2312_display_pp(0, 0, (uchar * )"���벻��Ϊ��");
				UTIL_GetKey_pp(3);
			}
		}

		MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
        //Os__GB2312_display(1, 0, (uchar * )"��ͻ����������");
        //Os__GB2312_display(2, 0, (uchar * )"������ԤԼ����");
		while(1)
		{
			Os__clr_display_pp(255);
			Os__GB2312_display_pp(0, 0, (uchar * )"������ԤԼ��:");
			memset(G_NORMALTRANS_aucMobileCHK,0,sizeof(G_NORMALTRANS_aucMobileCHK));
			if( UTIL_Input_pp(1,true,6,6,'N',G_NORMALTRANS_aucMobileCHK) != KEY_ENTER)
				return(ERR_CANCEL);

			util_Printf("�ֻ�ԤԼ��Len:=%d,=%s\n",strlen((char*)G_NORMALTRANS_aucMobileCHK),G_NORMALTRANS_aucMobileCHK);
			if (strlen((char*)G_NORMALTRANS_aucMobileCHK))
				break;
			else
			{
				Os__clr_display_pp(255);
				Os__GB2312_display_pp(0, 0, (uchar * )"ԤԼ�벻��Ϊ��");

				UTIL_GetKey_pp(3);
			}
		}
	}
	else
	{
        //Os__GB2312_display(2, 0, (uchar * )"�������ֻ�����:");
		if( UTIL_Input(3,true,11,11,'N',G_NORMALTRANS_aucMobileNUM,NULL) != KEY_ENTER )
			return(ERR_CANCEL);
		MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
        //Os__GB2312_display(2, 0, (uchar * )"������ԤԼ��:");
		memset(G_NORMALTRANS_aucMobileCHK,0,sizeof(G_NORMALTRANS_aucMobileCHK));

		if( UTIL_Input(3,true,6,6,'N',G_NORMALTRANS_aucMobileCHK,NULL) != KEY_ENTER )
			return(ERR_CANCEL);
		util_Printf("ԤԼ�� =%s\n",G_NORMALTRANS_aucMobileCHK);
	}
	return(SUCCESS);
}
unsigned char UTIL_DispMerchant(unsigned char ucRow,unsigned char ucTag,unsigned char ucMax)
{
	unsigned char ucResult = SUCCESS;
	unsigned char aucDispBuf_1[20],aucDispBuf_2[20],aucDispBuf_3[20],aucDispBuf_4[20];
	
    //Os__clr_display(255);
	if(ucRow!=0)
        //Os__GB2312_display(0,0,(unsigned char *)"��ѡ���̻���:");
		
	memset(aucDispBuf_1,0,sizeof(aucDispBuf_1));
	memset(aucDispBuf_2,0,sizeof(aucDispBuf_2));
	memset(aucDispBuf_3,0,sizeof(aucDispBuf_3));
	memset(aucDispBuf_4,0,sizeof(aucDispBuf_4));
	
	if(ucMax>0)
		memcpy( &aucDispBuf_1[2],RunData.aucMerchantName_cust[ucTag],14);
	if(ucMax>1)
		memcpy( &aucDispBuf_2[2],RunData.aucMerchantName_cust[ucTag+1],14);
	if(ucMax>2)
		memcpy( &aucDispBuf_3[2],RunData.aucMerchantName_cust[ucTag+2],14);
	if(ucMax>3)
		memcpy( &aucDispBuf_4[2],RunData.aucMerchantName_cust[ucTag+3],14);
	util_Printf("\nuctag:%02x,ucMax:%02x\n",ucTag,ucMax);	
	UTIL_Tiaoshi(aucDispBuf_1,20);		
	if((strlen((char *)&aucDispBuf_1[2]))&&(ucMax>0))
	{
		aucDispBuf_1[0] = '1'+ucTag;
		aucDispBuf_1[1] = '.';
        //Os__GB2312_display(ucRow,0,aucDispBuf_1);
		
	}
	UTIL_Tiaoshi(aucDispBuf_2,20);
	if((strlen((char *)&aucDispBuf_2[2]))&&(ucMax>1))
	{
		aucDispBuf_2[0] = '1'+ucTag+1;
		aucDispBuf_2[1] = '.';		
        //Os__GB2312_display(ucRow+1,0,aucDispBuf_2);
		
	}
	UTIL_Tiaoshi(aucDispBuf_3,20);
	if((strlen((char *)&aucDispBuf_3[2]))&&(ucMax>2))
	{
		aucDispBuf_3[0] = '1'+ucTag+2;
		aucDispBuf_3[1] = '.';		
        //Os__GB2312_display(ucRow+2,0,aucDispBuf_3);
		
	}
	UTIL_Tiaoshi(aucDispBuf_4,20);
	if((strlen((char *)&aucDispBuf_4[2]))&&(ucMax>3))
	{
		aucDispBuf_4[0] = '1'+ucTag+3;
		aucDispBuf_4[1] = '.';		
        //Os__GB2312_display(ucRow+3,0,aucDispBuf_4);
		
	}				
	return ucResult;
}


unsigned char UTIL_InputMerchant_Cust(unsigned char *pOut)
{
	unsigned char ucResult = SUCCESS;
	unsigned char ucIndex=0,ucTotal=0;
	unsigned char ucKey = 0x00,ucF4Times=0;
	unsigned char ucDone=0;

	memset(RunData.aucMerchantName_cust, 0, sizeof(RunData.aucMerchantName_cust));
	for(ucIndex=0;ucIndex<9;ucIndex++)
	{
		if(strlen((char*)DataSave0.ConstantParamData.aucMerchantName_cust[ucIndex])!=0)
		{
			memcpy(&RunData.aucMerchantName_cust[ucTotal], &DataSave0.ConstantParamData.aucMerchantName_cust[ucIndex], strlen((char*)DataSave0.ConstantParamData.aucMerchantName_cust[ucIndex]));
			ucTotal++;
		}
		//else
		//	break;
	}
	util_Printf("\nucTotal:%02x\n",ucTotal);
	if(ucTotal==0)
	{
		return ERR_MERSET;
	}
	UTIL_DispMerchant(1,0,3);

	while(1)
	{
		ucKey = UTIL_GetKey(30);
		switch(ucKey)
		{
			case KEY_F4:
				ucDone=0;
				ucF4Times++;
				util_Printf("\nucF4Times:%02x,ucTotal:%02x",ucF4Times,ucTotal);
				if(ucTotal<4)
				{
					UTIL_DispMerchant(1,0,3);
					break;
				}
				if(ucTotal<8)
				{
					if(ucF4Times%2==0)
					{
						UTIL_DispMerchant(1,0,3);
					}
					else
						UTIL_DispMerchant(0,3,ucTotal-3);
					break;
				}
				if(ucTotal<10)
				{
					if(ucF4Times%3==0)
					{
						UTIL_DispMerchant(1,0,3);
					}
					else if(ucF4Times%3==1)
						UTIL_DispMerchant(0,3,ucTotal-3);
					else
						UTIL_DispMerchant(0,7,ucTotal-7);
					break;
				}
			break;
			case KEY_CLEAR:
				ucDone=0;
				ucResult = ERR_CANCEL;
			break;
			default:
				if((ucKey>='1')&&(ucKey<='9')&&((ucKey-0x30)<=ucTotal)&&(pOut!=0x00))
				{
					ucDone = 1;
					NormalTransData.aucCustNo[0] = '0';
					NormalTransData.aucCustNo[1] = ucKey;//������
					memcpy(pOut,RunData.aucMerchantName_cust[ucKey-'1'],20);//�����ѡ���̻���
					break;
				}
		}
		if(ucDone == 1)
			break;
		if(ucResult == ERR_CANCEL)
			break;
	}

	return ucResult;
}
unsigned char UTIL_Check_BasetSet(void)
{
#ifdef GUI_PROJECT
	return ERR_END;
#endif

	Uart_Printf("\nG_NORMALTRANS_ucTransType_checkbase=%d,%d,%d",G_NORMALTRANS_ucTransType,TRANS_PURCHASE,DataSave0.ConstantParamData.ucDefaultTransParam);
	if(!I_Sale&&G_NORMALTRANS_ucTransType == TRANS_PURCHASE
		&&DataSave0.ConstantParamData.ucDefaultTransParam == TRANS_PURCHASE)
	{
		if(DataSave0.ConstantParamData.ucCashFlag != 0x31)
		{
            //Os__clr_display(255);
            //Os__GB2312_display(1,0,(unsigned char *)"�ݲ�֧�����ѽ���");
			MSG_WaitKey(3);
		}
		else
		{
			DataSave0.ChangeParamData.ucResult = 0x23;
		}
		return(SUCCESS);
	}
	if(!I_Auth&&G_NORMALTRANS_ucTransType == TRANS_PREAUTH
	&&DataSave0.ConstantParamData.ucDefaultTransParam == TRANS_PREAUTH)
	{
		if(DataSave0.ConstantParamData.ucCashFlag != 0x31)
		{
            //Os__clr_display(255);
            //Os__GB2312_display(1,0,(unsigned char *)" �ݲ�֧��Ԥ��Ȩ");
			MSG_WaitKey(3);
		}
		else
		{
			DataSave0.ChangeParamData.ucResult = 0x24;
		}
		return(SUCCESS);
	}

	if(DataSave0.ChangeParamData.ucCashierLogonFlag!=CASH_LOGONFLAG)
	{
		if(DataSave0.ConstantParamData.ucCashFlag != 0x31)
		{
            //Os__clr_display(255);
            //Os__GB2312_display(1,0,(unsigned char *)"  ��Ա��δǩ��");
            //Os__GB2312_display(2,0,(unsigned char *)"    ����ǩ����");
			MSG_WaitKey(3);
		}
		else
		{
			DataSave0.ChangeParamData.ucResult = 0x25;
		}
		#ifndef EMVTEST
			return(SUCCESS);
		#endif
	}
	if(DataSave0.ConstantParamData.ucLogonModiBatch)
	{
		if(DataSave0.ConstantParamData.ucCashFlag != 0x31)
		{
            //Os__clr_display(255);
            //Os__GB2312_display(1,0,(unsigned char *)" ���β�����ǩ��");
			MSG_WaitKey(3);
		}
		else
		{
			DataSave0.ChangeParamData.ucResult = 0x26;
		}
		#ifndef EMVTEST
			return(SUCCESS);
		#endif
	}

	if(DataSave0.ConstantParamData.ucFunctStep)
	{
		if(DataSave0.ConstantParamData.ucCashFlag != 0x31)
		{
            //Os__clr_display(255);
            //Os__GB2312_display(1,0,(unsigned char *)"    ���Ƚ��㣡");
			MSG_WaitKey(3);
		}
		else
		{
			DataSave0.ChangeParamData.ucResult = 0x27;
		}
		#ifndef EMVTEST
			return(SUCCESS);
		#endif
	}
	if(!DataSave0.Cashier_SysCashier_Data.ucSYSCashierExitFlag)
	{
		if(DataSave0.ConstantParamData.ucCashFlag != 0x31)
		{
            //Os__clr_display(255);
            //Os__GB2312_display(1,0,(uchar *)"���ܲ���Ա������");
            //Os__GB2312_display(2,0,(uchar *)"��������������Ա");
			MSG_WaitKey(3);
		}
		else
		{
			DataSave0.ChangeParamData.ucResult = 0x28;
		}
		#ifndef EMVTEST
			return(SUCCESS);
		#endif
	}

	if(DataSave0.ChangeParamData.ucStoreKeyRight)
	{
		if(DataSave0.ConstantParamData.ucCashFlag != 0x31)
		{
            //Os__clr_display(255);
            //Os__GB2312_display(1,0,(uchar *)"    ��Կ����");
            //Os__GB2312_display(2,0,(uchar *)"   ������ǩ����");
			MSG_WaitKey(3);
		}
		else
		{
			DataSave0.ChangeParamData.ucResult = 0x29;
		}
		#ifndef EMVTEST
			return(SUCCESS);
		#endif
	}
	return(ERR_END);

}


/*
int utf8togb2312(char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen)
{                                                                                     
  iconv_t cd;                                                                         
  if( (cd = iconv_open("gb2312","utf-8")) ==0 )                                       
    return -1;                                                                        
  memset(destbuf,0,destlen);                                                          
  char **source = (char**)&sourcebuf;                                                 
  char **dest   = (char**)&destbuf;                                                   
                                                                                      
  if(-1 == iconv(cd,source,&sourcelen,dest,&destlen))                                 
    return -1;                                                                        
  iconv_close(cd);                                                                    
  return 0;                                                                           
}              
*/