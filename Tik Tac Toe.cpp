#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>

#include <windows.h>
#include <tchar.h>

#include <cstdio>
#include <sstream>

#define STACK_SIZE (64*1024) // Размер стека для нового потока

class cell {
public:
    int leftUpX = 0;
    int leftUpY = 0;
    int rightDownX = 0;
    int rightDownY = 0;
    int value = 9;


    void writeCoordinate(int x_val, int y_val, int x_val2, int y_val2) {
        this->leftUpX = x_val;
        this->leftUpY = y_val;
        this->rightDownX = x_val2;
        this->rightDownY = y_val2;
    }

    void changeVal(int val) {
        if (value == 9) {
            value = val;
        }
    }
};

UINT myMessage = RegisterWindowMessage(L"MyCustomMessage");
UINT turnOf = RegisterWindowMessage(L"MyCustomMessageturnOf");

const int CHAT_SIZE = 1024;
TCHAR szMapName[] = _T("Test123321");
TCHAR szMapName2[] = _T("Test12332112321312");

HDC hdc;
PAINTSTRUCT ps;
COLORREF penRGB; //цвет линий
HPEN hPen; //перо для линий

std::string fileName = "Settings.txt";
LPCWSTR fileNameLP = TEXT("Settings.txt");

int colorSwitch; // параметр для красивой смены цвета

HBRUSH winBrush ; //кисть фона 

TCHAR clsName[] = _T("TicTacToe53789345");

int n ; // параметр для подсчёта клеток поля NxN
cell** field ; //создание двумерного массива-поля игры

RECT rec,rec2;


int width ;
int height ;
int onex; // одна n-ая часть от размера окна по x
int oney; // одну n-ая часть от размера окна по y

char* mem;
HANDLE hMap;

HWND hWnd;
HANDLE drawThread; // Поток для отрисовки
bool stopedThred = false; // Переменная обозначающая остановлен ли поток отрисовки
int xPos = onex; // Отрисовка квадрата
int yPos = oney;
int center = 40;
int dir = 5;
int randomDirection = 1;

HANDLE hMapTeam; 
int team;
char* teamSelect; // Отображение для выбора комманды
int turnNow=9;

void Title(int tm) {
    if (tm == team)
        SetWindowText(hWnd, L"TicTacToe           Сейчас Ваш Ход");
    else
        SetWindowText(hWnd, L"TicTacToe           Сейчас Не Ваш Ход");
}

void LiveNoWin() {
    std::string str;
    str = std::to_string(team)+teamSelect[1];
    
    strcpy((char*)teamSelect, str.c_str());
}

bool WinCheck(int player)
{
    // Проверка победы по горизонтали и вертикали
    bool rowWin;
    bool colWin;
    for (int i = 0; i < n; i++) {
        rowWin = true;
        colWin = true;
        for (int j = 0; j < n; j++) {
            if (field[j][i].value != player) {
                rowWin = false;
            }
            if (field[i][j].value != player) {
                colWin = false;
            }
        }
        if (rowWin || colWin) {
            return true; // Есть победитель
        }
    }

    // Проверка победы по диагоналям
    bool diag1Win = true;
    bool diag2Win = true;
    for (int i = 0; i < n; i++) {
        if (field[i][i].value != player) {
            diag1Win = false;
        }
        if (field[i][n-i-1].value != player) {
            diag2Win = false;
        }
    }
    if (diag1Win || diag2Win) {
        return true; // Есть победитель
    }

    return false; // Нет победителя
}

void rewriteVal() { 
    int atr;
    std::string strCheck = mem;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            atr = mem[n*i+j]-'0';
            field[i][j].changeVal(atr);
        }
    }

    if (strCheck.find("9")==std::string::npos) {
        EnableWindow(hWnd, FALSE);
        MessageBox(NULL, _T("Вы сыграли в ничью!"), _T("TicTacToe"),
            MB_OK | MB_TASKMODAL);
        PostQuitMessage(0);
    }

    if (WinCheck(0)) {
        EnableWindow(hWnd, FALSE);
        MessageBox(NULL, _T("Победили нолики!"), _T("TicTacToe"),
            MB_OK | MB_TASKMODAL);
        PostQuitMessage(0);
    }

    if (WinCheck(1)) {
        EnableWindow(hWnd, FALSE);
        MessageBox(NULL, _T("Победили крестики!"), _T("TicTacToe"),
            MB_OK | MB_TASKMODAL);
        PostQuitMessage(0);
    }
    
}

void rewriteMem() {
    std::string str = "";

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            str += std::to_string(field[i][j].value);
        }
    }

    strcpy((char*)mem, str.c_str());

    PostMessage(HWND_BROADCAST, myMessage, 0, 0);
}

std::string createStr(int type) {
    std::string str;

    switch (type)
    {
    case 0:
        str = std::to_string(3) + "\n"
            + std::to_string(0) + "\n"
            + std::to_string(63) + "\n"
            + std::to_string(255) + "\n"
            + std::to_string(255) + "\n"
            + std::to_string(0) + "\n"
            + std::to_string(0) + "\n"
            + std::to_string(320) + "\n"
            + std::to_string(240) + "\n"
            + std::to_string(0);
        return str;
    case 1:
        LOGBRUSH logBrush;
        GetObject(winBrush, sizeof(LOGBRUSH), &logBrush);
        COLORREF color = logBrush.lbColor;

        LOGPEN logPen;
        GetObject(hPen, sizeof(LOGPEN), &logPen);
        COLORREF color2 = logPen.lopnColor;

        str = std::to_string(n) + "\n"
            + std::to_string((int)GetRValue(color)) + "\n"
            + std::to_string((int)GetGValue(color)) + "\n"
            + std::to_string((int)GetBValue(color)) + "\n"
            + std::to_string((int)GetRValue(color2)) + "\n"
            + std::to_string((int)GetGValue(color2)) + "\n"
            + std::to_string((int)GetBValue(color2)) + "\n"
            + std::to_string(width) + "\n"
            + std::to_string(height) + "\n"
            + std::to_string(colorSwitch) + "\n";
        return str;
    }
}

void InpTeam() {
    std::string str = "";
    char N[30];
    std::cout << "Выберите за кого будете играть! 1-крестики  0- нолики" << std::endl;
    std::cin.getline(N, 30);

    if (N[0] == '\0') {
        team = 1;
        str = "0";
        strcpy((char*)teamSelect, str.c_str());
    }
    else {
        if (atoi(N) < 0 || atoi(N) > 1) {
            std::cout << "Некорректное значение, попробуйте ещё раз \n" << std::endl;

            return InpTeam();

        }
        else {
            if (atoi(N) == 1) {
                team = 1;
                str = "0";
                strcpy((char*)teamSelect, str.c_str());
            }
            else {
                team = 0;
                str = "1";
                strcpy((char*)teamSelect, str.c_str());
            }
        }
    }
}

int InpfileS() {
    char N[30];
    std::cout << "Ведите метод работы с файлами \n 1-При помощи отображения файлов на память \n 2-При помощи файловых переменных \n 3-При помощи потоков ввода-вывода \n 4-При помощи файловых функций" << std::endl;
    std::cin.getline(N, 30);

    if (N[0] == '\0') {
        return 3;
    }
    else {
        if (atoi(N) < 1 || atoi(N) > 4) {
            std::cout << "Неверное значение, попробуйте ещё раз\n" << std::endl;

            return InpfileS();

        }
        else {
            return atoi(N);
        }
    }
}

int Inp() {
    char N[30];
    std::cout << "Введите размер поля (NxN)" << std::endl;
    std::cin.getline(N, 30);

    if (N[0] == '\0') {
        return n;
    }
    else {
        if (atoi(N) < 1 || atoi(N) > 15) {
            std::cout << "Некорректное значение размера поля, попробуйте ещё раз \n" << std::endl;

            return Inp();

        }
        else {
            return atoi(N);
        }
    }
}

void fileSystem(int type, int operation) {

    switch (type)
    {
    case 1: { // Отображения
        if (operation == 0) { // Считывание с файла
            HANDLE fileHandle = CreateFile(fileNameLP, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (fileHandle == INVALID_HANDLE_VALUE) { // Если файла нет, создаём новый и заполняем его стандартными значениями
                std::string str1 = createStr(0);

                HANDLE fileHandle = CreateFile(fileNameLP,GENERIC_READ | GENERIC_WRITE,0, NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
                if (fileHandle == INVALID_HANDLE_VALUE) {
                    std::cout << "Не удалось создать файл." << std::endl;
                    return;
                }


                HANDLE mappingHandle = CreateFileMapping(fileHandle,NULL,PAGE_READWRITE, 0,str1.size(),NULL); 
                if (mappingHandle == NULL) {
                    std::cout << "Не удалось создать отображение файла." << std::endl; 
                    CloseHandle(fileHandle);
                    return;
                }


                LPVOID mappedData = MapViewOfFile(mappingHandle,FILE_MAP_WRITE,0,0,NULL);
                if (mappedData == NULL) {
                    std::cout << "Не удалось отобразить файл в память." << std::endl;
                    CloseHandle(mappingHandle);
                    CloseHandle(fileHandle);
                    return;
                }

                memcpy(mappedData, str1.c_str(), str1.size());

                UnmapViewOfFile(mappedData);
                CloseHandle(mappingHandle);
                CloseHandle(fileHandle);

                fileSystem(1, 0);
                return;
            }

            HANDLE mappingHandle = CreateFileMapping(fileHandle, NULL, PAGE_READONLY, 0, 0, NULL);

            LPVOID mappedData = MapViewOfFile(mappingHandle, FILE_MAP_READ, 0, 0, 0);

            char* data = reinterpret_cast<char*>(mappedData);
            std::stringstream ss(data);
            int num1;
            int numbers[10];


            // Считывание из файла
            for (int i = 0; i < 10; i++) {
                ss >> num1;
                numbers[i] = num1;
            }

            // Передаём значения во все переменные
            n = numbers[0];
            winBrush = CreateSolidBrush(RGB(numbers[1], numbers[2], numbers[3]));
            penRGB = RGB(numbers[4], numbers[5], numbers[6]);
            width = numbers[7];
            height = numbers[8];
            colorSwitch = numbers[9];

            //Очищаем память
            UnmapViewOfFile(mappedData);
            CloseHandle(mappingHandle);
            CloseHandle(fileHandle);
            DeleteObject(numbers);
        }
        else { //Запись в файл
            std::string str1 = createStr(1);

            HANDLE fileHandle = CreateFile(fileNameLP, GENERIC_READ | GENERIC_WRITE,0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (fileHandle == INVALID_HANDLE_VALUE) {
                std::cout << "Не удалось создать или открыть файл." << std::endl;
                return ;
            }


            HANDLE mappingHandle = CreateFileMapping(fileHandle,NULL, PAGE_READWRITE, 0, str1.size(),NULL);
            if (mappingHandle == NULL) {
                std::cout << "Не удалось создать отображение файла." << std::endl;
                CloseHandle(fileHandle);
                return ;
            }


            LPVOID mappedData = MapViewOfFile(mappingHandle,FILE_MAP_WRITE, 0, 0, NULL);
            if (mappedData == NULL) {
                std::cout << "Не удалось отобразить файл в память." << std::endl;
                CloseHandle(mappingHandle);
                CloseHandle(fileHandle);
                return ;
            }

            memcpy(mappedData, str1.c_str(), str1.size());

            UnmapViewOfFile(mappedData);
            CloseHandle(mappingHandle);
            CloseHandle(fileHandle);
        }

        break;
    }
    case 2: { // файловые переменные
        if (operation == 0) { // Чтение из файла
            FILE* file;

            if (fopen_s(&file, fileName.c_str(), "r") == 0) {
                int num1;
                int numbers[10];

                for (int i = 0; i < 10; i++) {  // Считывание из файла
                    fscanf_s(file, "%d", &num1);
                    numbers[i] = num1;
                }

                n = numbers[0];
                winBrush = CreateSolidBrush(RGB(numbers[1], numbers[2], numbers[3]));
                penRGB = RGB(numbers[4], numbers[5], numbers[6]);
                width = numbers[7];
                height = numbers[8];
                colorSwitch = numbers[9];

                // Закрываем файл
                DeleteObject(numbers);
                fclose(file);
            }
            else { // Если файла нет, создаём новый и заполняем его стандартными значениями

                std::string str1 = createStr(0);

                fopen_s(&file, fileName.c_str() , "wb");

                fwrite(str1.c_str(), sizeof(char), str1.size(), file);

                fclose(file);

                fileSystem(2, 0);
            }

        }
        else { //Запись в файл
            std::string str1 = createStr(1);

            FILE* file;

            fopen_s(&file, fileName.c_str(), "wb");

            fwrite(str1.c_str(), sizeof(char), str1.size(), file);

            fclose(file);
        }
        break;
    }
    case 3: { //  потоки ввода-вывода
        
        std::string str1;

        if (operation == 0) { // Чтение файла
            
            std::ifstream fileCheck(fileName.c_str());

            if (!fileCheck.is_open()) { // Если файл не нашёлся, то создаём его
                std::ofstream file1(fileName.c_str());

                str1 = createStr(0);

                file1 << str1;
                file1.close();
            }
            fileCheck.close();

            std::ifstream file(fileName.c_str());
            int numbers[10];

            for (int i = 0; i < 10; i++) {  // Считывание из файла
                file >> str1;
                numbers[i] = std::stoi(str1);
            }

            n = numbers[0];
            winBrush = CreateSolidBrush(RGB(numbers[1], numbers[2], numbers[3]));
            penRGB = RGB(numbers[4], numbers[5], numbers[6]);
            width = numbers[7];
            height = numbers[8];
            colorSwitch = numbers[9];

            // Закрываем файл
            DeleteObject(numbers);
            file.close();
        }
        else { // Запись файла
            std::ofstream file(fileName.c_str());

            str1 = createStr(1);

            file << str1;

            // Закрываем файл
            file.close();
        }
        break;
    }
    case 4: { //  файловые функции
        if (operation == 0) { // Чтение из файла
            char buffer[256];
            int numbers[10];
            int number;
            DWORD bytesRead;


            HANDLE fileHandle = CreateFile(fileNameLP,GENERIC_READ,0,NULL, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
            if (fileHandle == INVALID_HANDLE_VALUE) { // Если файла нет, то создаём файл со стандартными значениями
                DWORD bytesWrite;
                std::string str1 = createStr(0);

                HANDLE fileHandle = CreateFile(fileNameLP, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (fileHandle == INVALID_HANDLE_VALUE) {
                    std::cout << "Ошибка при открытии файла." << std::endl;
                    return;
                }

                WriteFile(fileHandle, str1.c_str(), str1.size(), &bytesWrite, NULL);

                CloseHandle(fileHandle);

                fileSystem(4, 0);

                return;
            }

            //Считываем файл 
            ReadFile(fileHandle, buffer, sizeof(buffer), &bytesRead, NULL);

            char* nextToken = nullptr;
            char* token = strtok_s(buffer, "\n", &nextToken);

            //Собираем данные в массив
            for (int i = 0; i < 10; i++) {  // Считывание из файла
               number = atoi(token);
               numbers[i] = number;
               token = strtok_s(NULL, "\n", &nextToken);
            }

            n = numbers[0];
            winBrush = CreateSolidBrush(RGB(numbers[1], numbers[2], numbers[3]));
            penRGB = RGB(numbers[4], numbers[5], numbers[6]);
            width = numbers[7];
            height = numbers[8];
            colorSwitch = numbers[9];

            CloseHandle(fileHandle);
            DeleteObject(numbers);
            DeleteObject(buffer);
        }
        else { // Запись в файл
            DWORD bytesWrite;
            std::string str1 = createStr(1);

            HANDLE fileHandle = CreateFile(fileNameLP, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (fileHandle == INVALID_HANDLE_VALUE) { 
                std::cout << "Ошибка при открытии файла." << std::endl;
                return;
            }

            WriteFile(fileHandle, str1.c_str(), str1.size(), &bytesWrite, NULL);

            CloseHandle(fileHandle);
        }
        break;
    }
    }
}

void buteRGB() {
    switch (colorSwitch)
    {
    case 0:
        penRGB += RGB(0, 17, 0);
        if (penRGB == RGB(255, 255, 0)) { colorSwitch = 1; }
        break;
    case 1:
        penRGB -= RGB(17, 0, 0);
        if (penRGB == RGB(0, 255, 0)) { colorSwitch = 2; }
        break;
    case 2:
        penRGB += RGB(0, 0, 17);
        if (penRGB == RGB(0, 255, 255)) { colorSwitch = 3; }
        break;
    case 3:
        penRGB -= RGB(0, 17, 0);
        if (penRGB == RGB(0, 0, 255)) { colorSwitch = 4; }
        break;
    case 4:
        penRGB += RGB(17, 0, 0);
        if (penRGB == RGB(255, 0, 255)) { colorSwitch = 5; }
        break;
    case 5:
        penRGB -= RGB(0, 0, 17);
        if (penRGB == RGB(255, 0, 0)) { colorSwitch = 0; }
        break;
    }
}

void rewriteCoordinates() {
    for (int i = 0; i < n; i++) { //заполняем элементы массивов координатами
        for (int j = 0; j < n; j++) {
            field[i][j].writeCoordinate((onex * i), (oney * j), (onex * (i + 1)), (oney * (j + 1)));
        }
    }
}

BOOL Line(HDC hdc, int x1, int y1, int x2, int y2)
{
    MoveToEx(hdc, x1, y1, NULL); //сделать текущими координаты x1, y1
    return LineTo(hdc, x2, y2);
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    if (msg == myMessage) {
        rewriteVal();
    }

    if (msg == turnOf) {
        turnNow = teamSelect[1] -'0';
        Title(turnNow);
    }

    switch (msg) {
    case WM_SIZE:
        GetWindowRect(hWnd, &rec2);

        GetClientRect(hWnd, &rec);

        width = (rec.right - rec.left);
        height = (rec.bottom - rec.top);
        onex = width / n; // высчитываем одну n-ую часть от размера окна по x
        oney = height / n; // высчитываем одну n-ую часть от размера окна по y

        rewriteCoordinates();

        return 0;
    case WM_LBUTTONDOWN:

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {

                std::string str;

                if (teamSelect[0] != '9') {

                    EnableWindow(hWnd, FALSE);
                    MessageBox(NULL, _T("Подождите пока зайдёт второй игрок"), _T("TicTacToe"),
                        MB_OK | MB_TASKMODAL);
                    EnableWindow(hWnd, TRUE);

                    return 0;
                }

                if (turnNow != team) {

                    EnableWindow(hWnd, FALSE);
                    MessageBox(NULL, _T("Сейчас ход другого игрока"), _T("TicTacToe"),
                        MB_OK | MB_TASKMODAL);
                    EnableWindow(hWnd, TRUE);

                    return 0;
                }

                if (LOWORD(lParam) < field[i][j].rightDownX &&
                    HIWORD(lParam) < field[i][j].rightDownY &&
                    LOWORD(lParam) > field[i][j].leftUpX &&
                    HIWORD(lParam) > field[i][j].leftUpY) {

                    field[i][j].changeVal(team);
                    rewriteMem();

                    if (team == 1) {
                        str = teamSelect[0] + std::to_string(0);
                    }
                    else {
                        str = teamSelect[0] + std::to_string(1);
                    }

                    strcpy((char*)teamSelect, str.c_str());

                    PostMessage(HWND_BROADCAST, turnOf, 0, 0);

                    return 0;
                }
            }
        }
        return 0;
    case WM_MOUSEWHEEL:

        //красивая смена цвета
        buteRGB();

        hPen = CreatePen(1, 5, penRGB);
        return 0;
    case WM_DESTROY:

        LiveNoWin();

        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        switch (wParam) {
        case 'C':

            if (GetKeyState(VK_SHIFT) & 0x8000) {
                system("notepad.exe");
            }
            return 0;

        case 'Q':
            if (GetKeyState(VK_CONTROL) & 0x8000) {
                LiveNoWin();

                PostQuitMessage(0);
            }
            return 0;

        case VK_ESCAPE: //ESC 0x1B  or  27

            LiveNoWin();

            PostQuitMessage(0);
            return 0;

        case VK_RETURN: //Enter 0x0D or 13

            DeleteObject(winBrush);

            winBrush = CreateSolidBrush(RGB((rand() % 255), (rand() % 255), (rand() % 255))); // Создаёт кисть случайного цвета

            SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)winBrush); // Заменяет цвет кисти bg  //SetClassLong(hWnd, GCL_HBRBACKGROUND, (LONG)randBrush)
            return 0;

        case '1':
            SetThreadPriority(drawThread, THREAD_PRIORITY_IDLE);
            return 0;
        case '2':
            SetThreadPriority(drawThread, THREAD_PRIORITY_LOWEST);
            return 0;
        case '3':
            SetThreadPriority(drawThread, THREAD_PRIORITY_NORMAL);
            return 0;
        case '4':
            SetThreadPriority(drawThread, THREAD_PRIORITY_ABOVE_NORMAL);
            return 0;
        case '5':
            SetThreadPriority(drawThread, THREAD_PRIORITY_TIME_CRITICAL);
            return 0;

        case VK_SPACE:
            if (!stopedThred)
            {
                SuspendThread(drawThread);
                stopedThred = true;
            }
            else {
                ResumeThread(drawThread);
                stopedThred = false;
            }

            return 0;
        }
        return 0;
    }


    return DefWindowProc(hWnd, msg, wParam, lParam);
}

DWORD WINAPI drawer(LPVOID)
{
    while (true)
    {
        InvalidateRect(hWnd, 0, TRUE);

        hdc = BeginPaint(hWnd, &ps);
        SelectObject(hdc, hPen);
        SelectObject(hdc, winBrush);

        //Фоновая Анимация

        Rectangle(hdc, xPos -center, yPos - center, xPos + center, yPos + center);

        switch (randomDirection)
        {
        case 0: // диагональ вправо+вверх
            if (xPos + center <= width && yPos- center >= 0) {
                xPos += dir;
                yPos -= dir;
            }
            else
                if(xPos + center >= width)
                    randomDirection = 3;
                else
                    randomDirection = 1;
            break;
        case 1: // диагональ вправо+вниз
            if (xPos + center <= width && yPos + center <= height) {
                xPos += dir;
                yPos += dir;
            }
            else
                if (xPos + center >= width)
                    randomDirection = 2;
                else
                    randomDirection = 0;
            break;
        case 2: // диагональ влево+вниз
            if (xPos - center >= 0 && yPos + center <= height) {
                xPos -= dir;
                yPos += dir;
            }
            else
                if (xPos - center <= 0)
                    randomDirection = 1;
                else
                    randomDirection = 3;
            break;
        case 3: // диагональ влево+вверх
            if (xPos - center >= 0 && yPos - center >= 0) {
                xPos -= dir;
                yPos -= dir;
            }
            else
                if (xPos - center <= 0)
                    randomDirection = 0;
                else
                    randomDirection = 2;
            break;
        }

        for (int i = 1; i < n; i++) { //отрисовка линий

            Line(hdc, (onex * i), 0, (onex * i), height);

            Line(hdc, 0, (oney * i), width, (oney * i));

        }

        for (int i = 0; i < n; i++) { //отрисовка элементов на поле
            for (int j = 0; j < n; j++) {
                if (field[i][j].value == 0) {
                    Ellipse(hdc,
                        field[i][j].leftUpX + (onex / 4),
                        field[i][j].leftUpY + (oney / 8),
                        field[i][j].rightDownX - (onex / 4),
                        field[i][j].rightDownY - (oney / 8));
                }
                if (field[i][j].value == 1) {
                    Line(hdc,
                        field[i][j].leftUpX + (onex / 4),
                        field[i][j].leftUpY + (oney / 8),
                        field[i][j].rightDownX - (onex / 4),
                        field[i][j].rightDownY - (oney / 8));

                    Line(hdc,
                        field[i][j].rightDownX - (onex / 4),
                        field[i][j].leftUpY + (oney / 8),
                        field[i][j].leftUpX + (onex / 4),
                        field[i][j].rightDownY - (oney / 8));
                }
            }
        }
        


        EndPaint(hWnd, &ps);

        Sleep(20);
    }
    return 0;
}

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    int fs = InpfileS();

    fileSystem(fs, 0);

    hPen = CreatePen(1, 5, penRGB);

    field = new cell * [n];
    rec = { 0 };
    rec2 = { 0 };

    hMapTeam = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0, CHAT_SIZE,
        szMapName2
    );

    hMap = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0, CHAT_SIZE,
        szMapName
    );

    mem = (char*)MapViewOfFile(
        hMap, FILE_MAP_ALL_ACCESS,
        0, 0,
        CHAT_SIZE
    );

    teamSelect = (char*)MapViewOfFile(hMapTeam, FILE_MAP_ALL_ACCESS, 0, 0, CHAT_SIZE);

    if (teamSelect[0]=='9') {
        std::cout << "Игра уже началась, вы не можете подключиться"<< std::endl;
        ExitProcess(0);
    }
    std::string strCheck = mem;

    // дескриптор текущей программы
    HINSTANCE hThis = GetModuleHandle(NULL);

    // class of the window
    WNDCLASS winCl = { 0 }; // обнуляем winCl
    winCl.lpszClassName = clsName; // имя класса
    winCl.hInstance = hThis; // дискриптор класса
    winCl.lpfnWndProc = WinProc;

    winCl.hbrBackground = winBrush;

    if (!RegisterClass(&winCl)) {
        std::cout << "RegisterClass failed! Code: " << GetLastError() << std::endl;
        return 1;
    }

    if (strCheck.length() == 0) { // Если окно не первое, то заблокируем ему ввод размера поля
        n = Inp();
        InpTeam();
    }

    hWnd = CreateWindow(
        clsName,
        _T("TicTacToe oficial"), // visible title of the window
        WS_OVERLAPPEDWINDOW, // style for window
        CW_USEDEFAULT, CW_USEDEFAULT, // x and y axes of screen
        width, height, // width and height
        HWND_DESKTOP, // descriptor of desktop
        NULL, // menu
        hThis,
        NULL // additional params
    );

    for (int i = 0; i < n; i++) { //заполняем массив поля массивами
        field[i] = new cell[n];
    }

    if (strCheck.length() > 0) { // Если окно уже есть, то считаем значения клеток
        rewriteVal();
        team = teamSelect[0]-'0';
        if (strCheck.find("1") == std::string::npos) {

            strcpy((char*)teamSelect, "91");
        }
        else {
            std::string str1="9";
            str1 += teamSelect[1];

            std::cout << str1 << std::endl;
            strcpy((char*)teamSelect, str1.c_str());
        }
        PostMessage(HWND_BROADCAST, turnOf, 0, 0);
    }
    else { // Если окно первое, то запишем в отображение что-нибудь
        rewriteMem();
        fileSystem(fs, 1); // Запишем файл,чтобы все элементы создавались с одинаковым кол.вом клеток
    }

    rewriteCoordinates();


    ShowWindow(hWnd, SW_SHOW); // show window to user

    drawThread = CreateThread(NULL, STACK_SIZE,drawer, NULL, 0, NULL); // Делаем поток для отрисовки

    BOOL bOk;
    MSG msg;
    while ((bOk = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (bOk < 0) {
            std::cout << "GetMessage failed! Code: " << GetLastError() << std::endl;
        }

        TranslateMessage(&msg); // translate scancodes to ascii and more
        DispatchMessage(&msg);
    }

    width = (rec2.right - rec2.left);
    height = (rec2.bottom - rec2.top);

    DestroyWindow(hWnd); // destroy window
    
    fileSystem(fs, 1);

    //Подчищаем память
    DeleteObject(hdc);
    DeleteObject(hPen);
    DeleteObject(winBrush);
    DeleteObject(clsName);
    DeleteObject(field);
    UnmapViewOfFile(mem);
    CloseHandle(hMap);
    CloseHandle(drawThread);
    UnmapViewOfFile(teamSelect);
    CloseHandle(hMapTeam);

    UnregisterClass(clsName, hThis); // attention! Here we use name of the class, not class itself

}
