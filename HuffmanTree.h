```c++
#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _CRT_NONSTDC_NO_DEPRECATE 1
#include<assert.h>
#include<string>
#include<stdio.h>
#include<stdlib.h>

#include"Huffman.hpp"
using namespace std;


struct CharInfo
{
	char _ch; // 当前字符
	long long _appearCount; // 当前字符在文件中出现的次数
	string _strCode; // 当前字符对应的Huffman编码

	CharInfo(long long count = 0)
		: _appearCount(count)
	{}

	bool operator!=(const CharInfo& c)
	{
		return _appearCount != c._appearCount;
	}

	bool operator==(const CharInfo& c)
	{
		return _appearCount == c._appearCount;
	}

	bool operator<(const CharInfo& c)
	{
		return _appearCount > c._appearCount;
	}

	long long operator+(const CharInfo& c)
	{
		return _appearCount + c._appearCount;
	}
};

class FileCompress
{
	struct ConfigInfo
	{
		char _ch; // 当前字符
		long long _appearCount; // 当前字符在文件中出现的次数
	};
public:
	FileCompress()
	{
		for (int i = 0; i < 256; ++i)
			_charInfo[i]._ch = i;
	}

	void CompressFile(const string& strFilePath)
	{
		// 1.获取文件中每个字符出现的次数
		FILE* fIn = fopen(strFilePath.c_str(), "r");
		assert(fIn);

		char * pReadBuff = new char[1024];
		while (1)
		{
			size_t rdSize = fread(pReadBuff, 1, 1024, fIn);
			if (0 == rdSize)
				break;

			for (size_t i = 0; i < rdSize; ++i)
				_charInfo[pReadBuff[i]]._appearCount++;
		}

		// 以字符的权值为结点创建Huffman树
		HuffManTree<CharInfo> ht(_charInfo, 256, CharInfo(0));

		// 获取每个字符的Huffman编码
		GetCharHuffManCode(ht.GetRoot());
		
		// 按照字符的编码重写改写文件
		FILE* fOut = fopen("2.hzp", "w");
		assert(fOut); 

		// 先写压缩编码的信息
		string strFileHead;
		strFileHead += GetFilePosFix(strFilePath);
		strFileHead += '\n';

		string strCode;
		char szCount[32];
		size_t linkCount = 0;
		for (size_t i = 0; i < 256; ++i)
		{
			if (_charInfo[i]._appearCount)
			{
				strCode += _charInfo[i]._ch;
				strCode += ',';
				itoa(_charInfo[i]._appearCount, szCount, 10);
				strCode += szCount;
				//strCode += ',';
				//strCode += _charInfo[i]._ch;
				strCode += '\n';
				linkCount++;
			}

		}

		itoa(linkCount, szCount, 10);
		strFileHead += szCount;
		strFileHead += '\n';
		/*strCode += '\r';*/
		strFileHead += strCode;
		fwrite(strFileHead.c_str(), 1, strFileHead.size(), fOut);

		//string strCode;
		//size_t linkCount = 0;
		//char szCount[32];
		//CharInfo a;
		//for (int i = 0; i < 256; ++i)
		//{
		//	if (_charInfo[i]._appearCount>0)
		//	{
		//		ConfigInfo info;
		//		info._ch = _charInfo[i]._ch;
		//		info._appearCount = _charInfo[i]._appearCount;
		//		strCode += _charInfo[i]._ch;
		//		strCode += _charInfo[i]._appearCount;
		//		
		//		linkCount++;
		//	}
		//}
		////strFileHead += itoa(linkCount, szCount, 10);
		//strFileHead += strCode;
		//strFileHead += "00000000";
		//fwrite(strFileHead.c_str(), 1, sizeof(strCode), fOut);
		// 写压缩数据
		fseek(fIn, 0, SEEK_SET);
		char* pWriteBuff = new char[1024];
		char ch = 0;
		char pos = 0;
		size_t wrSize = 0;
		while (true)
		{
			size_t rdSize = fread(pReadBuff, 1, 1024, fIn);
			if (rdSize == 0)
				break;
			for (size_t i = 0; i < rdSize; ++i)
			{
				string& strCode = _charInfo[pReadBuff[i]]._strCode;
				char j = 0;
				while (j < strCode.size())
				{
					ch <<= 1;
					if ('1' == strCode[j])
						ch |= 1;

					pos++;
					if (8 == pos)
					{
						pWriteBuff[wrSize++] = ch;
						if (wrSize == 1024)
						{
							
							fwrite(pWriteBuff, 1, 1024, fOut);
							wrSize = 0;
						}
						pos = 0;
					}
					j++;
				}
			}
		}

		if (pos < 8)
		{

			ch <<= (8 - pos);
			pWriteBuff[wrSize++] = ch;
		}
		fwrite(pWriteBuff, 1, wrSize, fOut);

		delete[] pWriteBuff;
		delete[] pReadBuff;
		ht.~HuffManTree();
		fclose(fIn);
		fclose(fOut);
	}


	// 解压缩
	void UnCompressFile(const string& strFilePath)
	{
		
		FILE* fIn = fopen(strFilePath.c_str(), "r");
		assert(fIn);

		// 一次读取一行
		string strFilePosFix;
		ReadLine(fIn, strFilePosFix);

		string strInfo;
		ReadLine(fIn, strInfo);
		size_t lineCount = atoi(strInfo.c_str());

		for (size_t i = 0; i < lineCount; i++)
		{
			strInfo = "";
			ReadLine(fIn, strInfo); // A,1
			
			/*cout << "**" << strInfo.find_first_of(',') + 1 << ":" << atoi(strInfo.c_str())<< endl;*/
			_charInfo[strInfo[0]]._appearCount = atoi(strInfo.c_str()+2);
		}
		
		string strUnCompressFile = GetFileName(strFilePath);
		strUnCompressFile += strFilePosFix;
		FILE* fOut = fopen(strUnCompressFile.c_str(), "w");
		size_t wrSize = 0;

		// 重建Huffman树
		HuffManTree<CharInfo> ht(_charInfo, 256, CharInfo(0));

		char* pRDBuff = new char[1024];
		char* pWRBuff = new char[1024];
		int pos = 7;
		HuffManTreeNode<CharInfo>* pCur = ht.GetRoot();
		size_t FileSize = pCur->_weight._appearCount;
		size_t unFileSize = 0;
		
		while (true)
		{
			
			size_t rdSize = fread(pRDBuff, 1, 1024, fIn);
			cout << rdSize << endl;
			if (rdSize == 0)
				break;
			
			/*for (int i = 130; i < 137; i++)
			{
				cout << pRDBuff[i];
			}*/

			for (size_t i = 0; i < rdSize; ++i)
			{
				pos = 7;
				while (pos >= 0)
				{
					if (pRDBuff[i] & (1 << pos))
						pCur = pCur->_pRight;
					else
						pCur = pCur->_pLeft;

					if (pCur->_pLeft == NULL && pCur->_pRight == NULL)
					{
						
						pWRBuff[wrSize++] = pCur->_weight._ch;
						unFileSize++;
						if (wrSize == 1024)
						{
							fwrite(pWRBuff, 1, 1024, fOut);
							wrSize = 0;
						}
						cout << FileSize << ":" << unFileSize << endl;
						if (unFileSize == FileSize)
						{
							fwrite(pWRBuff, 1, wrSize, fOut);
							pCur = ht.GetRoot();
							break;
						}
						pCur = ht.GetRoot();
					}
					pos--;
				}
			}
		}

		delete[] pRDBuff;
		delete[] pWRBuff;

		fclose(fIn);
		fclose(fOut);
	}
private:
	void GetCharHuffManCode(HuffManTreeNode<CharInfo> * pRoot)
	{
		if (pRoot == NULL)
			return;

		GetCharHuffManCode(pRoot->_pLeft);
		GetCharHuffManCode(pRoot->_pRight);

		if (pRoot->_pLeft == NULL && NULL == pRoot->_pRight)
		{
			HuffManTreeNode<CharInfo>* pCur = pRoot;
			HuffManTreeNode<CharInfo>* pParent = pCur->_pParent;

			string& strCode = _charInfo[pCur->_weight._ch]._strCode;
			while (pParent)
			{
				if (pCur == pParent->_pLeft)
					strCode += '0';
				else
					strCode += '1';

				pCur = pParent;
				pParent = pCur->_pParent;
			}

			// 逆转
			reverse(strCode.begin(), strCode.end());
		}
	}

	string GetFilePosFix(const string& path)
	{
		return path.substr(path.find_last_of('.'));
	}

	string GetFileName(const string& path)
	{
		return path.substr(0, path.find_last_of('.'));
	}

	void ReadLine(FILE* fIn, string& strInfo)
	{
		char ch = fgetc(fIn);
		if (EOF == ch)
			return;
		while (ch != '\n')
		{
			strInfo += ch;
			ch = fgetc(fIn);
			if (EOF == ch)
				return;
		}
	}

private:
	CharInfo _charInfo[256];
};

void TestFileCompress()
{
	FileCompress fc;
	fc.CompressFile("1.txt");
	fc.UnCompressFile("2.hzp");
}	
```
