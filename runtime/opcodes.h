/*

SYS
	sys(c, &r[a], r[b]) // c: nr, a: result

MOV
	r[a] = r[c] << b

MOVI, MOVI2
	r[a] = c << b

MVN
	r[a] = -(r[c] << b)

MVNI, MVNI2
	r[a] = -(c << b)

CMP
	z = r[b] == r[c]; n = r[b] < r[c]

CMPI, CMPI2
	z = r[b] == c; n = r[b] < c

MUL
	r[a] = r[b] * r[c]

MULI, MULI2
	r[a] = r[b] * c

DIV
	r[a] = r[b] / r[c]

DIVI, DIVI2
	r[a] = r[b] / c

MOD
	r[a] = r[b] % r[c]

MODI, MODI2
	r[a] = r[b] % c

ADD
	r[a] = r[b] + r[c]

ADDI, ADDI2
	r[a] = r[b] + c

SUB
	r[a] = r[b] - r[c]

SUBI, SUBI2
	r[a] = r[b] - c

LDB, LDW, LDB2, LDW2
	r[a] = mem[r[b] + c]

STB, STW, STB2, STW2
	mem[r[b] + c] = r[a]

POP
	r[a] = mem[r[b]]; r[b] += c

PSH
	r[b] += c; mem[r[b]] = r[a]

JUMP
	r[pc] += (c * WORD_SIZE)

CALL
	r[rt] = r[pc]; r[pc] += (c * WORD_SIZE)

BEQ
	if (z) r[pc] += (c * WORD_SIZE)

BNE
	if (!z) r[pc] += (c * WORD_SIZE)

BLS
	if (n) r[pc] += (c * WORD_SIZE)

BGE
	if (!n) r[pc] += (c * WORD_SIZE)

BLE
	if (z || n) r[pc] += (c * WORD_SIZE)

BGT
	if (!z && !n) r[pc] += (c * WORD_SIZE)

*/


/*
 * format a-b-c
 *
 * 18                 4    4    6
 * ------------------ ---- ---- ------
 * c                  b    a    oc
 *
 */

#define OC_MOV 8
#define OC_MVN 9
#define OC_CMP 10
#define OC_MUL 11
#define OC_DIV 12
#define OC_MOD 13
#define OC_ADD 14
#define OC_SUB 15

#define OC_MOVI 16
#define OC_MVNI 17
#define OC_CMPI 18
#define OC_MULI 19
#define OC_DIVI 20
#define OC_MODI 21
#define OC_ADDI 22
#define OC_SUBI 23

#define OC_LDB 24
#define OC_LDW 25
#define OC_STB 26
#define OC_STW 27

#define OC_POP 28
#define OC_PSH 29

#define OC_SYS 31

/*
 * format a-b-c32
 *
 * 18                 4    4    6         32
 * ------------------ ---- ---- ------    --------------------------------
 * unused             b    a    oc        c
 *
 */

#define OC_MOVI2 32
#define OC_MVNI2 33
#define OC_CMPI2 34
#define OC_MULI2 35
#define OC_DIVI2 36
#define OC_MODI2 37
#define OC_ADDI2 38
#define OC_SUBI2 39

#define OC_LDB2 40
#define OC_LDW2 41
#define OC_STB2 42
#define OC_STW2 43

/*
 * format c
 *
 * 26                     6
 * ---------------------- ------
 * c                      oc
 *
 */

#define OC_JUMP 48
#define OC_CALL 49
#define OC_BEQ 50
#define OC_BNE 51
#define OC_BLS 52
#define OC_BGE 53
#define OC_BLE 54
#define OC_BGT 55
