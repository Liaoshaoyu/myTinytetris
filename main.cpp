#include <ctime>
#include <curses.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

#include <iostream>
using namespace std;

// 超参数
int x = 431424,
        y = 598356,
        r = 427089,
        px = 247872,
        py = 799248,
        pr,
        c = 348480,
        p = 615696,
        tick,
        board[20][10],
        block[7][4] = {{x,      y,      x,      y},
                       {r,      p,      r,      p},
                       {c,      c,      c,      c},
                       {599636, 431376, 598336, 432192},
                       {411985, 610832, 415808, 595540},
                       {px,     py,     px,     py},
                       {614928, 399424, 615744, 428369}},
        score = 0;

// &运算得到[0, 1, 2, 3]中的一个数字，
int num(int param_x, int param_y){
//    cout<<"b-> "<<block[p][param_x]<<endl;
//    int bbb = 3 & 6;
//    cout<<"&-> "<<bbb<<endl;
//    int ppp = 3 & 7 >> 16;
//    cout<<"p-> "<<ppp<<endl;
    int offset = block[p][param_x] >> param_y;
    return 3 & offset;
}

void new_piece() {
    y = py = 0;
    p = rand() % 7;
    r = pr = rand() % 4;
    x = px = rand() % (10 - num(r, 16));
//    cout<<"======func new_piece======"<<endl;
//    cout<<"p: "<<p<<" r: "<<r<<" x: "<<x<<" func return: "<<num(r, 16)<<endl;
}

void set_piece(int param_x, int param_y, int param_r, int param_v){
    for (int i = 0; i<8; i+=2){
        int row_idx = num(param_r, i*2) + param_y;
        int col_idx = num(param_r, i*2+2) + param_x;
        board[row_idx][col_idx] = param_v;
    }

}

// 检查是否碰撞到顶部
bool check_hit(int param_x, int param_y, int param_r){
    // 触碰到顶部
    if (param_y + num(param_r, 18) > 19) {
        return true;
    }

    // board格子置零
    set_piece(px, py, pr, 0);

    c = 0;
    for (int i = 0; i < 8; i+=2){
        int row_idx = num(param_r, i*2) + param_y;
        int col_idx = num(param_r, i*2+2) + param_x;

        // 短路，与以下代码结果相同
//        if (board[row_idx][col_idx]){
//            c++;
//        }
        board[row_idx][col_idx] && c++;
    }

    // board格子赋值
    set_piece(px, py, pr, p + 1);

    return c;
}

void remove_line(){
    for(int row=y; row<=y+num(r, 18); row++){
        c = 1;
        for (int i = 0; i<10; i++){
            c *= board[row][i];
        }
        if (!c) {
            continue;
        }
        for (int i = row - 1; i > 0; i--) {
            memcpy(&board[i + 1][0], &board[i][0], 40);
        }
        memset(&board[0][0], 0, 10);
        score++;
    }
}

void update_piece() {
    set_piece(px, py, pr, 0);
    set_piece(px = x, py = y, pr = r, p + 1);
}

bool do_tick(){
    tick++;
    if (tick > 30){
        tick = 0;
        if (check_hit(x, y+1, r)){
            if (!y){
                return false;
            }

            // 消行并加分
            remove_line();

            new_piece();
        }else {
            y++;
            update_piece();
        }
    }

    // 否则，返回true
    return true;
}

void frame() {
    for (int i = 0; i < 20; i++) {
        move(1 + i, 1);
        for (int j = 0; j < 10; j++) {
            board[i][j] && attron(262176 | board[i][j] << 8);
            printw("  ");
            attroff(262176 | board[i][j] << 8);
        }
    }
    move(21, 1);
    printw("Score: %d", score);
    refresh();
}

void runloop(){
    // 死循环
    while (do_tick()){
        usleep(10000);

        char ch = getch();

        // 左移
        if ( ch == 68 && x > 0 && !check_hit(x - 1, y, r)) {
            x--;
        }

        // 右移
        if (ch == 67 && x + num(r, 16) < 9 && !check_hit(x + 1, y, r)) {
            x++;
        }

        // 快速下落
        if (ch == 66) {
            while (!check_hit(x, y + 1, r)) {
                y++;
                update_piece();
            }
            remove_line();
            new_piece();
        }

        // 转向
        if (ch == 65) {
            ++r %= 4;
            while (x + num(r, 16) > 9) {
                x--;
            }
            if (check_hit(x, y, r)) {
                x = px;
                r = pr;
            }
        }

        // 退出
        if (ch == 'q') {
            return;
        }

        // 默认更新块位置
        update_piece();

        // 产生块
        frame();
    }
}

int main() {
    srand(time(nullptr));
    initscr();
    start_color();
    // colours indexed by their position in the block
    for (int i = 1; i < 8; i++) {
        init_pair(i, i, 0);
    }
    new_piece();
    resizeterm(22, 22);
    noecho();
    timeout(0);
    curs_set(0);
    box(stdscr, 0, 0);
    runloop();
    endwin();
    return 0;
}