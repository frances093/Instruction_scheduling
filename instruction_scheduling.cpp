#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <math.h>

using namespace std;

// IQ
vector<string*> code;
vector<int*> numcode;
	// [ ix ][ inst. name ][ destination ][ op1 ][ op2 ]

// for RS
struct RSX
{
	bool occupied;	// true = is occupied 
	int inst_type;	// add, sub, addi, subi, mul, div, muli, divi
	int inst_num;	// store ix
	int dest;		// store destination
	int op[2];		// store actural data for operation
	int valid[2];	// -1: ok, 0: not ok, other: wait for which inst.
};
vector<RSX*> addRS;
vector<RSX*> mulRS;

// for ALU
struct ALU
{
	bool ALUoccupied;	// true = can not put in new instruction
	int release;		// which cycle will ALU be released
	int alurs;			// this istruction is from which RS
	int ins_num;		// ix
	int ins_type;		// type of this instruction
	int aluop[2];		// operant of this instruction
	int aludest;		// destination of this instruction
};
vector<ALU*> alu;

// RAT relate
int RAT[8] = { -1, -1, -1, -1, -1, -1, -1, -1};
	// -1: not waitiog any instruction, other: waitiog which RSX 
int RATgene[8] = { -1, -1, -1, -1, -1, -1, -1, -1};
	// -1: not waitiog any instruction, other: ix of instruction this RAT is waiting

// RF relate
int regi[8] = { 0, 0, 0, 0, 0, 0, 0, 0};

// contral variable
bool shouldexe = true;	// set false if there is unvalid input
bool noexception = true;	// operation exception
int nowcycle = 0;		// show now cycle
int timecomsum[8] = {1, 1, 1, 1, 1, 1, 1, 1};	// show svery instruction need time
int numinaddRS = 0;		// show how much instruction is in add RS
int numinmulRS = 0;		// show how much instruction is in mul RS

// for track
vector<int*> fortrack;

//==================== transist ========================
int findnum(string a)
{
	int temp = 0;
	if(a == "add"){return 0;	}
	else if(a == "sub"){return 1;	}
	else if(a == "mul"){return 4;	}
	else if(a == "div"){return 5;	}
	else if(a == "addi"){return 2;	}
	else if(a == "subi"){return 3;	}
	else if(a == "muli"){return 6;	}
	else if(a == "divi"){return 7;	}
	else if(a == "R0"){return 0;	}
	else if(a == "R1"){return 1;	}
	else if(a == "R2"){return 2;	}
	else if(a == "R3"){return 3;	}
	else if(a == "R4"){return 4;	}
	else if(a == "R5"){return 5;	}
	else if(a == "R6"){return 6;	}
	else if(a == "R7"){return 7;	}
	else if(a == "0"){return 0;	}
	else if(a == "1"){return 1;	}
	else if(a == "2"){return 2;	}
	else if(a == "3"){return 3;	}
	else if(a == "4"){return 4;	}
	else if(a == "5"){return 5;	}
	else if(a == "6"){return 6;	}
	else if(a == "7"){return 7;	}
	else if(a == "8"){return 8;	}
	else if(a == "9"){return 9;	}
	else
	{
		shouldexe = false;
		cout << "Inclding unvalid input!" << endl;
		return -1;	
	}
}

int getnum(string a)
{
	int num = 0;
	string temp;
	for(int k = 0; k < a.size() ;k++)
	{
		if(int(a[k]) > 57 || a[k] < 48)			// not number
		{
			return findnum(a);
		}
		else
		{
			temp = a[k];
			num = num + findnum(temp) * pow(10, a.size()-1-k);
		}
	}
	return num;
}

void transinst()
{
	int *ptrn;
	for (int i = 0; i < code.size(); i++)
	{
		ptrn = new int[4];
		*(ptrn) = findnum(*(code[i]));
		*(ptrn + 1) = findnum(*(code[i] + 1));
		*(ptrn + 2) = findnum(*(code[i] + 2));
		if(*(ptrn) == 2 || 3 || 6 || 7)		// op2 is int
		{
			*(ptrn + 3) = getnum(*(code[i] + 3));
		}
		else
		{
			*(ptrn + 3) = findnum(*(code[i] + 3));
		}
		numcode.push_back(ptrn);
		delete [] code[i];
	}
	if(shouldexe)
	{
		cout << "Instruction.	OK!" << endl;
	}
}

//================== read file =======================
void generateALU(int num)
{
	ALU *ptra;
	for(int i = 0; i < num; i++)
	{
		ptra = new ALU;
		ptra->ALUoccupied = false;
		alu.push_back(ptra);
	}
}

void generateRS(string line)
{
	RSX *ptrr;
	string temp;
	int type;
	
	for(int i = 0; i < line.size(); i++)
	{
		if(line[i] == ':')
		{
			type = getnum(temp);
			temp.clear();
		} 
		else
		{
			temp = temp + line[i];
		}
	}
	for (int i = 0; i < getnum(temp); i++)
	{
		ptrr = new RSX;
		ptrr->occupied = false;
		if(type == 0)
		{
			addRS.push_back(ptrr);	
		}
		else
		{
			mulRS.push_back(ptrr);
		}	
	}
}

void assigntimecomsum(string line)
{
	int assign = -1;
	string temp;
	
	for(int i = 0; i < line.size(); i++)
	{
		if(line[i] == ':')
		{// get instruction type
			assign = findnum(temp);
			temp.clear();
		} 
		else
		{
			temp = temp + line[i];
		}
	}
	timecomsum[assign] = getnum(temp);
}

void initialregister(string line)
{	
	int assign = -1;
	bool neg = false;
	string temp;
	
	for(int i = 0; i < line.size(); i++)
	{
		if(line[i] == '=')
		{	// get register name
			assign = findnum(temp);
			temp.clear();
		}
		else if(line[i] == '-')
		{
			neg = true;
		}
		else
		{
			temp = temp + line[i];
		}
	}
	
	if(neg)
	{
		regi[assign] = 0 - getnum(temp);
	}
	else
	{
		regi[assign] = getnum(temp);
	}
}

void storeinstruction(string line)
{
	int count_regi = 1;
	string temp;
	string *ptr;
	ptr = new string[4];
			
	for(int i = 0; i < line.size();i++)
	{
		switch(line[i])
		{
		case ':':
			temp.clear();
			break;
		case '\t':	// store instruction
			*(ptr) = temp;
			temp.clear();
			break;
		case ' ':	// store register name
			*(ptr + count_regi) = temp;
			temp.clear();
			count_regi++;
			break;
		case ',':
			break;
		default:
			temp = temp + line[i];			
			break;
		}
		
		if(i == line.size() - 1)
		{	//store finel variable
			*(ptr + count_regi) = temp;
			temp.clear();
		}	
	}
	code.push_back(ptr);	
}

void readfile() 
{
	ifstream file;
	string line;
	int datatype = 0;
	
	file.open("instruction.txt", ios::in);
	if (file.is_open())
	{
		while(getline(file, line))
		{
			if(line[0] == '.') { datatype++; }
			else if(line == "")	{ continue;	}
			else
			{
				switch(datatype)
				{
				case 1:	// num of RS
					generateRS(line);
					break;
				case 2:	// time comsum of each instruction
					assigntimecomsum(line);
					break;
				case 3:	// initiate register
					initialregister(line);
					break;
				case 4:	// instruction
					storeinstruction(line);
					break;
				default:
					break;
				}
			}
		}
		file.close();
	}
	else
	{
		cout << "Unable to open the file!" << endl;
	}
}

//=================== output file ==========================
void gettracktable()
{
	for(int i = 0; i < numcode.size(); i++)
	{
		int *ptri = new int[3];
		fortrack.push_back(ptri);
	}
}

void updatetracktable(int type, int ix)
{
	*(fortrack[ix-1] + type) = nowcycle;
} 

void outputtracktable()
{
	char output[] = "Ttrack.txt";
	fstream fp;
	fp.open(output, ios::out);
	if (!fp) 
	{
		cout << "Fail to open file! " << output << endl;
	}
	else
	{
		fp << "	" << "Iss.	" << "Dis.	" << "Wri." <<endl;
		for(int i = 0; i < numcode.size(); i++)
		{
			fp << "I" << i + 1 << "	"; 
			for(int j = 0; j < 3; j++)
			{
				fp << *(fortrack[i] + j) << "	";
			}
			fp << endl;
			delete [] fortrack[i];
		}
	}
	fp.close();
}

//===================== print status ========================

string getsign(int x)
{
	if(x == 0 || x == 2){return "+";}
	else if(x == 1 || x == 3){ return "-";}
	else if(x == 4 || x == 6){ return "*";}
	else if(x == 5 || x == 7){ return "/";}
}

void printRS(int rsx, RSX *ptrr)
{
	cout << "	" << "RS" << rsx;
	if(ptrr->occupied)
	{
		cout << "	" << getsign(ptrr->inst_type) << "	/	";
		if(ptrr->valid[0] == -1)		// have actual variable
		{
			cout << ptrr->op[0] << "	/	";
		}
		else
		{
			cout << "RS" << ptrr->op[0] << "(I" << ptrr->valid[0] << ")	/	";
		}
		if(ptrr->valid[1] == -1)
		{
			cout << ptrr->op[1];
		}
		else
		{
			cout << "RS" << ptrr->op[1] << "(I" << ptrr->valid[1] << ")"; 
		}
	}
	else 
	{
		cout << "		/		/	";
	}
	cout << endl;
}

void printALU(ALU *ptra)
{
	if(ptra->ALUoccupied)		// not empty
	{
		cout << "(RS" << ptra->alurs <<")	";
		cout << "I" << ptra->ins_num << ": ";
		//			op1						sign						op2
		cout << ptra->aluop[0] << " " << getsign(ptra->ins_type) << " " << ptra->aluop[1] << endl;
		cout << "release cycle: " << ptra->release << endl;
	}
	else						// is empty
	{
		cout << "empty" << endl;
	}
	cout << endl;
}

void printstatus()
{
	int justify = addRS.size();
	
	cout << "RS status(add):" << endl;
	for(int i = 0; i < addRS.size(); i++)
	{
		printRS(i, addRS[i]);
	}
	cout << "ALU(add): ";
	printALU(alu[0]);
	
	cout << "RS status(mul):" << endl;
	for(int j = 0; j < mulRS.size(); j++)
	{
		printRS(j + justify, mulRS[j]);
	}
	cout << "ALU(mul): ";
	printALU(alu[1]);
	cout << endl;
	
	cout << "RAT status:" << endl;
	cout << "        R0      R1      R2      R3      R4      R5      R6      R7" << endl;
	for(int k = 0; k < 8; k++)
	{
		if(RAT[k] == -1)
		{
			cout << "	empty";
		}
		else
		{
			cout << "	RS" << RAT[k]; 
		}
	}
	cout << endl << endl;
	
	cout <<"RF status:" << endl;
	cout << "        R0      R1      R2      R3      R4      R5      R6      R7" << endl;
	cout << "	" << regi[0] << "	" << regi[1] << "	" << regi[2] << "	" << regi[3];
	cout << "	" << regi[4] << "	" << regi[5] << "	" << regi[6] << "	" << regi[7] << endl << endl;
	cout << "===========================================================================" << endl;	
}

//======================= Issue =============================

void datacapture(RSX *ptrr)
{
	//op2
	if(ptrr->inst_type == 2 || ptrr->inst_type == 3 || ptrr->inst_type == 6 || ptrr->inst_type == 7)	// addi, subi, muli, divi
	{
		ptrr->valid[1] = -1;
	} 
	else									// not addi, subi, muli, divi
	{
		if(RAT[ptrr->op[1]] == -1)			// don't need to wait
		{
			ptrr->op[1] = regi[ptrr->op[1]];
			ptrr->valid[1] = -1;
		}
		else								// need to wait
		{
			ptrr->valid[1] = RATgene[ptrr->op[1]];
			ptrr->op[1] = RAT[ptrr->op[1]];
		}	
	}
	
	// op1
	if(RAT[ptrr->op[0]] == -1)				// don't have to wait
	{
		ptrr->op[0] = regi[ptrr->op[0]];
		ptrr->valid[0] = -1;
	}
	else									// have to wait
	{
		ptrr->valid[0] = RATgene[ptrr->op[0]];	// set what inst. is being wait
		ptrr->op[0] = RAT[ptrr->op[0]];
	}
	
}

bool putinI(int ix, int rsx, RSX *ptrr)
{
	if(ptrr->occupied == true)
	{
		return false;
	}
	else
	{
		ptrr->occupied = true;
		ptrr->inst_num = ix + 1;
		ptrr->inst_type = *(numcode[ix]);
		ptrr->dest = *(numcode[ix] + 1);
		ptrr->op[0] = *(numcode[ix] + 2);
		ptrr->op[1] = *(numcode[ix] + 3);
		ptrr->valid[0] = 0;
		ptrr->valid[1] = 0;
		
		datacapture(ptrr);
		
		RAT[*(numcode[ix] + 1)] = rsx;
		RATgene[*(numcode[ix] + 1)] = ix + 1;
		
		delete [] numcode[ix];
		
		return true;
	}
	
}

bool Issue(int ix)
{
	int justify = addRS.size();
	
	if(*(numcode[ix]) < 4)	//add
	{
		for(int i = 0; i < addRS.size(); i++)
		{
			if(putinI(ix, i, addRS[i]))
			{
				numinaddRS++;
				return true;
			}
		}	
	}
	else				//mul
	{
		for(int j = 0; j < mulRS.size(); j++)
		{
			if(putinI(ix, justify, mulRS[j]))
			{
				numinmulRS++;
				return true;
			}
			justify++;
		}
	}
	return false;
}

//================== Dispatch =============================

int forfirstin(RSX *ptrr, int nowissue)
{
	if(ptrr->occupied)				// not empty
	{
		if(ptrr->valid[0] == -1 && ptrr->valid[1] == -1 && ptrr->inst_num != nowissue)	// data is ready
		{
			return ptrr->inst_num;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	} 
}

int putinD(RSX *ptrr, int rsx, ALU *ptra)
{
	ptra->ALUoccupied = true;
	ptra->release = nowcycle + timecomsum[ptrr->inst_type] - 1;
	
	ptra->alurs = rsx;
	ptra->ins_num = ptrr->inst_num;
	ptra->ins_type = ptrr->inst_type;
	ptra->aluop[0] = ptrr->op[0];
	ptra->aluop[1] = ptrr->op[1];
	ptra->aludest = ptrr->dest;
	
	return ptra->ins_num;
} 

int Dispatchadd(ALU *ptra, int nowissue)
{
	int temp = 100;
	int now;
	int position = -1;
	
	if(ptra->ALUoccupied)
	{
		return -1;
	}
	
	// find earier in and data are ready
	for(int i = 0; i < addRS.size(); i++)
	{
		now = forfirstin(addRS[i], nowissue);
		if(now < temp && now > -1)
		{
			temp = now;
			position = i;
		}
	}
	if(position == -1)
	{
		return -1;
	}
	
	return putinD(addRS[position], position, alu[0]);;
}

int Dispatchmul(ALU *ptra, int nowissue)
{
	int temp = 100;
	int now;
	int position = -1;
	if(ptra->ALUoccupied)
	{
		return -1;
	}
	
	// find earier in and data are ready
	for(int i = 0; i < mulRS.size(); i++)
	{
		now = forfirstin(mulRS[i], nowissue);
		if(now < temp && now > -1)
		{
			temp = now;
			position = i;
		}
	}
	
	if(position == -1)		// no instruction is ready
	{
		return -1;
	}
	
	return putinD(mulRS[position], position + addRS.size(), alu[1]);;
}

//================== Write back ===========================

int getoutcome(int type, int op1, int op2)
{
	switch(type)
	{
	case 0:
		return op1 + op2;
	case 1:
		return op1 - op2;
	case 2:
		return op1 + op2;
	case 3:
		return op1 - op2;
	case 4:
		return op1 * op2;
	case 5:
		if(op2 == 0)
		{
			cout << "!!!!!!!! nonvalid outcome!!!!!!!!!!!" << endl;
			cout << op1 << " " << getsign(type) << " " << op2 << endl;
			noexception = false;
			return 0;
		}
		return op1 / op2;
	case 6:
		return op1 * op2;
	case 7:
		if(op2 == 0)
		{
			cout << "!!!!!!!! nonvalid outcome!!!!!!!!!!!" << endl;
			cout << op1 << " " << getsign(type) << " " << op2 << endl;
			noexception = false;
			return 0;
		}
		return op1 / op2;
	}
}

void broadcast(RSX *ptrr, int ix, int outcome)
{
	for(int i = 0; i < 2; i++)
	{
		ptrr->valid[i] = -1;
		ptrr->op[i] = outcome;
	}
}

void releaseRS(RSX *ptrr)
{
	ptrr->occupied = false;
}

bool Writeback(ALU *ptra)
{
	int outcome;
	
	if(!ptra->ALUoccupied)
	{
		return false;
	}
	
	if(ptra->release != nowcycle)
	{
		return false;
	}
	
	outcome = getoutcome(ptra->ins_type, ptra->aluop[0], ptra->aluop[1]);
	
	// update register and RAT
	RATgene[ptra->aludest] = -1;
	RAT[ptra->aludest] = -1;
	regi[ptra->aludest] = outcome;
	
	// update RS
	for(int i = 0; i < addRS.size(); i++)
	{
		broadcast(addRS[i], ptra->ins_num, outcome);
	}
	for(int j = 0; j < mulRS.size(); j++)
	{
		broadcast(mulRS[j], ptra->ins_num, outcome);
	}
	
	// release RSX
	if(ptra->ins_type < 4)		// +, -
	{
		releaseRS(addRS[ptra->alurs]);
	}
	else						// *, /
	{
		releaseRS(mulRS[ptra->alurs - addRS.size()]);
	}
	
	cout << "Write back: I" << ptra->ins_num <<"(=" << outcome << ")" << endl;
	updatetracktable(2, ptra->ins_num);
	
	ptra->ALUoccupied = false;
	
	return true;
}

//========================================================

bool isempty(ALU *ptra)
{
	if(!ptra->ALUoccupied)
	{
		return true;
	}
	return false;
}

int main(int argc, char **argv) 
{
	// control variable
	int nowissue = 0;
	int temp;
	bool shouldcout;
	bool cdbempty;
	//int limit = 70;
	
	// set and get num. inst.
	readfile();
	transinst();
	code.clear();
	gettracktable();
	
	// generate alu
	generateALU(2);
	
	while(shouldexe && noexception)
	{
		nowcycle++;
		shouldcout = false;
		
		cout << "Cycle: " << nowcycle << endl << endl;
		
		// Issue
		if(nowissue < numcode.size())
		{
			if(Issue(nowissue))
			{
				nowissue++;	
				cout << "Issue: I" << nowissue  << endl;
				updatetracktable(0, nowissue);
				shouldcout = true;	
			}
		}
		else if(nowissue == numcode.size())
		{
			nowissue++;	
		}
		
		// Dispatch
		if(numinaddRS != 0)
		{
			temp = Dispatchadd(alu[0], nowissue);
			if(temp != -1)
			{
				numinaddRS--;
				cout << "Dispatch(add): I" << temp << endl;
				updatetracktable(1, temp);
				shouldcout = true;
			}
		}
		if(numinmulRS != 0)
		{
			temp = Dispatchmul(alu[1], nowissue);
			if(temp != -1)
			{
				numinmulRS--;
				cout << "Dispatch(mul): I" << temp << endl;
				updatetracktable(1, temp);
				shouldcout = true;
			}
		}
		
		// Write back
		cdbempty = true;
		if(Writeback(alu[0]))
		{
			cdbempty = false;
			shouldcout = true;	
		}
		if(Writeback(alu[1]) && cdbempty)
		{
			shouldcout = true;
		}
		
		// print now status
		if(shouldcout)		// if issue, dispatch, write back happen in this cycle
		{
			cout << endl;
			printstatus();
		}
		else
		{
			cout << "Do nothing" << endl;
			cout << "===========================================================================" << endl;
		}
		
		// end
		if( nowissue > numcode.size() &&  numinaddRS == 0 && numinmulRS == 0 && isempty(alu[0]) && isempty(alu[1]))
		{
			outputtracktable();
			break;
		}
		
		//limit--;
		//if(limit < 0)
		//{
		//	break;
		//}
	}
	system("pause");
}
