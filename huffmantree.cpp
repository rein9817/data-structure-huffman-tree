#include <iostream>
#include <map>
#include <queue>
#include <string>
using namespace std;

struct node //basic tree node structure
{
    int frequency;
    char ch;
    node *left;
    node* right;
}; 
typedef node* link;

struct cmp{       //刻給priority_queue用的自定類別
    bool operator()(link l,link r)
    {
        return l->frequency > r->frequency;
    }
};
//construct node
link construct(char c,int n,link l,link r)  //initalizing
{
    link newnode=(link)malloc(sizeof(node));
    newnode->ch=c;
    newnode->frequency=n;
    newnode->left=l;
    newnode->right=r;
    return newnode;
} 
void decode(link root,int &index,string binary,string &ans)//string s is decided by compressed file
{                                        //index determine  s[i]
    if(root == NULL)                  
        return;
    if(!root->left && !root->right) //store
    {
        ans.push_back(root->ch);
        return;
    }
    index++;
    
    if(binary[index] == '0') 
        decode(root->left,index,binary,ans);
    else 
        decode(root->right,index,binary,ans);
}
void encode(link root,string s,map<char,string> &m) 
{
    if(root==NULL) return ;
    if(root->left== NULL && root->right==NULL) //find a leaf node then store
    {                                          //所有加密節點都已經放在裡面了
        m[root->ch]=s;
    }
    encode(root->left,s+"0",m); 
    encode(root->right,s+"1",m);
}

//build huffman tree
link compressed_huffmantree(FILE* f,priority_queue<link,vector<link>,cmp > &p,map<char,int> &table)
{
    char a;
    map<char,int>::iterator it;
    while((a=fgetc(f))!=EOF) //compute frequency
        table[a]++;

    for(it=table.begin();it!=table.end();it++)
        p.push(construct(it->first,it->second,NULL,NULL)); //build basic leaf node and store them
    
    while(p.size()!=1) //merge the lowest and second lowest node 
    {
        link a=p.top();
        p.pop();
        link b=p.top();
        p.pop();
        p.push(construct('\0',a->frequency+b->frequency,a,b)); //'\0' means ascii 0
    }
    link root1=p.top();
    fclose(f);
    return root1; //return root
}

//table is for character frequency 
//huffmancode is for the binary code
void compress(link root,map<char,int> &table,FILE* data) 
{
    FILE* enc=fopen("wordenc.txt","w"); //build file
    FILE* dictionary=fopen("worddic.txt","w"); //build file

    map<char,string> huffmancode;   //map->first store the character,map->second store the code
    encode(root,"",huffmancode); //紀錄加密後的字串和對應的字母
    map<char,string>::iterator it;
    string s1=""; //加總所有的密碼
    char ch;
    while((ch=fgetc(data))!=EOF)
    {
        s1+=huffmancode[ch];
    }
    //cout<<s1<<endl;
    
    fprintf(enc,"%s",s1.c_str()); //output the encoded string 
    //output the frequency
    
    for(it=huffmancode.begin();it!=huffmancode.end();it++) 
        fprintf(dictionary,"%c-Frequency:%d,code:%s\n",it->first,table[it->first],it->second.c_str());
    
    //close all the file
    fclose(enc);
    fclose(dictionary);
    fclose(data);
    //output compression rate
    map<char,int>::iterator key; //recording pos
    int original_file_size=0;
    for(key=table.begin();key!=table.end();key++)
        original_file_size+=key->second;
    
    original_file_size*=8;
    int compressed_file_size=0;
    int l;
    for(it=huffmancode.begin();it!=huffmancode.end();it++) 
    {
        l=it->second.length();
        compressed_file_size+=l*table[it->first];
    }
    float compression_rate=(float)compressed_file_size/original_file_size;
    printf("Compression rate is:%0.6f%%  \n",(float)(1-compression_rate)*100);
    return ;
}

void decompress(FILE *f,link root) //write to file
{                                   //f equals datain
    FILE *dec=fopen("worddec.txt","w"); //build file
    string s="";
    string ans="";
    char c;
    int k=-1;
    while((c=fgetc(f))!=EOF) //fetch all the content
        s.push_back(c);
    
    // cout<<s<<endl; 
    while(k<(int)s.size()-2) //traverse tree
        decode(root,k,s,ans);
    //cout<<ans<<endl;
    fprintf(dec,"%s",ans.c_str());
    fclose(f);
    fclose(dec);
}
//reconstruct huffman tree
link decompressed_huffmantree(priority_queue<link,vector<link>,cmp > &p)
{
    FILE* dic=fopen("worddic.txt","r");
    char o;
    int f;
    while((fscanf(dic,"%c-Frequency:%d,code:%s\n",&o,&f))!=EOF) //input successfully
        p.push(construct(o,f,NULL,NULL)); //store in priority queue
    while(p.size()!=1) //merge again
    {
        link a=p.top();
        p.pop();
        link b=p.top();
        p.pop();
        p.push(construct('\0',a->frequency+b->frequency,a,b)); //'\0' means ascii 0
    }
    link str=p.top();
    fclose(dic); 
    return str;
}

void clear(link root) //avoid memory leak!!! very important!!!
{
    if(root==NULL) return;
    clear(root->left);
    clear(root->right);
    free(root);
}

int main()
{
    char op; //operation
    char filename[200]; //length of filename
    FILE* datain;
    // FILE* dataout;
    cout<<"Please input A or B (A for compression and B for decompression):";
    cin>>op;
    priority_queue<link,vector<link>,cmp > p; //default to from low to high
    if(op=='A')
    {
        cout<<"Please enter filename:";
        scanf("%s",filename);
        datain=fopen(filename,"r");
        map<char,int> table; //compute the frequency 
        link root=compressed_huffmantree(datain,p,table); //傳址進然後保存內容
        datain=fopen(filename,"r");
        compress(root,table,datain);
        cout<<"File created successfully\n";
        fclose(datain);
        clear(root);
    }
    else if(op=='B')
    {
        cout<<"Please enter filename:";
        scanf("%s",filename);
        datain=fopen(filename,"r");
        link root=decompressed_huffmantree(p); //reconstruct tree
        decompress(datain,root); 
        cout<<"File created successfully\n";
        clear(root);
    }
    else cout<<"You input unexpected character"<<endl;
    return 0;
}