#include <iostream>
#include <pthread.h>
#include <queue>
#include <unistd.h>
#include <vector>
#include <semaphore.h>
#include <fstream>

std::queue<int> guests;             // Очередь из гостей.
sem_t semaphore;                    // Семафора.
std::ofstream fout("output.txt"); // Файл для вывода.

struct Room {                       // Структура комнаты. Хранит ее номер. Нужен для понимания
    Room (int number) {             // какой поток работает.
        this->number = number;
    }

    int number;
};

void* rest_in_hotel(void* args) {   // Метод, описывающий работу потока. Получает на вход комнату.
    Room* room = (Room*)args;       // Переводим полученную в виде void* комнату в настоящую комнату.
    while (!guests.empty()) {       // Начало работы потока. guests - "портфель задач", потоки будут работать
        sem_wait(&semaphore);   // пока он не кончится. Здесь я ставлю семафору, чтобы другие потоки не
        int days = guests.front();   // работали, пока работает этот поток. (Нужно для красивого вывода и отсутствия
        if (guests.empty()) {        // путаницы. Здесь я еще раз проверяю, не стал ли портфель за время работы пустым.
            sem_post(&semaphore);   // Если он стал пустым, семафору можно убрать и вернуть nullptr - работать
            return nullptr;              // больше не нужно.
        }
        guests.pop();                    // Убираем гостя, что получил комнату, из портфеля.
        fout << "Guest check in the room number " << room->number << std::endl; // Выводим еще и в файл.
        std::cout << "Guest check in the room number " << room->number << std::endl;
        while (days != 0) {              // Начинаем отсчитывать дни.
            fout << "Guest in the room number " << room->number << " has " << days << " days left\n";
            std::cout << "Guest in the room number " << room->number << " has " << days << " days left\n";
            usleep(100000);              // Чтобы поток не работал слишком быстро, замедляем его немного.
            days--;
        }
        fout << "Guest check out room number " << room->number << std::endl;
        std::cout << "Guest check out room number " << room->number << std::endl;
        sem_post(&semaphore);       // Работа с гостем закончена, отключаем семафор.
    }
    return nullptr;
}

void console_work() {                   // Ввод гостей с консоли.
    std::cout << "Input the number of guests\n";
    int number = -1;
    while (number < 0) {                // Проверка, что число введено верно.
        try {
            std::cin >> number;
            if (number < 0)
                std::cout << "Number of guests can't be less than 0\n";
        } catch (std::exception) {
            std::cout << "Wrong input. Try again\n";
        }
    }
    std::cout << "Input the amount of rest days for every guest\n";
    for (int i = 1; i < number + 1; ++i) {          // Вводим дни отдыха каждого гостя и проверяем, верные ли они.
        int days = -1;
        while (days < 1 || days > 31) {
            try {
                std::cin >> days;
                if (days < 1)
                    std::cout << "The amount of days can't be less than 1\n";
                if (days > 31)
                    std::cout << "The amount of days can't be more than 31\n";
            } catch (std::exception) {
                std::cout << "Wrong input. Try again\n";
            }
        }
        guests.push(days);          // Полученный результат кладем в портфель задач.
    }
}

void file_input() {                    // Ввод данных с файла.
    std::ifstream fin("input.txt");  // Файл для ввода
    int number;

    try {                               // Проверка числа.
        fin >> number;
    } catch (std::exception) {
        std::cout << "Wrong input. Change the file\n";
        return;
    }

    if (number < 0)
        throw std::runtime_error("Wrong input. Change the file\n");

    for (int i = 1; i < number + 1; ++i) {
        int days;
        try {
            fin >> days;
        } catch (std::exception) {
            std::cout << "Wrong input. Change the file\n";
            return;
        }
        if (days < 1 || days > 31)
            throw std::runtime_error("Wrong input. Change the file\n");
        guests.push(days);          // Полученный результат кладем в портфель задач.
    }
}

void automatic_input() {               // Автоматический ввод данных с рандомным определением кол-ва дней.
    std::cout << "Input the number of guests\n";
    int number = -1;
    while (number < 0) {               // Проверяем, верно ли количество гостей.
        try {
            std::cin >> number;
            if (number < 0)
                std::cout << "Number of guests can't be less than 0\n";
        } catch (std::exception) {
            std::cout << "Wrong input. Try again\n";
        }
    }
    srand (time(0));        // Настраиваем рандомайзер.
    for (int i = 0; i < number; i++) {
        guests.push(rand() % 30 + 1);       // Получаем случайное число и кладем в портфель. Прибавленная
    }                                          // единица не дает получить 0.
}

void model() {                        // Выбор типа ввода.
    while (true) {                    // Цикл нужен на случай неверного ввода.
        std::cout << "Choose the input type\n1 - console input\n2 - file input\n3 - automatic random input\n";
        std::string choice;
        std::cin >> choice;
        if (choice == "1") {
            console_work();           // Ввод с консоли.
            return;
        } else if (choice == "2") {
            file_input();             // Ввод с файла.
            return;
        } else if (choice == "3") {
            automatic_input();        // Автоматический ввод.
            return;
        } else {
            std::cout << "Wrong input. Try again";
        }
    }
}

int main(int argc, char* argv[]) {
    sem_init(&semaphore, 0, 1);         // Инициализация семафоры.
    std::vector<Room> rooms;                              // Вектор комнат. Комнаты всегда генерируются одинаково.
    for (int i = 1; i < 31; i++) {                        // У комнат есть порядковые номера для удобства вывода.
        Room room = Room(i);
        rooms.push_back(room);
    }

    if (argc == 1) {                                      // Проверка на наличие командной строки.
        try {
            model();                                              // Запускаем выбор ввода данных.
        } catch (std::runtime_error e) {
            std::cout << e.what();
            return 0;
        }
    } else {
        for (int i = 0; i < argc - 1; ++i) {
            try {
                guests.push(std::atoi(argv[i]));        // Переводим аргументы из командной строки в int.
            } catch (std::exception) {
                std::cout << "Wrong input\n";
                return 0;
            }
        }
    }
    pthread_t room_threads[30];                           // Создаем массив потоков.
    for (int i = 0; i < 30; i++) {
        pthread_create(&room_threads[i], nullptr, rest_in_hotel, &rooms[i]);    // Создаем потоки.
        usleep(100000);     // Метод получает поток, в который передается метод его работы и комната.
    }                       // Затем программа немного ждет для удобства.

    for (int i = 0; i < 30; i++) {
        pthread_join(room_threads[i], nullptr);     // Соединяем все потоки, чтобы работали одновременно.
    }
    return 0;
}
