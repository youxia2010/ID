#include <string.h>
#include "SCSId.h"

CSCSId::CSCSId(const string &strOtherChar)
{
	int idx = 0;
	for (const char *ptr = strOtherChar.c_str(); *ptr != '\0' && idx < 4; idx++)
	{
		int nChar = 0;
		if ((char)(*ptr & 0x80) == (char)0x00) nChar = 1;	//1�ֽ�
		else if ((char)(*ptr & 0xE0) == (char)0xC0) nChar = 2;//2�ֽ�
		else if ((char)(*ptr & 0xF0) == (char)0xE0) nChar = 3;//3�ֽ�
		else if ((char)(*ptr & 0xF8) == (char)0xF0) nChar = 4;//4�ֽ�
		else if ((char)(*ptr & 0xFC) == (char)0xF8) nChar = 5;//5�ֽ�
		else if ((char)(*ptr & 0xFE) == (char)0xFC) nChar = 6;//6�ֽ�
		m_vecOtherCodes.push_back(string(ptr, nChar));
		ptr += nChar;
	}
}

CSCSId::~CSCSId()
{
}

int CSCSId::Encode(const char *pSrc, int nSrcLen, char *pDst, int &nDstLen)
{
	int nOffset = 0;
	int nChar = 0;
	bool bEncode = true;
	const char *pSrcTmp = pSrc;
	char *pDstTmp = pDst;

	//�ȼ���������Ҫ��
	if (nDstLen < nSrcLen / 2 + nSrcLen % 2)
	{
		return -1;
	}

	char ch;
	int char_len;
	while(nOffset < nSrcLen)
	{
		//����
		if (*pSrcTmp >= 48 && *pSrcTmp <= 57)
		{
			ch = *pSrcTmp - 47;
			char_len = 1;
		}
		else 
		{
			if ((char)(*pSrcTmp & 0x80) == (char)0x00) char_len = 1;	//1�ֽ�
			else if ((char)(*pSrcTmp & 0xF0) == (char)0xE0) char_len = 3;//3�ֽ�
			else if ((char)(*pSrcTmp & 0xE0) == (char)0xC0) char_len = 2;//2�ֽ�
			else if ((char)(*pSrcTmp & 0xF8) == (char)0xF0) char_len = 4;//4�ֽ�
			else if ((char)(*pSrcTmp & 0xFC) == (char)0xF8) char_len = 5;//5�ֽ�
			else if ((char)(*pSrcTmp & 0xFE) == (char)0xFC) char_len = 6;//6�ֽ�

			//�������ַ��������Ƿ��ǿɱ�����ַ�
			int i = 0;
			for (i = 0; i < m_vecOtherCodes.size(); i++)
			{
				if (strncmp(pSrcTmp, m_vecOtherCodes[i].c_str(), char_len) == 0)
				{
					break;
				}
			}

			if (i < m_vecOtherCodes.size()) //���ֲ��ɱ�����ַ�
			{
				ch = (char)0x0B + i;
			}
			else
			{
				bEncode = false;
				break;
			}
		}

		pSrcTmp += char_len;
		nOffset += char_len;
		nChar++;
		if (nChar % 2 == 1)
		{
			ch = ch << 4;
			*pDstTmp = ch;
		}
		else
		{
			*pDstTmp = *pDstTmp + ch;
			pDstTmp++;
		}
	}

	//�����������ַ���ĩβ��0x0F���
	if (bEncode && nChar % 2 == 1)
	{
		*pDstTmp = *pDstTmp + (char)0x0F;
	}

	//���ڲ��ɱ�����ַ��������岻����
	if (bEncode == false)
	{
		if (nDstLen < nSrcLen + 1)
		{
			return -1;
		}
		*pDst = (char)0xFF;
		memcpy(pDst + 1, pSrc, nSrcLen);
		nDstLen = nSrcLen + 1;
	}
	else
	{
		nDstLen = nChar / 2 + nChar % 2;
	}

	return 0;
}

int CSCSId::Decode(const char *pSrc, int nSrcLen, char *pDst, int &nDstLen)
{
	//δ����ѹ��������ʱֱ��copy��ȥ
	if (*pSrc == (char)0xFF)
	{
		if (nDstLen < nSrcLen - 1)
		{
			return -1;
		}
		memcpy(pDst, pSrc + 1, nSrcLen - 1);

		nDstLen = nSrcLen - 1;
		return 0;
	}

	const char *pSrcTmp = pSrc;
	char *pDstTmp = pDst;
	int nOffset = 0;
	int nDecodeLen = 0;
	char ch;
	char chdecode[2];
	
	while(nOffset < nSrcLen)
	{
		ch = *pSrcTmp;
		pSrcTmp++;
		nOffset++;
		chdecode[0] = ((ch & 0xF0) >> 4) & 0x0F;
		chdecode[1] = (char)(ch & 0x0F);
		for (int i = 0; i < 2; i++)
		{
			if (chdecode[i] >= (char)0x01 && chdecode[i] <= (char)0x0A) //����
			{
				if(++nDecodeLen > nDstLen) return -1;
				*pDstTmp = chdecode[i] + 47;
				pDstTmp++;
			}
			else if (chdecode[i] < (char)0x0F)
			{
				nDecodeLen += m_vecOtherCodes[chdecode[i] - (char)0x0B].size();
				if (nDecodeLen > nDstLen) return -1;
				memcpy(pDstTmp, m_vecOtherCodes[chdecode[i] - (char)0x0B].c_str(), m_vecOtherCodes[chdecode[i] - (char)0x0B].size());
				pDstTmp += m_vecOtherCodes[chdecode[i] - (char)0x0B].size();
			}
			else if (chdecode[i] == (char)0x0F)
			{
				nDstLen = nDecodeLen;
				return 0;
			}
		}
	}

	nDstLen = nDecodeLen;
	return 0;
}
