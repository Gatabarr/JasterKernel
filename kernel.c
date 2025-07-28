#include "keyboard_map.h"

/* Объявления внешних функций */
extern void keyboard_handler(void);
extern void write_port(unsigned short port, unsigned char data);
extern unsigned char read_port(unsigned short port);
extern void load_idt(unsigned long *idt_ptr);

/* Параметры экрана */
#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE (BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE * LINES)

/* Порты клавиатуры */
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

/* Настройки IDT */
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08

/* Коды клавиш */
#define ENTER_KEY_CODE 0x1C
#define BACKSPACE_KEY_CODE 0x0E
#define ESC_KEY_CODE 0x01

/* Видеопамять */
unsigned int current_loc = 0;
char *vidptr = (char*)0xb8000;

/* Буфер ввода */
#define INPUT_BUFFER_SIZE 256
char input_buffer[INPUT_BUFFER_SIZE];
unsigned int input_buffer_pos = 0;

/* Режим калькулятора */
int calculator_mode = 0;

/* Структура записи IDT */
struct IDT_entry {
    unsigned short int offset_lowerbits;
    unsigned short int selector;
    unsigned char zero;
    unsigned char type_attr;
    unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];

/* Простая реализация strcmp */
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

/* Инициализация IDT */
void idt_init(void) {
    unsigned long keyboard_address = (unsigned long)keyboard_handler;
    unsigned long idt_address;
    unsigned long idt_ptr[2];

    /* Настройка обработчика клавиатуры */
    IDT[0x21].offset_lowerbits = keyboard_address & 0xffff;
    IDT[0x21].selector = KERNEL_CODE_SEGMENT_OFFSET;
    IDT[0x21].zero = 0;
    IDT[0x21].type_attr = INTERRUPT_GATE;
    IDT[0x21].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;

    /* Инициализация PIC */
    write_port(0x20, 0x11);
    write_port(0xA0, 0x11);
    write_port(0x21, 0x20);
    write_port(0xA1, 0x28);
    write_port(0x21, 0x04);
    write_port(0xA1, 0x02);
    write_port(0x21, 0x01);
    write_port(0xA1, 0x01);
    write_port(0x21, 0xFD); /* Разрешить только прерывания клавиатуры */
    write_port(0xA1, 0xFF); /* Запретить все прерывания от slave PIC */

    /* Загрузка IDT */
    idt_address = (unsigned long)IDT;
    idt_ptr[0] = (sizeof(struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
    idt_ptr[1] = idt_address >> 16;
    load_idt(idt_ptr);
}

/* Вывод строки */
void kprint(const char *str) {
    unsigned int i = 0;
    while (str[i] != '\0') {
        vidptr[current_loc++] = str[i++];
        vidptr[current_loc++] = 0x07;
    }
}

/* Новая строка */
void kprint_newline(void) {
    unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
    current_loc = current_loc + (line_size - current_loc % (line_size));
}

/* Очистка экрана */
void clear_screen(void) {
    unsigned int i = 0;
    while (i < SCREENSIZE) {
        vidptr[i++] = ' ';
        vidptr[i++] = 0x07;
    }
    current_loc = 0;
}

/* Показать главное меню */
void show_main_menu(void) {
    clear_screen();
    kprint("Welcome to Jaster Kernel!");
    kprint_newline();
    kprint("Functions, to select, type and press Enter");
    kprint_newline();
    kprint_newline();
    kprint("calculator - calc");
    kprint_newline();
    kprint("Your command: ");
    calculator_mode = 0;
    input_buffer_pos = 0;
}

/* Упрощенный калькулятор */
void simple_calculator(void) {
    int num1 = 0, num2 = 0, result = 0;
    char op = 0;
    int i = 0;
    
    /* Парсим первое число */
    while (input_buffer[i] >= '0' && input_buffer[i] <= '9') {
        num1 = num1 * 10 + (input_buffer[i] - '0');
        i++;
    }
    
    /* Получаем оператор */
    op = input_buffer[i++];
    
    /* Парсим второе число */
    while (input_buffer[i] >= '0' && input_buffer[i] <= '9') {
        num2 = num2 * 10 + (input_buffer[i] - '0');
        i++;
    }
    
    /* Выполняем операцию */
    switch (op) {
        case '+': result = num1 + num2; break;
        case '-': result = num1 - num2; break;
        case '*': result = num1 * num2; break;
        case '/': 
            if (num2 == 0) {
                kprint("Error: Division by zero!");
                kprint_newline();
                return;
            }
            result = num1 / num2;
            break;
        default:
            kprint("Error: Invalid operator!");
            kprint_newline();
            return;
    }
    
    /* Выводим результат */
    kprint("Result: ");
    
    /* Выводим первое число */
    char num_str[32];
    int n = num1, pos = 0;
    do {
        num_str[pos++] = n % 10 + '0';
        n /= 10;
    } while (n > 0);
    
    for (int j = pos-1; j >= 0; j--) {
        vidptr[current_loc++] = num_str[j];
        vidptr[current_loc++] = 0x07;
    }
    
    /* Выводим оператор */
    vidptr[current_loc++] = ' ';
    vidptr[current_loc++] = 0x07;
    vidptr[current_loc++] = op;
    vidptr[current_loc++] = 0x07;
    vidptr[current_loc++] = ' ';
    vidptr[current_loc++] = 0x07;
    
    /* Выводим второе число */
    n = num2;
    pos = 0;
    do {
        num_str[pos++] = n % 10 + '0';
        n /= 10;
    } while (n > 0);
    
    for (int j = pos-1; j >= 0; j--) {
        vidptr[current_loc++] = num_str[j];
        vidptr[current_loc++] = 0x07;
    }
    
    /* Выводим равно и результат */
    kprint(" = ");
    
    n = result;
    pos = 0;
    do {
        num_str[pos++] = n % 10 + '0';
        n /= 10;
    } while (n > 0);
    
    for (int j = pos-1; j >= 0; j--) {
        vidptr[current_loc++] = num_str[j];
        vidptr[current_loc++] = 0x07;
    }
    
    kprint_newline();
}

/* Обработчик прерывания клавиатуры */
void keyboard_handler_main(void) {
    unsigned char status;
    char keycode;

    /* Читаем статус клавиатуры */
    status = read_port(KEYBOARD_STATUS_PORT);
    if (!(status & 0x01)) {
        goto end;
    }

    /* Читаем скан-код клавиши */
    keycode = read_port(KEYBOARD_DATA_PORT);
    
    /* Обработка Esc - возврат в главное меню */
    if (keycode == ESC_KEY_CODE) {
        show_main_menu();
        goto end;
    }
    
    /* Игнорируем отпускание клавиши */
    if (keycode & 0x80) {
        goto end;
    }

    /* Обработка Enter */
    if (keycode == ENTER_KEY_CODE) {
        input_buffer[input_buffer_pos] = '\0';
        
        if (!calculator_mode) {
            if (strcmp(input_buffer, "calc") == 0) {
                calculator_mode = 1;
                clear_screen();
                kprint("=== Simple Calculator ===");
                kprint_newline();
                kprint("Operators: /, *, +, -");
                kprint_newline();
                kprint("Enter expression (e.g. 5+3) or press Esc to exit");
                kprint_newline();
                kprint("> ");
            }
        } else {
            if (input_buffer_pos > 0) {
                kprint_newline();
                simple_calculator();
                kprint("> ");
            }
        }
        input_buffer_pos = 0;
        goto end;
    }

    /* Обработка Backspace */
    if (keycode == BACKSPACE_KEY_CODE) {
        if (input_buffer_pos > 0) {
            input_buffer_pos--;
            current_loc -= 2;
            vidptr[current_loc] = ' ';
            vidptr[current_loc + 1] = 0x07;
        }
        goto end;
    }

    /* Обработка обычных клавиш */
    char key = keyboard_map[(unsigned char)keycode];
    if (key && input_buffer_pos < INPUT_BUFFER_SIZE - 1) {
        if (calculator_mode) {
            if ((key >= '0' && key <= '9') || key == '+' || key == '-' || 
                key == '*' || key == '/') {
                input_buffer[input_buffer_pos++] = key;
                vidptr[current_loc++] = key;
                vidptr[current_loc++] = 0x07;
            }
        } else {
            input_buffer[input_buffer_pos++] = key;
            vidptr[current_loc++] = key;
            vidptr[current_loc++] = 0x07;
        }
    }

end:
    /* Отправка EOI */
    write_port(0x20, 0x20);
}

/* Точка входа */
void kmain(void) {
    /* Инициализация */
    show_main_menu();

    /* Инициализация буфера ввода */
    input_buffer[0] = '\0';

    /* Настройка прерываний */
    idt_init();

    /* Включение прерываний */
    __asm__("sti");

    /* Основной цикл */
    while(1) {
        __asm__("hlt");
    }
}