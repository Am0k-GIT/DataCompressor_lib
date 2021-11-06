// Библиотека для хранения и обработки данных с плавающей точкой.
// Временное хранение осуществляется в кеше (до 256 значений), 
// прямого доступа к которому нет.
// Усредненные значения из кеша при его заполнению попадают в 
// стек (до 65536 значений),
// перезаписываемый по кругу при заполнении.
// Из стека можно получить как конкретное значение по индексу, так и обработанный
// результат - усредненное, медианное, отфильтрованное значение согласно максимальному
// % отклонения от усредненного или медианного значения.

#pragma once

#include <Arduino.h>
#include <statistics.h>

class DataCompressor {

private:
	uint8_t m_cashIndex = 0;                                                                          // индекс кеша
	uint16_t m_stackIndex = 0;                                                                        // индекс стека
	uint8_t m_cashSize = 0;                                                                           // размер кеша
	uint16_t m_stackSize = 0;                                                                         // размер стека
	uint8_t m_cashPrecision = 20;                                                                     // %, значения с отклонением выше которого отбрасываются при усреднениях
	bool m_full;                                                                                      // флаг полного заполнения стека
	float* m_cashArray;                                                                               // динамический массив для кеша
	float* m_stackArray;                                                                              // динамический массив для стека

	void cashToStack(uint8_t count);                                                                  // сбрасываем весь кеш в одно значение стека
	void addToStack(float data);                                                                      // добавляем данные в стек

public:

	DataCompressor(uint8_t CashSize, uint16_t StackSize);                                             // конструктор по умолчанию
	DataCompressor(const DataCompressor& source);                                                     // конструктор копирования
	~DataCompressor();                                                                                // деструктор

	DataCompressor& operator=(const DataCompressor& source);                                          // перегрузка =
	float& operator[] (const uint16_t index);                                                         // перегрузка []

	uint16_t getLastIndex();                                                                          // получение последнего записанного индекса в стеке
	void setCashPrecision(uint8_t percent);                                                           // установка %, значения с отклонением выше которого отбрасываются при усреднениях
	void push(float data);                                                                            // добавление данных
	bool full();                                                                                      // проверка на заполненность стека
	float getAverage();                                                                               // получение усредненного значения
	float getMedian();                                                                                // получение медианного значения
	float getFiltered(float (*referenceFcn)(float* array, uint16_t length), uint8_t maxDiffPercent);  // получение отфильтрованного значения
};
