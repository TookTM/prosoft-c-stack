#include "cstack.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

// Структура элемента стека
struct node {
	struct node* prev;    // Указатель на предыдущий элемент (стек растет вниз)
	unsigned int size;    // Размер данных в элементе
	void* data;           // Указатель на данные элемента
};

// Структура дескриптора стека в глобальном списке
struct stack_list {
	struct stack_list* prev;  // Указатель на предыдущий дескриптор в списке
	struct stack_list* next;  // Указатель на следующий дескриптор в списке
	int descriptor;           // Уникальный числовой идентификатор стека
	struct node* stack_root;  // Указатель на корневой узел стека
};

// Глобальный корневой элемент списка всех стеков
struct stack_list list_root = { NULL, NULL, 0, NULL };

// Функция создания нового стека
hstack_t stack_new(void)
{
	// Выделяем память под новый дескриптор стека
	struct stack_list* new_root = (struct stack_list*)malloc(sizeof(struct stack_list));
	if (new_root) {
		// Инициализация временных переменных для поиска свободного дескриптора
		struct stack_list* temp;
		temp = &list_root;
		unsigned int free_disc = 0;  // Флаг найденного свободного дескриптора
		unsigned int disc = 0;       // Текущий проверяемый дескриптор

		// Проходим по всем существующим стекам для поиска свободного дескриптора
		while (temp->next != NULL) {
			temp = temp->next;
			if (stack_valid_handler(disc) && free_disc == 0) {
				free_disc = 1;  // Нашли свободный дескриптор
			}
			else if (free_disc != 1) {
				disc++;  // Продолжаем поиск
			}
		}

		// Вставляем новый дескриптор в глобальный список
		new_root->next = temp->next;
		new_root->prev = temp;
		temp->next = new_root;
		new_root->descriptor = disc;  // Устанавливаем найденный дескриптор

		// Выделяем память под корневой узел стека
		new_root->stack_root = (struct node*)malloc(sizeof(struct node));
		if (new_root->stack_root) {
			// Инициализируем корневой узел стека
			new_root->stack_root->prev = NULL;  // Корневой узел не имеет предыдущего
			new_root->stack_root->data = NULL;  // Корневой узел не содержит данных
			new_root->stack_root->size = 0;     // Размер данных корневого узла = 0

			return new_root->descriptor;  // Возвращаем дескриптор нового стека
		}
		else {
			// Ошибка: не удалось выделить память под корневой узел
			return -1;
		}
	}
	else {
		// Ошибка: не удалось выделить память под дескриптор стека
		return -1;
	}
}

// Функция полного удаления стека и освобождения всей памяти
void stack_free(const hstack_t hstack)
{
	// Проверяем, что дескриптор валиден (стек существует)
	if (!stack_valid_handler(hstack))
	{
		// Находим дескриптор удаляемого стека в глобальном списке
		struct stack_list* temp;
		temp = &list_root;
		temp = temp->next;
		while (temp->descriptor != hstack) {
			temp = temp->next;
		}

		// Последовательно удаляем все элементы стека
		while (stack_size(hstack) != 0) {
			struct node* stack_r = temp->stack_root;
			// Находим верхний элемент стека (последний добавленный)
			while (stack_r->prev != NULL) {
				stack_r = stack_r->prev;
			}
			// Находим элемент, который ссылается на верхний элемент
			struct node* last_node = temp->stack_root;
			while (last_node->prev != stack_r) {
				last_node = last_node->prev;
			}
			// Удаляем верхний элемент из цепочки
			last_node->prev = stack_r->prev;
			// Освобождаем память данных и самого элемента
			free(stack_r->data);
			free(stack_r);
		}

		// Освобождаем корневой узел стека
		free(temp->stack_root);

		// Удаляем дескриптор из глобального списка
		struct stack_list* prevtemp = temp->prev;
		if (temp->next != NULL) {
			// Если есть следующий элемент, перелинковываем
			struct stack_list* fortemp = temp->next;
			prevtemp->next = fortemp;
			fortemp->prev = prevtemp;
		}
		else {
			// Если это последний элемент, просто обрываем связь
			prevtemp->next = temp->next;
		}

		// Освобождаем память дескриптора стека
		free(temp);
	}
}

// Функция проверки валидности дескриптора стека
int stack_valid_handler(const hstack_t hstack)
{
	struct stack_list* temp = &list_root;
	int valid_handler = 1;  // Предполагаем, что дескриптор невалиден

	// Проходим по всем стекам в глобальном списке
	while (temp->next != NULL) {
		temp = temp->next;
		// Если нашли стек с заданным дескриптором, помечаем как валидный
		if (hstack == temp->descriptor) {
			valid_handler = 0;
		}
	}
	return valid_handler;  // 0 - валидный, 1 - невалидный
}

// Функция получения количества элементов в стеке
unsigned int stack_size(const hstack_t hstack)
{
	// Проверяем, что дескриптор валиден
	if (!stack_valid_handler(hstack))
	{
		// Находим нужный стек в глобальном списке
		struct stack_list* temp;
		temp = &list_root;
		temp = temp->next;
		while (temp->descriptor != hstack) {
			temp = temp->next;
		}

		// Подсчитываем количество элементов в стеке
		struct node* stack_r = temp->stack_root;
		unsigned int size = 0;
		while (stack_r->prev != NULL) {
			stack_r = stack_r->prev;
			size++;
		}
		return size;  // Возвращаем количество элементов
	}
	else {
		// Невалидный дескриптор - возвращаем 0
		return 0;
	}
}

// Функция добавления элемента в стек
void stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
	// Проверяем условия: валидный дескриптор, данные не NULL, размер > 0
	if (!stack_valid_handler(hstack) && data_in && size > 0) {
		// Находим нужный стек в глобальном списке
		struct stack_list* temp;
		temp = &list_root;
		temp = temp->next;
		while (temp->descriptor != hstack) {
			temp = temp->next;
		}

		// Находим текущий верхний элемент стека
		struct node* stack_r = temp->stack_root;
		while (stack_r->prev != NULL) {
			stack_r = stack_r->prev;
		}

		// Создаем новый элемент стека
		struct node* new_node = (struct node*)malloc(sizeof(struct node));
		if (new_node) {
			// Инициализируем новый элемент
			new_node->prev = stack_r->prev;  // Связываем с предыдущей цепочкой
			new_node->size = size;           // Сохраняем размер данных
			new_node->data = malloc(size);   // Выделяем память под данные

			// Связываем текущий верхний элемент с новым элементом
			stack_r->prev = new_node;

			// Копируем данные в новый элемент
			if (new_node->data) {
				memcpy(new_node->data, data_in, size);
			}
		}
	}
}

// Функция извлечения элемента из стека
unsigned int stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
	// Проверяем условия: валидный дескриптор и стек не пустой
	if (!stack_valid_handler(hstack) && stack_size(hstack) != 0) {
		// Находим нужный стек в глобальном списке
		struct stack_list* temp;
		temp = &list_root;
		temp = temp->next;
		while (temp->descriptor != hstack) {
			temp = temp->next;
		}

		// Находим верхний элемент стека (последний добавленный)
		struct node* stack_r = temp->stack_root;
		while (stack_r->prev != NULL) {
			stack_r = stack_r->prev;
		}

		// Проверяем, что буфер достаточного размера и не NULL
		if (stack_r->size <= size && data_out != NULL) {
			// Копируем данные из верхнего элемента в буфер
			memcpy(data_out, stack_r->data, stack_r->size);
			unsigned int result;
			result = stack_r->size;  // Сохраняем размер скопированных данных

			// Находим элемент, который ссылается на верхний элемент
			struct node* prev_stack = temp->stack_root;
			while (prev_stack->prev != stack_r) {
				prev_stack = prev_stack->prev;
			}
			// Удаляем верхний элемент из цепочки
			prev_stack->prev = stack_r->prev;

			// Освобождаем память данных и самого элемента
			free(stack_r->data);
			free(stack_r);

			// Возвращаем размер скопированных данных
			return result;
		}
		else {
			// Буфер слишком мал или NULL - возвращаем 0
			return 0;
		}
	}
	else {
		// Невалидный дескриптор или пустой стек - возвращаем 0
		return 0;
	}
}

