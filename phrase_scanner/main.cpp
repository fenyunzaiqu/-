//
//  main.cpp
//  phrase_scanner
//
//  Created by 周南 on 11/2/21.
//

#include <iostream>
#include <list>
#include <map>
#include <fstream>
#include <sstream>
#include <stdlib.h>
using namespace std;



//将关键词的种别写上
const static int begin_code=1, end_code=2,
integer_code=3,if_code=4,then_code=5,
else_code=6,function_code=7,read_code=8,write_code=9,
identifier_code=10,const_code=11,EOLN_code=24,EOF_code=25;
const static int SEM = 23;// ;分号 23
const static int Lbrac = 21;// ( 左括号
const static int Rbrac = 22;// ) 右括号
const static int assign_code=20;// := 符号 赋值 assign
const static int SUB=18;//减号
const static int MUL=19;//*号

class Variable{
public:
    Variable(string name,string process,int typeName,int position):name(name),process(process),typeName(typeName),position(position){}//构造函数
    string getName(){
        return name;
    }
    int getType(){
        return typeName;
    }
    string getProcess(){
        return process;
    }
    int getPosition(){
        return position;
    }
    string Format(int level,string padding){//level代表层次,padding储存要输出的字符串
        string res = padding + "Variable:\n";
        res += padding + "变量名vname:" + name + "\n";
        res += padding + "所属过程vproc:" + process + "\n";
        res += padding + "分类vkind: 0(变量）" + "\n";
        if (typeName == function_code){
            res+= padding + "变量类型vtype:function\n";
        }//如果是函数名的话
        else if(typeName == integer_code){//如果是INT类型的话
            res+= padding + "变量类型vtype: integer\n";
        }
        res += padding + "变量层次vlev:" + to_string(level)+"\n";
        res += padding + "变量表的位置vadr:" +to_string(position)+"\n";
        return res;
    }
private:
    string name;//变量名
    string process;//变量过程
    int typeName;//变量类型
    int position;//变量位置
};

class Process{
public:
    Process(string name,int level):processName(name),level(level){}
    Process(){}//两个构造函数，一个为空一个为满变量
    void addVar(Variable v){//往过程名表里加变量
        vars.push_back(v);
    }
    bool HashVar(Variable v){//检测变量名有没有重复，有重复返回true
        for(auto p : vars){//通过itertor遍历vars这个list容器 c++11写法，auto为变量推导
            if(p.getName()==v.getName()){//找到重复的了
                return true;
            }
        }
        return false;//没找到重复的
    }
    list<Variable> getVar(){
        return this->vars;//this为指向这个对象的指针，返回这个过程名表的所有变量名（以list的形式）
    }
    string getName(){
        return processName;
    }
    int getLevel(){
        return this->level;
    }
    string Format(string padding){
        if(getName()=="main"){//祝函数
            return "";
        }
        string res = padding + "Process";
        res += padding + "过程名pname:" + getName() + "\n";
        res += padding + "过程类型ptype: function函数\n";
        res += padding + "过程层次plev: "+to_string(level)+"\n";
        res += padding + "第一个变量位置fadr: "+ to_string(vars.front().getPosition())+"\n";
        res += padding + "最后一个变量位置ladr:" +to_string(vars.back().getPosition())+"\n";
        return res;
    }
private:
    list<Variable> vars;//将过程里的所有变量存在一个list里
    string processName;//过程名
    int level;//过程层次
};


struct lexSyn{//词法分析器文件
    string name;//名字
    int type;//种别
};

class Analysis{
public:
    Analysis(string data){//构造函数，输入参数是词法分析器后的文件
        this->source = data;//保存
        string name;
        int t;
        istringstream input(data);//将string字符串转化为输入流
        while(input>>name>>t){//转化为名字，类型，用list保存
            sourceList.push_back(lexSyn{name,t});
        }
    }
    ~Analysis(){//析构函数，释放资源
        for(auto p : allProcess){
            delete p;
        }
    }
    void startAnalysis();//（主）程序->分程序
    void dumpError(ostream &);//输出错误
    void dumpVar(ostream &);//输出变量表
private:
    Process *nowProcess;//指针，现在的过程
    Process *lastProcess;//指向最后的过程
    list <Process *> allProcess;//用list保存所有的过程
    list <lexSyn> sourceList;//保存所有的词（经过词法分析器后）
    string error;//保存所有的错误信息
    string sym;
    string source;//输入词法分析器的文件
    int line=1;//行号
    int varCount=0;//总变量
    void writeError(string);
    void addVar(string,int,int);
    list<lexSyn>::iterator getNext(list<lexSyn>::iterator p);//获得下一个词（的迭代器）
    list<lexSyn>::iterator getLast(list<lexSyn>::iterator p);//获得上一个词（的迭代器）
    
    //返回一个指向词法（名字，种别）的迭代器
    //分程序-> begin <说明语句表>;<执行语句表> end
    list<lexSyn>::iterator branch_procedure(list<lexSyn>::iterator);
    
    /*
     <说明语句表>-><说明语句>|<说明语句表>;<说明语句>
     消除左递归后为：
     <说明语句表>→<说明语句><说明语句表A>
     <说明语句表A>→;<说明语句><说明语句表A>│空
     */
    list<lexSyn>::iterator declare_statement_table(list<lexSyn>::iterator);
   
    /*
   左递归：<执行语句表>→<执行语句>│<执行语句表>；<执行语句>
   消除左递归后为
   <执行语句表>→<执行语句><执行语句表A>
   <执行语句表A>→;<执行语句><执行语句表A>│空  */
    list<lexSyn>::iterator exec_statment_table(list<lexSyn>::iterator);
    
    // <说明语句表A>→;<说明语句><说明语句表A>│空
    list<lexSyn>::iterator declare_statement_tableA(list<lexSyn>::iterator);
    
    // <说明语句>→<变量说明>│<函数说明>
    list<lexSyn>::iterator declare_statement(list<lexSyn>::iterator);
    
    list<lexSyn>::iterator varOrFunc(list<lexSyn>::iterator);
    
    // <变量说明>→integer <变量>
    list<lexSyn>::iterator var_declare(list<lexSyn>::iterator);
    
    // <函数说明>→integer function <标识符>（<参数>）；<函数体>
    list<lexSyn>::iterator func_declare(list<lexSyn>::iterator);

    // <变量>→<标识符>
    list<lexSyn>::iterator var(list<lexSyn>::iterator);
    
    // <执行语句表A>→;<执行语句><执行语句表A>│空
    list<lexSyn>::iterator exec_statment_tableA(list<lexSyn>::iterator);
    
    // <执行语句>→<读语句>│<写语句>│<赋值语句>│<条件语句>
    list<lexSyn>::iterator exec_statment(list<lexSyn>::iterator);
    
    /* <赋值语句>→<变量>:=<算术表达式>
       左递归：<算术表达式>→<算术表达式>-<项>│<项>
       改写：<算术表达式>→<项><算术表达式A>
       <算术表达式A>→-<项><算术表达式A>|空 */
    list<lexSyn>::iterator math_expression(list<lexSyn>::iterator);
    
    // <算术表达式A>→-<项><算术表达式A>|空
    list<lexSyn>::iterator math_expressionA(list<lexSyn>::iterator);
    
    /* 左递归：<项>→<项>*<因子>│<因子>
       改写<项>→<因子><项A>
       <项A>→*<因子><项A>│空 */
    
    list<lexSyn>::iterator item(list<lexSyn>::iterator);
    
    // <项A>→*<因子><项A>│空
    list<lexSyn>::iterator itemA(list<lexSyn>::iterator);
    
    // <因子>→<变量>│<常数>│<函数调用>
    list<lexSyn>::iterator factor(list<lexSyn>::iterator);
    
    // <条件语句>→if<条件表达式>then<执行语句>else <执行语句>
    list<lexSyn>::iterator condition_statement(list<lexSyn>::iterator);
    
    // <条件表达式>→<算术表达式><关系运算符><算术表达式>
    list<lexSyn>::iterator condition_expression(list<lexSyn>::iterator);
    
    // <关系运算符> →<│<=│>│>=│=│<>
    list<lexSyn>::iterator relation_operator(list<lexSyn>::iterator);
    
    void checkVar(string varName,int type){
        if(!isLetterAndNum(varName[0])){
            return;
        }
        if(nowProcess->HashVar(Variable(varName,nowProcess->getName(),type,0))){//检查变量表里有无同名变量
            return;
        }
        writeError(to_string(line)+"行未定义的变量:" +varName);
    }
    bool isLetterAndNum(char a){
        return isLetter(a)||isNumber(a);}
    bool isLetter(char a){
        if(a>='a' && a<='z') return true;
        if(a>='A' && a<='Z') return true;
        return false;
    }
    bool isNumber(char a){
        if(a>='0'&&a<='9')
            return true;
        return false;
    }
};

void Analysis::writeError(string msg){
    error+=msg+"\n";
}

//程序->分程序
void Analysis::startAnalysis(){
    auto p = sourceList.begin();//p为第一个迭代器，可以理解为下标
    while(p->type == EOLN_code){//当遇到换行后
        p++;//下一个词
        line++;//行数+1
    }
    nowProcess = new Process("main",0);//新建一个主进程
    allProcess.push_back(nowProcess);//放到队列最后
    auto next = branch_procedure(p);//主程序main->分程序branch
    if(next->type!=EOF_code)//如果最后分析返回的不是EOF
        writeError(to_string(line)+"行无法找到EOF");
}

void Analysis::dumpError(ostream &stream){
    stream<<error<<endl;
}

void Analysis::dumpVar(ostream &stream){
    for(auto p : allProcess){//不用p=.begin的方法的好处是，不用（*p）用括号括起来解开迭代器的引用（解引用）
        string padding="";
        for(int i=0;i<p->getLevel();i++){
            padding+="\t";//在同一行打印本层次的所有变量表
        }
        stream<<p->Format(padding);//控制输出格式
        auto vars = p->getVar();//一个过程中有多个变量，用vars保存这个过程中的所有变量
        for(auto v : vars){//遍历所有的变量
            stream<<v.Format(p->getLevel(), padding);//.访问将属性转换成字符串的Format方法(函数)，将这个过程的层次输入进Format，打印输出出来
        }
    }
}

void Analysis::addVar(string name, int type, int notNext=0){
    Variable tmpVar = Variable(name,nowProcess->getName(),type,(++varCount-notNext));
    //名字，过程名，类型，相对位置
    varCount -= notNext;
    if(nowProcess->HashVar(tmpVar)){//检测在变量表里有无相同变量名
        writeError(to_string(line)+"行多次定义相同变量名："+name);
    }
    else{
        nowProcess->addVar(tmpVar);
    }
}

list<lexSyn>::iterator Analysis::getNext(list<lexSyn>::iterator p){
    p++;//向前一个词（的迭代器）
    while(p->type == EOLN_code){//因为可能有多个空行，所以要跳到不是空行为止
        p++;
        line++;
    }
    return p;
}

list<lexSyn>::iterator Analysis::getLast(list<lexSyn>::iterator p){
    p--;
    while(p->type == EOLN_code){//同上
        p--;
        line--;
    }
    return p;
}

//分程序-> begin <说明语句表>;<执行语句表> end
list<lexSyn>::iterator Analysis::branch_procedure(list<lexSyn>::iterator p){
    auto next = getNext(p);
    if(p->type != begin_code){
        writeError("第"+to_string(line)+"行无法找到BEGIN标识符，只找到了"+p->name);
    }
    next = declare_statement_table(next); //说明语句表函数
    auto nnext=getNext(next);
    if(next->type!=SEM)//如果说说明语句表后不是分号;
    {
        writeError("第"+to_string(line)+"行缺少分号;");
        nnext = getLast(next);//那么返回上一个
    }
    next=exec_statment_table(nnext);// 执行语句表函数
    if(next->type!=end_code){
        writeError(to_string(line)+"行无法找到END标识符号");
    }
    return getNext(next);//返回END后的EOF(如果有的话）的标识符给main主函数
}

/*
 <说明语句表>-><说明语句>|<说明语句表>;<说明语句>
 消除左递归后为：
 <说明语句表>→<说明语句><说明语句表A>
 <说明语句表A>→;<说明语句><说明语句表A>│空
 */
list<lexSyn>::iterator Analysis::declare_statement_table(list<lexSyn>::iterator p){
    return declare_statement_tableA(declare_statement(p)); //相当于先执行说明语句函数，然后再用说明语句A执行剩下的词（通过返回的迭代器）
}


// <说明语句表A>→;<说明语句><说明语句表A>│空
list<lexSyn>::iterator Analysis::declare_statement_tableA(list<lexSyn>::iterator p){
    if(p->type!=SEM){//如果不是分号;
        return p;
    }
    auto p_next=getNext(p);//分号的下一个
    auto next = declare_statement(p_next);//进说明语句
    if(next == p_next){//如果没有说明语句
        return getLast(p_next);//那么返回分号
    }
    return declare_statement_tableA(next);//如果有说明语句那么进说明语句A
}

// <说明语句>→<变量说明>│<函数说明>
list<lexSyn>::iterator Analysis::declare_statement(list<lexSyn>::iterator p){
    if(p->type==integer_code)//如果是INTEGER关键词
    {
        return varOrFunc(getNext(p));//把INTEGER的后一个词扔进看是函数还是变量
    }
    else{
        auto backup = p;
        p=getNext(p);
        auto next=varOrFunc(p);
        if(next==p){
            return getLast(p);
        }
        if(next->type==SEM)//如果是分号
        {
            writeError(to_string(line)+"行无法找到INTEGER标识符而是找到了："+backup->name);
            return next;
        }
        return getLast(p);
    }
    return p;
}

list<lexSyn>::iterator Analysis::varOrFunc(list<lexSyn>::iterator p){
    auto next = var(p);//看这个单词是不是变量标识符
    if(next==p){//如果不是，那看是不是函数
        if(p->type!=function_code){//并且也不是FUNCTION标识符
            return p;//不是函数，那么就将这个词（的迭代器返回到上层）
        }
        p=getNext(p);
        next=var(p);
        addVar(p->name, function_code);
        auto nnext=getNext(next);
        if(next->type!=Lbrac){//如果不是左括号
            writeError(to_string(line)+"行缺少左括号(");
            nnext = getLast(nnext);
        }
        next = var(nnext);
        if(next->type!=Rbrac){
            writeError(to_string(line)+"行缺少右括号)");
            next=getLast(next);
        }
        nnext=getNext(next);//最后一个词的后一个
        if(nnext->type!=SEM){
            writeError(to_string(line)+"行缺少分号;");
            nnext=getLast(nnext);//相当于补上分号
        }
        lastProcess=nowProcess;//过程的条件都有了Function(); 那么这个（暂时）就是最后一个过程
        nowProcess=new Process(p->name,lastProcess->getLevel()+1);//然后创建一个下一层的新过程
        allProcess.push_back(nowProcess);//放到List存着
        addVar(p->name, function_code,1);//相对位置+1
        auto ret=branch_procedure(getNext(nnext));//分号的下一个词，看还是不是程序
        nowProcess=lastProcess;
        return ret;
    }
    if(next->type==SEM){
        addVar(p->name, integer_code);//如果这个变量时标识符，且下一个词（符号）是分号，那么加入变量表里去
    }
    return next;
}

list<lexSyn>::iterator Analysis::var(list<lexSyn>::iterator p){
    if(p->type==identifier_code){
        return getNext(p);
    }
    return p;
}


/*
左递归：<执行语句表>→<执行语句>│<执行语句表>；<执行语句>
消除左递归后为
<执行语句表>→<执行语句><执行语句表A>
<执行语句表A>→;<执行语句><执行语句表A>│空  */
list<lexSyn>::iterator Analysis::exec_statment_table(list<lexSyn>::iterator p){
    return exec_statment_tableA(exec_statment(p));
}


// <执行语句表A>→;<执行语句><执行语句表A>│空
list<lexSyn>::iterator Analysis::exec_statment_tableA(list<lexSyn>::iterator p){
    if(p->type!=SEM){//如果第一个符号不是分号，那么返回上一层，说明不是执行语句A
        return p;
    }
    p=getNext(p);//如果是分号了那么获得下一个词
    auto next = exec_statment(p);//进执行语句
    if(next!=p){//不为空
        next=exec_statment_tableA(next);//再进执行语句A
        return next;
    }
    return p;//为空就返回下一个词
}

// <执行语句>→<读语句>│<写语句>│<赋值语句>│<条件语句>
// <读语句>→read(<变量>)
// <写语句>→write(<变量>)
// <赋值语句>→<变量>:=<算术表达式>
// <条件语句>→if<条件表达式>then<执行语句>else <执行语句>
list<lexSyn>::iterator Analysis::exec_statment(list<lexSyn>::iterator p){
    if(p->type==read_code){
        p=getNext(p);
        auto nnext=getNext(p);
        if(p->type!=Lbrac){
            nnext=getLast(nnext);
            writeError(to_string(line)+"行缺少左括号(");
        }
        auto next=var(nnext);//检查是不是变量
        checkVar(nnext->name, integer_code);
        nnext=getNext(next);
        if(next->type!=Rbrac){
            nnext=getLast(nnext);
            writeError(to_string(line)+"行缺少右括号)");
        }
        return nnext;
    }
    if(p->type==write_code){
        p=getNext(p);
        if(p->type!=Lbrac){
            p=getLast(p);
            writeError(to_string(line)+"行缺少左括号(");
        }
        auto next=var(getNext(p));
        checkVar(p->name, integer_code);
        if(next->type!=Rbrac){
            next=getLast(next);
            writeError(to_string(line)+"行缺少右括号)");
        }
        return getNext(next);
    }
    auto next=var(p);
    
    // <赋值语句>→<变量>:=<算术表达式>
    if(next!=p){//如果不为空（是变量），p是变量，next是下一个字符
        checkVar(p->name, integer_code);//变量不重复
        if(next->type!= assign_code){//如果不是assgin := 符号的话
            next=getLast(next);
        }
        return math_expression(getNext(next));//扔到算术表达式函数里
    }
    
    // <条件语句>→if<条件表达式>then<执行语句>else <执行语句>
    if(p->type==if_code){
        p=getNext(p);
        next=condition_expression(p);
        if(next->type!=then_code){
            writeError(to_string(line)+"行缺少then关键词,而是找到了"+next->name);
        }
        next=exec_statment(getNext(next));
        if(next->type!=else_code){
            writeError(to_string(line)+"行缺少else关键词,而是找到了"+next->name);
        }
        next=exec_statment(getNext(next));
        return next;
    }
    return p;
}

/*
 <赋值语句>→<变量>:=<算术表达式>
 左递归：<算术表达式>→<算术表达式>-<项>│<项>
 改写：<算术表达式>→<项><算术表达式A>
 <算术表达式A>→-<项><算术表达式A>|空
 */
list<lexSyn>::iterator Analysis::math_expression(list<lexSyn>::iterator p){
    return math_expressionA(item(p)); //等同于p为全局变量，然后不返回值 item();mathexpreesionA();
}

// <算术表达式A>→-<项><算术表达式A>|空 注意那个-号，为18
list<lexSyn>::iterator Analysis::math_expressionA(list<lexSyn>::iterator p){
    if(p->type==SUB){
        auto next=getNext(p);
        return math_expressionA(item(next));
        //等同于前进一个字符，然后先扔进项里，项遍历完后返回的词（的迭代器）再扔算术表达式A里
    }
    return p;
}


/* 左递归：<项>→<项>*<因子>│<因子>
 # 改写<项>→<因子><项A>
 # <项A>→*<因子><项A>│空 */
list<lexSyn>::iterator Analysis::item(list<lexSyn>::iterator p){
    return itemA(factor(p));//等同于先因子后项A
}

// <项A>→*<因子><项A>│空 注意这个乘号 为19
list<lexSyn>::iterator Analysis::itemA(list<lexSyn>::iterator p){
    auto next = factor(p);//扔进因子里 可能有错误
    if(next==p){//如果因子是错的或者不存在
        if(next->type!=MUL){
            return next;//如果不是*MUL这个那么就直接return退出
        }
        next=factor(getNext(p));//扔到因子里
        if(next==p){//如果没有因子也直接退出
            return p;
        }
        return itemA(next);//继续扔到项A里
    }
    return p;
}

// <因子>→<变量>│<常数>│<函数调用>
// <函数调用>→<标识符>(<算数表达式>)
// <常数>→<无符号整数>
list<lexSyn>::iterator Analysis::factor(list<lexSyn>::iterator p){
    auto next=var(p);//检查是不是变量，如果是 next就是下一个词，如果不是那么next就是p
    if(p != next || p->type==Lbrac){//有一个标识符变量或者有一个左括号
        if(next->type == Lbrac){//左括号
            next = getNext(next);
            checkVar(p->name, function_code);//变量有无重复
            next=math_expression(next);//算术表达式
            if(next->type!=Rbrac){
                writeError(to_string(line)+"行缺少右括号");//右括号
                next=getLast(next);
            }
            return getNext(next);
        }
        checkVar(p->name, integer_code);
        return next;
    }
    if(p->type == const_code){
        return getNext(p);
    }
    return p;
}


// <条件表达式>→<算术表达式><关系运算符><算术表达式>
list<lexSyn>::iterator Analysis::condition_expression(list<lexSyn>::iterator p){
    return math_expression(relation_operator(math_expression(p)));
}

//这里三层函数可能很难理解，长注释一下；如果p是全局变量（p的作用是标记现在读到词的位置），那么这里的代码会是
// math_expression();
// relation_operator();
// math_expression();
//p自动在往后走（往后读词）
//但是这个p只是一个局部变量，离开作用域就没了，所以需要返回一个值（传递消息）来让p读词接力
//那么这里三重函数就是先进math，math返回后一个词的位置给realtion，最后再返回给最外面的math。

// <关系运算符> →<│<=│>│>=│=│<> //这六个关系运算符
list<lexSyn>::iterator Analysis::relation_operator(list<lexSyn>::iterator p){
    int i=p->type;
    if(i==12||i==13||i==14||i==15||i==16||i==17){
        return getNext(p);
    }
    writeError(to_string(line)+"行关系运算符出错，为"+p->name);
    return p;
}

int main(int argc, const char * argv[]) {
    ifstream source("/Users/zhounan/Downloads/out.txt");
    ofstream out("/Users/zhounan/Downloads/out2.txt");
    ofstream error("/Users/zhounan/Downloads/serror.txt");
    ofstream table("/Users/zhounan/Downloads/table.txt");
    string data{istreambuf_iterator<char>{source},istreambuf_iterator<char>{}};
    source.close();
    out<<data;
    out.close();
    Analysis anlysisor=Analysis(data);
    anlysisor.startAnalysis();
    anlysisor.dumpError(error);
    anlysisor.dumpVar(table);
    table.close();
    error.close();
    return 0;
}
