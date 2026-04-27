/*
 * Same as esoteric WhiteSpace language with exceptions
 * Numbers limited to unsigned 31 bits + sign, IE +- 2^31-1
 * Labels limited to 64 chars
 * Space=0, Tab=1, LF=2
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>
using namespace std;
#ifndef WIN32
#define	scanf_s	scanf
#endif

//#define	DEBUG
//#define		SUBDEBUG
//#define		SUBDEBUG1
//#define		STORE_DEBUG
//#define		IO_DEBUG

#define	MY_STACK_SIZE	1024
unsigned int mystack[MY_STACK_SIZE];
unsigned int *mysp = &(mystack[MY_STACK_SIZE]);

const char *mysubstack[MY_STACK_SIZE];
const char **mysubsp = &(mysubstack[MY_STACK_SIZE]);

#define	MY_PROG_SIZE	8192
char myprog[MY_PROG_SIZE+1];
const char *mypc;

#define	MAX_LABEL		128
map<string, int> maLabels;
char szLabel[MAX_LABEL+1];

#define	MAX_NUM	31
char szNum[MAX_NUM+1];

map<int, int> maHeap;

int ExecStackPositiveNumber(const char *&pc, int scan);
int ExecStackNegativeNumber(const char *&pc, int scan);
int ExecStackDuplicate(const char *&pc, int scan);
int ExecStackSwap(const char *&pc, int scan);
int ExecStackDiscard(const char *&pc, int scan);

int ExecAdd(const char *&pc, int scan);
int ExecSub(const char *&pc, int scan);
int ExecMul(const char *&pc, int scan);
int ExecDiv(const char *&pc, int scan);
int ExecMod(const char *&pc, int scan);

int ExecStore(const char *&pc, int scan);
int ExecRetrieve(const char *&pc, int scan);

int ExecLabel(const char *&pc, int scan);
int ExecCall(const char *&pc, int scan);
int ExecJump(const char *&pc, int scan);
int ExecJZ(const char *&pc, int scan);
int ExecJN(const char *&pc, int scan);
int ExecReturn(const char *&pc, int scan);
int ExecEnd(const char *&pc, int scan);

int ExecOutChar(const char *&pc, int scan);
int ExecOutNum(const char *&pc, int scan);
int ExecInChar(const char *&pc, int scan);
int ExecInNum(const char *&pc, int scan);

#define	MAX_OPCODE_LEN	4

struct SOpCodes {
	const char *m_szOp;
	int m_nOpLen;
	int (*m_pExec)(const char *&pc, int scan);
} opCodes[] = {
	{ "000", 3, ExecStackPositiveNumber },
	{ "001", 3, ExecStackNegativeNumber },
	{ "020", 3, ExecStackDuplicate },
	{ "021", 3, ExecStackSwap },
	{ "022", 3, ExecStackDiscard },

	{ "1000", 4, ExecAdd },
	{ "1001", 4, ExecSub },
	{ "1002", 4, ExecMul },
	{ "1010", 4, ExecDiv },
	{ "1011", 4, ExecMod },

	{ "110", 3, ExecStore },
	{ "111", 3, ExecRetrieve },

	{ "200", 3, ExecLabel },
	{ "201", 3, ExecCall },
	{ "202", 3, ExecJump },
	{ "210", 3, ExecJZ },
	{ "211", 3, ExecJN },
	{ "212", 3, ExecReturn },
	{ "222", 3, ExecEnd },

	{ "1200", 4, ExecOutChar },
	{ "1201", 4, ExecOutNum },
	{ "1210", 4, ExecInChar },
	{ "1211", 4, ExecInNum }
};
#define	NUM_OPCODES	(sizeof(opCodes)/sizeof(SOpCodes))

const char *FindLabel(const char *pc, const char *szLab)
{
	const char *argpc;
	int i;
	map<string, int>::iterator it;

	/*
	 * First, look up in hash table, if it's there we're done
	 */
	it = maLabels.find(szLab);
	if(it != maLabels.end()){
#ifdef SUBDEBUG2
		fprintf(stderr, "FindLabels: found label %s in hash tab at PC=%d\n", szlab, pc - &(myprog[0]));
#endif
		return(&(myprog[it->second]));
	}

	/*
	 * Find all labels - we pretend to execute, but really just looking
	 */
	for(; *pc != '\0' && *pc != '\n'; ){
		for(i = 0; i < NUM_OPCODES; i++){
			if(::strncmp(pc, opCodes[i].m_szOp, opCodes[i].m_nOpLen) == 0){
				argpc = pc + opCodes[i].m_nOpLen;
				if((*opCodes[i].m_pExec)(argpc, 1) == 0){
#ifdef SUBDEBUG1
					fprintf(stdout, "RUN-TIME ERROR PC=%d OPC=%.*s\n",
						pc - &(myprog[0]), opCodes[i].m_nOpLen, pc);
#endif
					return(NULL);
				}
				/*
				 * See if label was added to hash table.  We could restrict this to only
				 * checking if the opcode is "200", but the hash table lookup is fast
				 */
				it = maLabels.find(szLab);
				if(it != maLabels.end()){
#ifdef SUBDEBUG1
					fprintf(stderr, "FindLabels: found label %s at PC=%d\n", szLab, pc - &(myprog[0]));
#endif
					return(&(myprog[it->second]));
				}
				pc = argpc;
				break;
			}
		}
		if(i >= NUM_OPCODES){
#ifdef SUBDEBUG1
			fprintf(stdout, "RUN-TIME ERROR OPCODE not found\n");
#endif
			return(NULL);
		}
	}
#ifdef SUBDEBUG1
	fprintf(stderr, "RUN-TIME ERROR - label %s not found\n", szLab);
#endif
	return(NULL);
}

int main(int argc, char **argv)
{
	const char *argpc;
	char *s;
	int no, c, i, nr;

	no = 0;
	s = &(myprog[0]);

	/*
	 * Read entire program into memory
	 */
	while((c = getc(stdin)) != '\n' && no < MY_PROG_SIZE){
		*s++ = c;
		no++;
	}
	if(no > MY_PROG_SIZE){
		fprintf(stdout, "TOO BIG\n");
		return(1);
	}
	*s = '\0';

	/*
	 * Now run it
	 */
	for(mypc = &(myprog[0]); *mypc != '\0' && *mypc != '\n'; ){
		for(i = 0; i < NUM_OPCODES; i++){
			if(::strncmp(mypc, opCodes[i].m_szOp, opCodes[i].m_nOpLen) == 0){
				argpc = mypc + opCodes[i].m_nOpLen;
				if((nr = (*opCodes[i].m_pExec)(argpc, 0)) == 0){
#ifdef DEBUG
					fprintf(stdout, "RUN-TIME ERROR PC=%d OPC=%.*s\n",
						mypc - &(myprog[0]), opCodes[i].m_nOpLen, mypc);
#else
					fprintf(stdout, "RUN-TIME ERROR\n");
#endif
					return(0); // changed this line -- my validator must have program return 0
				}
				/* End of program? */
				if(nr == 2){
					return(0);
				}
				mypc = argpc;
				break;
			}
		}
		if(i >= NUM_OPCODES){
			fprintf(stdout, "RUN-TIME ERROR\n");
			break;
		}
	}
	return 0;
}

int GetLabel(const char *&pc, char *szLabel, int nMax)
{
	char *p;
	int n = 0;

	for(p = szLabel; n < nMax && (*pc == '0' || *pc == '1'); n++, pc++){
		*p++ = *pc;
	}
	/* Label does not end with '2' */
	if(*pc != '2'){
		return(0);
	}
	pc++;
	*p ='\0';
	return(1);
}

int PushStack(int n)
{
	if(mysp == &(mystack[0])){
		return(0);
	}
	*--mysp = n;
	return(1);
}

int PopStack(int &n)
{
	if(mysp == &(mystack[MY_STACK_SIZE])){
		return(0);
	}
	n = *mysp++;
	return(1);
}

int PeekStack(int &n)
{
	if(mysp == &(mystack[MY_STACK_SIZE])){
		return(0);
	}
	n = *mysp;
	return(1);
}

int ExecStackPositiveNumber(const char *&pc, int scan)
{
	int n = 0;
	int i;

	if(GetLabel(pc, &(szNum[0]), MAX_NUM) == 0){
		return(0);
	}
	if(scan == 0){
		for(i = 0; szNum[i] != '\0'; i++){
			n <<= 1;
			if(szNum[i] == '1'){
				n |= 1;
			}
		}
		if(PushStack(n) == 0){
			return(0);
		}
#ifdef DEBUG
		fprintf(stderr, "Push %d\n", n);
#endif
	}
	return(1);
}

int ExecStackNegativeNumber(const char *&pc, int scan)
{
	int n = 0;
	int i;

	if(GetLabel(pc, &(szNum[0]), MAX_NUM) == 0){
		return(0);
	}
	if(scan == 0){
		for(i = 0; szNum[i] != '\0'; i++){
			n <<= 1;
			if(szNum[i] == '1'){
				n |= 1;
			}
		}
		n = -n;
		if(PushStack(n) == 0){
			return(0);
		}
#ifdef DEBUG
		fprintf(stderr, "Push %d\n", n);
#endif
	}
	return(1);
}

int ExecStackDuplicate(const char *&pc, int scan)
{
	int n;

	if(scan == 0){
		if(PeekStack(n) == 0){
			return(0);
		}
		if(PushStack(n) == 0){
			return(0);
		}
#ifdef DEBUG
		fprintf(stdout, "ExecStackDuplicate: dup %d\n", n);
#endif
	}
	return(1);
}

int ExecStackSwap(const char *&pc, int scan)
{
	int n1, n2;

	if(scan == 0){
		if(PopStack(n1) == 0 || PopStack(n2) == 0){
			return(0);
		}
		if(PushStack(n1) == 0 || PushStack(n2) == 0){
			return(0);
		}
#ifdef DEBUG
		fprintf(stdout, "ExecStackSwap: swap %d %d\n", n1, n2);
#endif
	}
	return(1);
}

int ExecStackDiscard(const char *&pc, int scan)
{
	int n;

	if(scan == 0){
		if(PopStack(n) == 0){
#ifdef DEBUG
			fprintf(stdout, "ExecStackDiscard: discard %d\n", n);
#endif
			return(0);
		}
	}
	return(1);
}


int ExecAdd(const char *&pc, int scan)
{
	int n1, n2;

	if(scan == 0){
		if(PopStack(n1) == 0 || PopStack(n2) == 0){
			return(0);
		}
		if(PushStack(n1+n2) == 0){
			return(0);
		}
#ifdef DEBUG
		fprintf(stdout, "ExecAdd: Adding %d and %d to push %d\n", n1, n2, n1+n2);
#endif
	}
	return(1);
}

int ExecSub(const char *&pc, int scan)
{
	int n1, n2;

	if(scan == 0){
		if(PopStack(n1) == 0 || PopStack(n2) == 0){
			return(0);
		}
		if(PushStack(n2-n1) == 0){
			return(0);
		}
#ifdef DEBUG
		fprintf(stdout, "ExecSub: Subtract %d - %d to push %d\n", n2, n1, n2-n1);
#endif
	}
	return(1);
}

int ExecMul(const char *&pc, int scan)
{
	int n1, n2;

	if(scan == 0){
		if(PopStack(n1) == 0 || PopStack(n2) == 0){
			return(0);
		}
		if(PushStack(n1*n2) == 0){
			return(0);
		}
#ifdef DEBUG
		fprintf(stdout, "ExecMul: Multiplying %d and %d to push %d\n", n1, n2, n1*n2);
#endif
	}
	return(1);
}

int ExecDiv(const char *&pc, int scan)
{
	int n1, n2;

	if(scan == 0){
		if(PopStack(n1) == 0 || PopStack(n2) == 0){
			return(0);
		}
		if(PushStack(n2/n1) == 0){
			return(0);
		}
#ifdef DEBUG
		fprintf(stdout, "ExecDiv: Divide %d by %d to push %d\n", n2, n1, n2/n1);
#endif
	}
	return(1);
}

int ExecMod(const char *&pc, int scan)
{
	int n1, n2;

	if(scan == 0){
		if(PopStack(n1) == 0 || PopStack(n2) == 0){
			return(0);
		}
		if(PushStack(n2%n1) == 0){
			return(0);
		}
#ifdef DEBUG
		fprintf(stdout, "ExecMod: Nod %d by %d to push %d\n", n2, n1, n2%n1);
#endif
	}
	return(1);
}


int ExecStore(const char *&pc, int scan)
{
	int nval, naddr;
	if(scan == 0){
		if(PopStack(nval) == 0 || PopStack(naddr) == 0){
			return(0);
		}
		maHeap[naddr] = nval;
#ifdef STORE_DEBUG
		fprintf(stderr, "ExecStore: Stored %d at addr %d\n", nval, naddr);
#endif
	}
	return(1);
}

int ExecRetrieve(const char *&pc, int scan)
{
	int naddr, nval;
	map<int, int>::iterator it;

	if(scan == 0){
		if(PopStack(naddr) == 0){
			return(0);
		}
		it = maHeap.find(naddr);
		if(it == maHeap.end()){
			nval = 0;
		} else {
			nval = it->second;
		}
		if(PushStack(nval) == 0){
			return(0);
		}
#ifdef STORE_DEBUG
		fprintf(stderr, "ExecRetrieve: Retrieved %d from heap at addr %d\n", nval, naddr);
#endif
	}
	return(1);
}


int ExecLabel(const char *&pc, int scan)
{
	map<string, int>::iterator it;
	int off;

	if(GetLabel(pc, &(szLabel[0]), MAX_LABEL) == 0){
		return(0);
	}
	/* This is the mark location offset in program */
	off = pc - &(myprog[0]);

	/*
	 * First, look up in hash table, if it's there, we have a problem.
	 */
	it = maLabels.find(&(szLabel[0]));
	if(it != maLabels.end()){
		if(it->second != off){
#ifdef SUBDEBUG2
			fprintf(stderr, "ExecLabel: multiply defined label %s in hash tab at PC=%d\n", szlab, pc - &(myprog[0]));
#endif
			return(0);
		}
		/* As long as location is the same, we're good */
		return(1);
	}
#ifdef SUBDEBUG1
	fprintf(stderr, "ExecLabel: Added label \"%s\" at offset %d\n", &(szLabel[0]), pc - &(myprog[0]));
#endif
	maLabels[&(szLabel[0])] = off;
	return(1);
}

int ExecCall(const char *&pc, int scan)
{
	char szLabel[MAX_LABEL+1];

	if(GetLabel(pc, &(szLabel[0]), MAX_LABEL) == 0){
		return(0);
	}
	if(scan == 1){
#ifdef SUBDEBUG
		fprintf(stderr, "ExecCall: Calling label \"%s\" at offset %d\n", &(szLabel[0]), pc - &(myprog[0]));
#endif
	} else {
		if(mysubsp == &(mysubstack[0])){
#ifdef SUBDEBUG
			fprintf(stderr, "ExecCall: Call stack full\n");
#endif
			return(0);
		}
		*--mysubsp = pc;
		if((pc = FindLabel(pc, &(szLabel[0]))) == NULL){
#ifdef SUBDEBUG
			fprintf(stderr, "ExecCall: Can't find label %s\n", &(szLabel[0]));
#endif
			return(0);
		}
#ifdef SUBDEBUG
		fprintf(stderr, "ExecCall: Calling subroutine label %s offset %d\n",
			&(szLabel[0]), pc - &(myprog[0]));
#endif
	}
	return(1);
}

int ExecJump(const char *&pc, int scan)
{
	char szLabel[MAX_LABEL+1];

	if(GetLabel(pc, &(szLabel[0]), MAX_LABEL) == 0){
		return(0);
	}
	if(scan == 1){
#ifdef DEBUG
		fprintf(stderr, "ExecJump: Jump to label \"%s\" at offset %d\n", &(szLabel[0]), pc - &(myprog[0]));
#endif
	} else {
		if((pc = FindLabel(pc, &(szLabel[0]))) == NULL){
#ifdef SUBDEBUG
			fprintf(stderr, "ExecJump: Can't find label %s\n", &(szLabel[0]));
#endif
			return(0);
		}
#ifdef SUBDEBUG
		fprintf(stderr, "ExecJump: Calling subroutine label %s offset %d\n",
			&(szLabel[0]), pc - &(myprog[0]));
#endif
	}
	return(1);
}

int ExecJZ(const char *&pc, int scan)
{
	int n;
	char szLabel[MAX_LABEL+1];

	if(GetLabel(pc, &(szLabel[0]), MAX_LABEL) == 0){
		return(0);
	}
	if(scan == 1){
#ifdef DEBUG
		fprintf(stderr, "ExecJZ: Jump Zero to label \"%s\" at offset %d\n", &(szLabel[0]), pc - &(myprog[0]));
#endif
	} else {
		if(PopStack(n) == 0){
			return(0);
		}
		if(n == 0){
			if((pc = FindLabel(pc, &(szLabel[0]))) == NULL){
#ifdef DEBUG
				fprintf(stderr, "ExecJZ: Can't find label %s\n", &(szLabel[0]));
#endif
				return(0);
			}
#ifdef DEBUG
			fprintf(stderr, "ExecJZ: Jumping to label %s offset %d since top of stack is 0\n",
				&(szLabel[0]), pc - &(myprog[0]));
#endif
		}
	}
	return(1);
}

int ExecJN(const char *&pc, int scan)
{
	int n;
	char szLabel[MAX_LABEL+1];

	if(GetLabel(pc, &(szLabel[0]), MAX_LABEL) == 0){
		return(0);
	}
	if(scan == 1){
#ifdef DEBUG
		fprintf(stderr, "ExecJN: Jump Zero to label \"%s\" at offset %d\n", &(szLabel[0]), pc - &(myprog[0]));
#endif
	} else {
		if(PopStack(n) == 0){
			return(0);
		}
		if(n < 0){
			if((pc = FindLabel(pc, &(szLabel[0]))) == NULL){
#ifdef DEBUG
				fprintf(stderr, "ExecJN: Can't find label %s\n", &(szLabel[0]));
#endif
				return(0);
			}
#ifdef DEBUG
			fprintf(stderr, "ExecJN: Jumping to label %s offset %d since top of stack is negative\n",
				&(szLabel[0]), pc - &(myprog[0]));
#endif
		}
	}
	return(1);
}

int ExecReturn(const char *&pc, int scan)
{
	int off;

	if(scan == 0){
		if(mysubsp == &(mysubstack[MY_STACK_SIZE])){
#ifdef SUBDEBUG
			fprintf(stderr, "ExecReturn: Empty stack\n");
#endif
			return(0);
		}
		off = pc - &(myprog[0]);
		pc = *mysubsp++;
#ifdef SUBDEBUG
		fprintf(stderr, "ExecReturn: Returning to offset %d from %d\n", pc - &(myprog[0]), off);
#endif
	}
	return(1);
}

int ExecEnd(const char *&pc, int scan)
{
	if(scan == 0){
#ifdef DEBUG
		fprintf(stderr, "ExecEnd: End of program\n");
#endif
	}
	return(2);
}


int ExecOutChar(const char *&pc, int scan)
{
	int n;

	if(scan == 0){
		if(PopStack(n) == 0){
			return(0);
		}
		putc(n, stdout);
		fflush(stdout);
	}
	return(1);
}

int ExecOutNum(const char *&pc, int scan)
{
	int n;

	if(scan == 0){
		if(PopStack(n) == 0){
			return(0);
		}
		fprintf(stdout, "%d", n);
		fflush(stdout);
	}
	return(1);
}

int ExecInChar(const char *&pc, int scan)
{
	int c, naddr;

	if(scan == 0){
		c = getc(stdin);
		if(PopStack(naddr) == 0){
			return(0);
		}
#ifdef IO_DEBUG
		fprintf(stderr, "ExecInChar: Read char 0x%x and stored at addr %d\n", c, naddr);
#endif
		maHeap[naddr] = c;
	}
	return(1);
}

int ExecInNum(const char *&pc, int scan)
{
	int nval, naddr;

	if(scan == 0){
		nval = 0;
		scanf_s("%d", &(nval));
		if(PopStack(naddr) == 0){
			return(0);
		}
#ifdef IO_DEBUG
		fprintf(stderr, "ExecInNum: Read num %d and stored at addr %d\n", nval, naddr);
#endif
		maHeap[naddr] = nval;
	}
	return(1);
}


