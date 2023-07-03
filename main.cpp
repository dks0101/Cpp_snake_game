
#include "snake.h"
#include <ctime>
using namespace std;

time_t startGT;
time_t overGT;

bool writeEndAndGetInput();
void printScore(WINDOW*, int, int, int);
void printMission(WINDOW*, int, int);
void draw(WINDOW*, Snake&, char*, int, int);
void proccesInput(WINDOW*, Snake&, int);
int MAP(int x, int y);

int main()
{	
	countA = 0;
	countP = 0;
	countG = 0;
	comA = ' ';
	comB = ' ';
	comG = ' '; 
	comP = ' ';
	int sz;
	int x,y;
	x = 100;
	y = 30;
	
	initscr();                                                                                                                                                                                                                      	
	noecho();
	cbreak();

    //화면이 너무 작으면 실행하지 않음
    int xMax, yMax;
    getmaxyx(stdscr, yMax, xMax);
    if(xMax<103 || yMax<32) {
        endwin();
        std::cout << "SizeError: Terminal size is too small. (yMax>30 && xMax>90)\n";
        std::cout << "Current size: " << yMax << " " << xMax << "\n";
        std::cout << "Terminating Process...\n";
        return 0;
    }
	
 	start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK); 
	mvprintw(y/2+1,x/2-10,"press any button to start...");
	refresh();

	getch();
	
	endwin();

	WINDOW *win = newwin(y+2, x-38, 0, 0); //height, width, startY, startX
	
	wbkgd(win, COLOR_PAIR(1));
    wattron(win, COLOR_PAIR(1));
	box(win, 0, 0);
	
	nodelay(win, TRUE);
	keypad(win, TRUE);

	WINDOW *score = newwin(y/2+1, x-60, 0,x-37);
	wbkgd(score, COLOR_PAIR(1));
    wattron(score, COLOR_PAIR(1));
	box(score, 0, 0);
	wrefresh(score);
	nodelay(score, TRUE);

	WINDOW *mission = newwin(y/2+1, x-60, y/2+1,x-37);
	wbkgd(mission, COLOR_PAIR(1));
    wattron(mission, COLOR_PAIR(1));
	box(mission, 0, 0);
	wrefresh(mission);
	nodelay(mission, TRUE);


	do
	{	
		startGT = time(NULL);
		Snake snake(y, x-40);
		refresh();
		wrefresh(win);
		sz = snake.getBodySize();
		printScore(score, 0, 1,sz);
		printMission(mission,0,1);
		countA = 0;
		countP = 0;
		countG = 0;

		//핵심 루프
		while(!snake.getExit())
		{
			char *tbl = snake.checkMap();

			//프레임 업데이트
			draw(win, snake, tbl, snake.getHeight(), snake.getWidth());			
			printScore(score, snake.getPoints(), snake.getLevel(), snake.getBodySize());
			printMission(mission, snake.getPoints(), snake.getLevel());
			
			int input = wgetch(win); //유저입력
			proccesInput(win, snake, input);
			
			if(snake.getExit())break;
			snake.checkPoints();
			snake.bodyMove();
			snake.makeMove();
			snake.checkForApple();
			snake.checkForPoison();
			snake.checkForwall();
			snake.checkGate();
			snake.changeGate();
			snake.mission();
			snake.getSize(); // 길이 3미만 체크
			usleep(200*1000); // 강제딜레이 0.2초
		}	

	}while(writeEndAndGetInput());
	
	delwin(score);
	delwin(win);
	delwin(mission);
	endwin();
}

//게임오버 실행문
bool writeEndAndGetInput()
{	
	overGT = time(NULL);
	int score = countA + countB - countP + countG*2;
	WINDOW* endwin = newwin(32,110, 0, 0);
	box(endwin, 0, 0);
	nodelay(endwin, TRUE);
	keypad(endwin, TRUE);
	mvwprintw(endwin, 14, 40, "GAME OVER");
	mvwprintw(endwin, 18, 40, "q to quit.");
	int c;
	do{
		c = wgetch(endwin);
	}while(c!=10 && c!=' ' && c!='q' && c!='Q');
	werase(endwin);
	wrefresh(endwin);
	delwin(endwin);
	return (c=='q' || c=='Q')?false:true;
}

//스코어보드 업데이트 함수
void printScore(WINDOW* w, int score, int level, int sz)
{
	werase(w);
	wbkgd(w, COLOR_PAIR(1));
    wattron(w, COLOR_PAIR(1));
	box(w, 0, 0);
	mvwprintw(w, 0, 14, "Score Board"); 
    mvwprintw(w, 4, 1, "B: %d/%d",sz,missionB);
    mvwprintw(w, 6, 1, "+: %d",countA ); 
    mvwprintw(w, 8, 1, "-: %d",countP); 
    mvwprintw(w, 10, 1, "G: %d",countG); 
	wrefresh(w);
}

//미션보드 업데이트 함수
void printMission(WINDOW* w, int score, int level)
{
	werase(w);
	wbkgd(w, COLOR_PAIR(1));
    wattron(w, COLOR_PAIR(1));
	box(w, 0, 0);
    mvwprintw(w, 0, 14, "Mission Board"); 
    mvwprintw(w, 4, 1, "B: %d   (%c)",missionB,comB); 
    mvwprintw(w, 6, 1, "+: %d   (%c)",missionA,comA); 
    mvwprintw(w, 8, 1, "-: %d   (%c)",missionP,comP); 
    mvwprintw(w, 10, 1, "G: %d   (%c)",missionG,comG); 
	wrefresh(w);
}

//프레임 업데이트 함수. 게임보드 전체를 다시 그려줌
void draw(WINDOW* win, Snake& snake, char* table, int height, int width)
{
	werase(win);
	wbkgd(win, COLOR_PAIR(1));
    wattron(win, COLOR_PAIR(1));
	box(win, 0, 0);
	int ch;
	
	//table에 담긴 문자에 따라 객체 그려줌
	for(int i=0; i<(height*width); ++i)
	{
		if(table[i]!=' ')
		{
			int y = i/width;
			int x = i-(y*width);
			
			int d;
			switch(table[i])
			{
				case 'a':
					ch = '+';
					break;
				case 'x':
					ch = '-';
					break;
				case 'h':
					
					ch = 'H';
				
					break;
				case 'b':
					ch = 'O';
					break;

				case '1':
					ch = 'W';
					break;

				case '2':
					ch = 'M';
					break;
				case '3':
					ch = ' ';
					break;
				case 'G':
					ch = ' ';
					break;
			}
			mvwaddch(win, 1+y,1+x, ch);
		}
		
	}
	wrefresh(win);
}

//방향키 입력 받기
void proccesInput(WINDOW* win, Snake& snake, int input)
{
	int d = snake.getDirection();
	switch(input)
	{
		case KEY_UP:
			if(d == 2) {
				snake.setExit(true);
				break;}
			if(d!=0 && !(d==2 && snake.getPoints() > 0))
			snake.setDirection(0);
			break;
		case KEY_DOWN:
			if(d == 0) {
				snake.setExit(true);
				break;}
			if(d!=2 && !(d==0 && snake.getPoints() > 0))
			snake.setDirection(2);
			break;
		case KEY_LEFT:
			if(d == 1) {
				snake.setExit(true);
				break;}
			if(d!=3 && !(d==1 && snake.getPoints() > 0))
			snake.setDirection(3);
			break;
		case KEY_RIGHT:
			if(d == 3) {
				snake.setExit(true);
				break;}
			if(d!=1 && !(d==3 && snake.getPoints() > 0))
			snake.setDirection(1);
			break;
		case 'Q':
		case 'q':
			snake.setExit(true);
			break;
		
		default:
			break;
	}
}