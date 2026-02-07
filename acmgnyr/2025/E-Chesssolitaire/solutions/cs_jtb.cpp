/*
 * Chess Solitaire - ICPC East Division Regional
 * Solution by John Buck
 *
 * It supports simple pawns that can capture on any diagonal (up or down) from their position.
 * No promotion.
 * No en passante.
 */
//#define	PLAY_DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef	unsigned long	move_t;

int board[8][8];
int nSize;
move_t movelist[64*64];

const char *szPieces = "-PNBRQK";

#define	MAKE_MOVE(p, sr, sc, dr, dc) (((p)<<16) | ((sr)<<12) | ((sc)<<8) | ((dr)<<4) | (dc))
#define	GET_PIECE(m)			((int)((m>>16) & 0xff))
#define	GET_START_ROW(m)		((int)((m>>12) & 0xf))
#define	GET_START_COL(m)		((int)((m>>8) & 0xf))
#define	GET_ROW(m)				((int)((m>>4) & 0xf))
#define	GET_COL(m)				((int)(m & 0xf))
#define	ANSWER					0x80000000

#define	GET_PIECE_CHAR(m)	szPieces[GET_PIECE(m)]

#ifdef PLAY_DEBUG
int indent = 0;
#endif

void show_board()
{
#ifdef PLAY_DEBUG
	int r, c;

	::fprintf(stderr, "%*s--------\n", indent, "");
	for(r = nSize - 1; r >= 0; r--){
		::fprintf(stderr, "%*s", indent, "");
		for(c = 0; c < nSize; c++){
			putc(szPieces[board[r][c]], stderr);
		}
		putc('\n', stderr);
	}
	putc('\n', stderr);
#endif
}

#define	check_can_move(sr, sc, r, c) if(r >= 0 && r < nSize && c >= 0 && c < nSize && board[r][c])pMoveList[nmoves++] = MAKE_MOVE(board[sr][sc], sr, sc, r, c)

int move_pawn(int r, int c, move_t *pMoveList)
{
	int rdir, cdir, nmoves = 0;

	/* Not sure which way it moves (up or down), so we'll do both? */
	for(rdir = -1; rdir < 2; rdir += 2){
		for(cdir = -1; cdir < 2; cdir += 2){
			check_can_move(r, c, r+rdir, c+cdir);
		}
	}
	return(nmoves);
}


int move_knight(int r, int c, move_t *pMoveList)
{
	int nmoves = 0;

	check_can_move(r, c, r-2,c-1);
	check_can_move(r, c, r-2, c+1);
	check_can_move(r, c, r+2, c-1);
	check_can_move(r, c, r+2, c+1);
	check_can_move(r, c, r-1, c-2);
	check_can_move(r, c, r-1, c+2);
	check_can_move(r, c, r+1, c-2);
	check_can_move(r, c, r+1, c+2);
	return(nmoves);
}

/*
 * Across up-and-down or diagonally, depending on dr, dc
 */
int line_move(int r, int c, int dr, int dc, move_t *pMoveList)
{
	int nmoves = 0;
	int p = board[r][c];
	int sr = r, sc = c;

	for(;;){
		r += dr;
		if(r < 0 || r >= nSize){
			break;
		}
		c += dc;
		if(c < 0 || c >= nSize){
			break;
		}
		if(board[r][c]){
			pMoveList[nmoves++] = MAKE_MOVE(p, sr, sc, r, c);
			break;
		}
	}
	return(nmoves);
}

int move_bishop(int r, int c, move_t *pMoveList)
{
	int nmoves = line_move(r, c, -1, -1, pMoveList);
	nmoves    += line_move(r, c, -1, 1, pMoveList+nmoves);
	nmoves    += line_move(r, c, 1, -1, pMoveList + nmoves);
	nmoves    += line_move(r, c, 1, 1, pMoveList + nmoves);
	return(nmoves);
}

int move_rook(int r, int c, move_t *pMoveList)
{
	int nmoves = line_move(r, c, -1, 0, pMoveList);
	nmoves    += line_move(r, c, 1, 0, pMoveList + nmoves);
	nmoves    += line_move(r, c, 0, -1, pMoveList + nmoves);
	nmoves    += line_move(r, c, 0, 1, pMoveList + nmoves);
	return(nmoves);
}

int move_queen(int r, int c, move_t *pMoveList)
{
	int nmoves = move_bishop(r, c, pMoveList);
	nmoves += move_rook(r, c, pMoveList + nmoves);
	return(nmoves);
}

int move_kingg(int r, int c, move_t *pMoveList)
{
	int rdir, cdir, nmoves = 0;
	for(rdir = -1; rdir < 2; rdir++){
		for(cdir = -1; cdir < 2; cdir++){
			if(rdir == 0 && cdir == 0){
				continue;
			}
			check_can_move(r, c, r+rdir, c+cdir);
		}
	}
	return(nmoves);
}

int move_null(int r, int c, move_t *pMoveList)
{
	return 0;
}

int (*pMoveFuncs[])(int r, int c, move_t *pMoveList) = {
	move_null, move_pawn, move_knight, move_bishop, move_rook, move_queen, move_kingg
};

int compareMoves(void const *pm1, void const *pm2)
{
	move_t m1 = *((move_t *)pm1);
	move_t m2 = *((move_t *)pm2);
	return(m1-m2);
}

/*
 * Recursive routine to attempt a solution with given number of pieces.
 */
bool play_solitaire(int nLeft, move_t *pMoveList)
{
	int r, c, nmoves, tmp1, tmp2, m, cap_r, cap_c;

#ifdef PLAY_DEBUG
	indent += 4;
	::fprintf(stderr, "%*s%hs: ********* nLeft=%d\n", indent, "", __FUNCTION__, nLeft);
#endif
	show_board();
	if(pMoveList > &(movelist[64*64-10])){
		::fprintf(stderr, "Yikes! Too many\n");
		exit(1);
	}

	// Only one left, we are dun.
	if(nLeft <= 1){
#ifdef PLAY_DEBUG
		indent -= 4;
#endif
		return(true);
	}
	for(r = 0; r < nSize; r++){
		for(c = 0; c < nSize; c++){
			nmoves = (*pMoveFuncs[board[r][c]])(r, c, pMoveList);
			if(nmoves){
				::qsort((void *)pMoveList, nmoves, sizeof(move_t), compareMoves);
				tmp1 = board[r][c];
				board[r][c] = 0;
				for(m = 0; m < nmoves; m++){
					cap_r = GET_ROW(pMoveList[m]);
					cap_c = GET_COL(pMoveList[m]);
					tmp2 = board[cap_r][cap_c];
					board[cap_r][cap_c] = tmp1;
					if(play_solitaire(nLeft-1, pMoveList+nmoves)){
#ifdef PLAY_DEBUG
						indent -= 4;
#endif
						pMoveList[m] |= ANSWER;
						return(true);
					}
					board[cap_r][cap_c] = tmp2;
				}
				board[r][c] = tmp1;
			}
		}
	}
#ifdef PLAY_DEBUG
	indent -= 4;
#endif
	return(false);
}

int main()
{
    int m, i, r, c;
	char szPiece[8], szLoc[8];
	const char *s;
	move_t move;

	if(::fscanf(stdin, "%d %d", &(nSize), &(m)) != 2 ||
		nSize < 2 || nSize > 8 || m < 2 || m > nSize * nSize){
		::perror("bad input");
		return 0;
	}
	for(i = 0; i < m; i++){
		if(::fscanf(stdin, "%s %s", &(szPiece[0]), &(szLoc[0])) != 2){
			::perror("piece bad input");
			return 0;
		}
		s = ::strchr(szPieces, szPiece[0]);
		if(s == NULL){
			::perror("bad piece");
			return 0;
		}
		r = szLoc[0] - 'A';
		c = szLoc[1] - '1';
		if(r < 0 || r >= nSize || c < 0 || c >= nSize){
			::perror("bad loc");
			return 0;
		}
		board[r][c] = (int)(s - szPieces);
	}
	show_board();
	if(play_solitaire(m, &(movelist[0]))){
		for(i = 0; i < sizeof(movelist)/sizeof(move_t); i++){
			move = movelist[i];
			if(move & ANSWER){
				printf("%c: %c%d -> %c%d\n", GET_PIECE_CHAR(move), GET_START_ROW(move)+'A', GET_START_COL(move)+1, GET_ROW(move) + 'A', GET_COL(move) + 1);
			}
		}
	} else {
		printf("no solution\n");
	}
	return(0);
}
