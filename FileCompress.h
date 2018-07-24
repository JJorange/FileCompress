#pragma once
 
#include<fstream>
#include<string>
#include<assert.h>
#include"HuffmanTree.h"

typedef long long LongType;

struct CharInfo
{
	char _ch;			//�ַ�
	LongType _count;	//����
	string _code;		//����

	CharInfo operator+(const CharInfo& info)
	{
		CharInfo ret;
		ret._count = _count + info._count;
		return ret;
	}
	bool operator > (const CharInfo& info)const
	{
		return _count > info._count;
	}
	bool operator!=(const CharInfo& info)
	{
		return _count != info._count;
	}
};

class FileCompress
{
	typedef HaffmanTreeNode<CharInfo> Node;
public:
	struct ConfigInfo
	{
		char _ch;
		LongType _count;
	};
	FileCompress()
	{
		for (size_t i = 0; i < 256; i++)
		{
			_hashInfos[i]._ch = i;
			_hashInfos[i]._count = 0;
		}
	}
	void Compress(const char* file)
	{
		//1��ͳ���ļ����ַ����ֵĴ���
		ifstream ifs(file, ios_base::in | ios_base::binary);
		char ch;
		//while (ifs >> ch)
		while (ifs.get(ch))
		{
			_hashInfos[(unsigned char)ch]._count++;
		}
		//2������Huffman Tree
		CharInfo invalid;
		invalid._count = 0;
		HaffmanTree<CharInfo> tree(_hashInfos, 256, invalid);

		//3������haffman code
		GenerateHaffmanCode(tree.GetRoot());

		//4��ѹ��
		string compressfile = file;
		compressfile += ".haffman";
		ofstream ofs(compressfile.c_str(), ios_base::out | ios_base::binary);

		//5��д���ַ����������ڽ�ѹ��ʱ�ؽ�haffman tree
		//�����ƶ�д
		for (size_t i = 0; i < 256; i++)
		{
			if (_hashInfos[i]._count>0)
			{			
				ConfigInfo info;
				info._ch = _hashInfos[i]._ch;
				info._count = _hashInfos[i]._count;
				ofs.write((char*)&info, sizeof(ConfigInfo));
			}
		}
		//CharInfo end;
		ConfigInfo end;
		end._count = 0;
		ofs.write((char*)&end, sizeof(ConfigInfo));

		char value = 0;
		int pos = 0;
		ifs.clear();
		ifs.seekg(0);
		while (ifs.get(ch))
		{
			string& code = _hashInfos[(unsigned char)ch]._code;
			for (size_t i = 0; i < code.size(); i++)
			{
				if (code[i] == '0')
					value &= (~(1 << pos));
				else if (code[i] == '1')
					value |= (1 << pos);
				else
					assert(false);
				++pos;
				if (pos == 8)
				{
					ofs.put(value);
					pos = 0;
					value = 0;
				}
			}
		}
		if (pos > 0)
		{
			ofs.put(value);
			printf("%x ", value);
		}
	}

	void GenerateHaffmanCode(Node* root)
	{
		if (root == NULL)
			return;
		if (root->_left == NULL && root->_right == NULL)
		{
			string& code = _hashInfos[(unsigned char)root->_w._ch]._code;
			Node* cur = root;
			Node* parent = cur->_parent;
			while (parent)
			{
				if (cur == parent->_left)
					code += '0';
				else
					code += '1';
				cur = parent;
				parent = parent->_parent;
			}
			reverse(code.begin(), code.end());
		}
		GenerateHaffmanCode(root->_left);
		GenerateHaffmanCode(root->_right);
	}
	//void GenerateHaffmanCode(Node* root)
	//{
	//	if (root == NULL)
	//		return;
	//	if (root->_left)
	//	{
	//		root->_left->_w._code = root->_w._code + '0';
	//		GenerateHaffmanCode(root->_left);
	//	}
	//	if (root->_right)
	//	{
	//		root->_right->_w._code = root->_w._code + '1';
	//		GenerateHaffmanCode(root->_right);
	//	}
	//}
	void Uncompress(const char* file)
	{
		//1����ѹ���ļ����н�ѹ��
		ifstream ifs(file, ios_base::in | ios_base::binary);

		string uncompressfile = file;
		size_t pos = uncompressfile.rfind('.'); //�Ӻ���ǰ�ҵ�һ�� .
		assert(pos != string::npos);
		uncompressfile.erase(pos); //ɾ����׺

#ifdef _DEBUG
		uncompressfile += ".unhaffman";
#endif

		ofstream ofs(uncompressfile.c_str(), ios_base::out | ios_base::binary);
		//�ؽ�haffman��
		while (1)
		{
			ConfigInfo info;
			ifs.read((char*)&info, sizeof(ConfigInfo));
			if (info._count > 0)
				_hashInfos[(unsigned char)info._ch]._count = info._count;
			else
				break;
		}

		CharInfo invalid;
		invalid._count = 0;
		HaffmanTree<CharInfo> tree(_hashInfos, 256, invalid);

		//��ѹ��
		Node* root = tree.GetRoot();
		LongType filecount = root->_w._count;
		Node* cur = root;
		char ch;
		while (ifs.get(ch))
		{
			for (size_t pos = 0; pos < 8; ++pos)
			{
				if (ch & (1 << pos)) //1
					cur = cur->_right;
				else // 0
					cur = cur->_left;

				if (cur->_left == NULL && cur->_right == NULL) //��Ҷ����
				{
					ofs.put(cur->_w._ch);
					cur = root;
					if (--filecount == 0)
						break;
				}
			}
		}
	}
private:
	CharInfo _hashInfos[256];
};

void TestFileCompress()
{
	FileCompress fc;
	fc.Compress("input.txt");
	fc.Uncompress("input.txt.haffman");
}

//80000
