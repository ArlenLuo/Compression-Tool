#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <deque>
#include <vector>
#include <algorithm>

#define SIZE 256

using namespace std;

struct node{
	unsigned char val;
	int times;
	node * left;
	node * right;
	node(unsigned char v){
		val=v;
		times=0;
		left=NULL;
		right=NULL;
	}
	node(node * lnode,node *rnode){
		left=lnode;
		right=rnode;
		if(lnode==NULL || rnode==NULL ){
			times=0;
		}else{
			times=lnode->times+rnode->times;
		}
	}
}* root;

struct HFcode{
	vector<int> hfcode;
};

void useage(string prog);

void compress(string inputFilename, string outputFilename);
void BuildHuffmanTree(string inputFilename,int timeslist[]);
bool compare(const node * n1,const node * n2);
void HuffmanCoding(int &sum,HFcode codelist[]);
void _HuffmanCoding(node * hotnode,vector <int> & nowcode,int & sum,HFcode codelist[]);
void FileCoding(string inputFilename,string outputFilename,int sum,int timeslist[],HFcode codelist[]);
void StoreHuffmanTree(node * hotnode ,int &v, int &len,ofstream & ofs);
void StoreLeaves(node * hotnode,ofstream & ofs);
void DeleteHuffmanTree(node * Node);

void decompress(string inputFilename, string outputFilename);
int getbit(char byt,int len);
node* RecoverHuffmanTree(ifstream & ifs,char &byt,int &len,int &nbr);
void RecoverLeaves(node * hotnode,ifstream & ifs);
void FileDecoding(ifstream & ifs,string outputFilename,int remain);

int main(int argc, char* argv[]) {
  int i;
  string inputFilename, outputFilename;
  bool isDecompress = false;
  for (i = 1; i < argc; i++) {
    if (argv[i] == string("-d")) isDecompress = true;
    else if (inputFilename == "") inputFilename = argv[i];
    else if (outputFilename == "") outputFilename = argv[i];
    else useage(argv[0]);
  }
  if (outputFilename == "") useage(argv[0]);
  if (isDecompress) decompress(inputFilename, outputFilename);
  else compress(inputFilename, outputFilename);
  return 0;
}

void useage(string prog) {
	cerr << "Useage: " << endl
		<< "    " << prog << "[-d] input_file output_file" << endl;
	exit(2);
}

void compress(string inputFilename, string outputFilename) {
	int timeslist[SIZE]={0};
	int sum=0;
	HFcode codelist[SIZE];
	BuildHuffmanTree(inputFilename,timeslist);
	HuffmanCoding(sum,codelist);
	FileCoding(inputFilename,outputFilename,sum,timeslist,codelist);
	DeleteHuffmanTree(root);
};

void BuildHuffmanTree(string inputFilename,int timeslist[]){
	deque<node *> que1;
	deque<node *> que2;

	//读取文件
	ifstream ifs(inputFilename.c_str(), ios::in | ios::binary);
	char c;
	while (ifs.get(c)){
		unsigned char uc=(unsigned char)c;
		if(timeslist[uc]==0)
			que1.push_back(new node(uc));
		timeslist[uc]++;
	}
	ifs.close();

	//保证que1有两个元素
	if(que1.size()==0){
		que1.push_back(new node(0));
		que1.push_back(new node(255));
	}
	if(que1.size()==1)
		que1.push_back(new node(255-que1[0]->val));
	for(unsigned int i=0;i<que1.size();i++)
		que1[i]->times=timeslist[que1[i]->val];

	//构造huffman tree
	sort(que1.begin(),que1.end(),compare);
	while(!(que1.empty() && que2.size()==1)){//循环结束状态
		if(que2.empty()||(que1.size()>1 && !que2.empty() && que1[1]->times<que2[0]->times)){
			node * Node=new node(que1[0],que1[1]);
			que1.pop_front();
			que1.pop_front();
			que2.push_back(Node);
		}else if(que1.empty()||(que2.size()>1 && !que1.empty() && que2[1]->times<que1[0]->times)){
			node * Node=new node(que2[0],que2[1]);
			que2.pop_front();
			que2.pop_front();
			que2.push_back(Node);
		}else {
			node * Node;
			if (que1[0]<=que2[0])
				Node=new node(que1[0],que2[0]);
			else
				Node=new node(que2[0],que1[0]);
			que1.pop_front();
			que2.pop_front();
			que2.push_back(Node);
		}
	}
	root=que2[0];
}

bool compare(const node * n1,const node * n2){
	return (n1->times)<(n2->times);
}

void HuffmanCoding(int &sum,HFcode codelist[]){
	vector <int> tmpcode;
	_HuffmanCoding(root,tmpcode,sum,codelist);
}

void _HuffmanCoding(node * hotnode,vector <int> & nowcode,int & sum,HFcode codelist[]){
	if(hotnode->left!=NULL && hotnode->right!=NULL){
		nowcode.push_back(0);
		_HuffmanCoding(hotnode->left,nowcode,sum,codelist);
		nowcode.pop_back();
		nowcode.push_back(1);
		_HuffmanCoding(hotnode->right,nowcode,sum,codelist);
		nowcode.pop_back();
	}else if(hotnode->left==NULL && hotnode->right==NULL ){
		codelist[hotnode->val].hfcode=nowcode;
		sum++;
	}
}

void FileCoding(string inputFilename,string outputFilename,int sum,int timeslist[],HFcode codelist[]){
	ifstream ifs(inputFilename.c_str(), ios::in | ios::binary);
	ofstream ofs(outputFilename.c_str(), ios::out | ios::binary);
	int i,cnt=0;
	//压入header：1.叶子数-1（一共可能256个叶子 叶子数减1乘2恰为树枝数）2.读码的余数 3.HF树形状（前序遍历，左为0右为1） 4.HF树叶子的信息
	ofs.put(char(sum-1));
	for(i=0;i<SIZE;i++){
		if (timeslist[i]!=0)
			cnt+=codelist[i].hfcode.size()*timeslist[i];
	}
	ofs.put((unsigned char)(cnt%8));
	int byt=0,len=0;
	StoreHuffmanTree(root,byt,len,ofs);
	if(len!=0)
		ofs.put((unsigned char)(byt<<(8-len)));
	StoreLeaves(root,ofs);

	//压入body
	byt=len=0;
	char ch;
	while(ifs.get(ch)){
		unsigned char usch=(unsigned char)ch;
		for(unsigned int i=0;i<codelist[usch].hfcode.size();i++){
			byt=(byt<<1)|codelist[usch].hfcode[i];
			len++;
			if(len==8){
				ofs.put(char(byt));
				byt=0;
				len=0;
			}
		}
	}
	if(len!=0){
		vector <int> remains=vector<int>(8-len,0);
		for(int i=0;i<8-len;i++)
			byt=(byt<<1)|remains[i];
		ofs.put(char(byt));
	}
	ifs.close();
	ofs.close();
}

void StoreHuffmanTree(node * hotnode ,int &v, int &len,ofstream & ofs){
	if(hotnode->left!=NULL && hotnode->right!=NULL){
		v<<=1;
		len++;
		if(len==8){
			ofs.put((unsigned char)(v));
			v=0;
			len=0;
		}
		StoreHuffmanTree(hotnode->left,v,len,ofs);
		v<<=1;
		v|=1;
		len++;
		if(len==8){
			ofs.put((unsigned char)(v));
			v=0;
			len=0;
		}
		StoreHuffmanTree(hotnode->right,v,len,ofs);
	}
}

void StoreLeaves(node * hotnode,ofstream & ofs){
	if(hotnode->left==NULL && hotnode->right==NULL){
		ofs.put(hotnode->val);
	}else if(hotnode->left!=NULL && hotnode->right!=NULL){
		StoreLeaves(hotnode->left,ofs);
		StoreLeaves(hotnode->right,ofs);
	}
}

void DeleteHuffmanTree(node * Node){
	if(Node->left!=NULL)
		DeleteHuffmanTree(Node->left);
	if(Node->right!=NULL)
		DeleteHuffmanTree(Node->right);
	delete Node;
}

void decompress(string inputFilename, string outputFilename) {
	char info;
	ifstream ifs(inputFilename.c_str(), ios::in | ios::binary);
	ifs.get(info);
	int sum=int(0xff & info)*2;//树枝个数
	ifs.get(info);
	int remain=info;
	ifs.get(info);
	int len=8;
	root=RecoverHuffmanTree(ifs,info,len,sum);
	RecoverLeaves(root,ifs);
	FileDecoding(ifs,outputFilename,remain);
	ifs.close();
	DeleteHuffmanTree(root);
}

int getbit(char byt,int len){
	return 1&(byt>>(len-1));
}

node* RecoverHuffmanTree(ifstream & ifs,char &byt,int &len,int &nbr){
	node * lnode=NULL;
	node * rnode=NULL;
	if(getbit(byt,len)==0)
	{
		len--;
		nbr--;
		if(len==0){
			ifs.get(byt);
			len=8;
		}
		lnode=RecoverHuffmanTree(ifs,byt,len,nbr);
		if(getbit(byt,len)==1){
			len--;
			nbr--;
			if(nbr==0){
				rnode = new node(NULL,NULL);
			}else{
				if(len==0){
					ifs.get(byt);
					len=8;
				}
				rnode=RecoverHuffmanTree(ifs,byt,len,nbr);
			}
		}
	}
	return new node(lnode,rnode);
}

void RecoverLeaves(node * hotnode,ifstream & ifs){
	if(hotnode->left==NULL && hotnode->right==NULL){
		char ch;
		ifs.get(ch);
		hotnode->val=ch;
	}else if(hotnode->left!=NULL && hotnode->right!=NULL){
		RecoverLeaves(hotnode->left,ifs);
		RecoverLeaves(hotnode->right,ifs);
	}
}

void FileDecoding(ifstream & ifs,string outputFilename,int remain){
	ofstream ofs(outputFilename.c_str(), ios::out | ios::binary);
	char byt1;//byt2先读，byt1是它前面的byte
	char byt2;
	int i;
	if(!ifs.get(byt1))
		return;
	node * hotnode=root;
	while(ifs.get(byt2)){
		for(i=7;i>=0;i--){
			if(((byt1>>i) &1)==0)
				hotnode=hotnode->left;
			else
				hotnode=hotnode->right;
			if(hotnode->left==NULL && hotnode->right==NULL){
				ofs.put(hotnode->val);
				hotnode=root;
			}
		}
		byt1=byt2;
	}
	for(i=7;i>=((8-remain)%8);i--){
		if(((byt1>>i) &1)==0)
			hotnode=hotnode->left;
		else
			hotnode=hotnode->right;
		if(hotnode->left==NULL && hotnode->right==NULL){
			ofs.put(hotnode->val);
			hotnode=root;
		}
	}
	ofs.close();
}

