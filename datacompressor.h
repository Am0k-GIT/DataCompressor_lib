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

template <typename T>

class DataCompressor {

private:
	uint8_t m_cashIndex = 0;                                                                          // индекс кеша
	uint16_t m_stackIndex = 0;                                                                        // индекс стека
	uint8_t m_cashSize = 0;                                                                           // размер кеша
	uint16_t m_stackSize = 0;                                                                         // размер стека
	uint8_t m_cashPrecision = 20;                                                                     // %, значения с отклонением выше которого отбрасываются при усреднениях
	bool m_full;                                                                                      // флаг полного заполнения стека
	T* m_cashArray;                                                                                   // динамический массив для кеша
	T* m_stackArray;                                                                                  // динамический массив для стека

	void cashToStack(uint8_t count)                                                                   // сбрасываем весь кеш в одно значение стека
	{
		sortArray(m_cashArray, count, ascending);
		addToStack(filterValue(m_cashArray, count, medianValue, m_cashPrecision));
	}

	void addToStack(T data)                                                                           // добавляем данные в стек
	{
		if (m_full)
		{
			for (uint16_t i = 0; i < (m_stackSize - 1); i++)
			{
				m_stackArray[i] = m_stackArray[i + 1];
			}
			m_stackArray[m_stackIndex] = data;
		}
		else
		{
			m_stackArray[m_stackIndex] = data;
			if (m_stackIndex < (m_stackSize - 1))
			{
				m_stackIndex++;
			}
			else
			{
				m_full = true;
			}
		}
		m_cashIndex = 0;
	}

public:


	DataCompressor(uint8_t CashSize, uint16_t StackSize)                                              // конструктор по умолчанию
	{
		m_cashSize = CashSize;
		m_stackSize = StackSize;
		m_cashArray = new T[m_cashSize];
		m_stackArray = new T[m_stackSize];
		m_full = 0;
	}

	DataCompressor(const DataCompressor& source)                                                      // конструктор копирования
	{
		m_cashSize = source.m_cashSize;
		m_stackSize = source.m_stackSize;
		m_cashIndex = source.m_cashIndex;
		m_stackIndex = source.m_stackIndex;
		m_cashPrecision = source.m_cashPrecision;
		m_full = source.m_full;
		if (source.m_cashArray)
		{
			m_cashArray = new T[m_cashSize];
			for (uint8_t i = 0; i < m_cashSize; ++i)
				m_cashArray[i] = source.m_cashArray[i];
		}
		else
			m_cashArray = 0;
		if (source.m_stackArray)
		{
			m_stackArray = new T[m_stackSize];
			for (uint16_t i = 0; i < m_stackSize; ++i)
				m_stackArray[i] = source.m_stackArray[i];
		}
		else
			m_stackArray = 0;
	}

	~DataCompressor()                                                                                 // деструктор
	{
		delete[] m_cashArray;
		delete[] m_stackArray;
	}

	DataCompressor& operator=(const DataCompressor& source)                                           // перегрузка =
	{
		if (this == &source)
			return *this;
		m_cashSize = source.m_cashSize;
		m_stackSize = source.m_stackSize;
		m_cashIndex = source.m_cashIndex;
		m_stackIndex = source.m_stackIndex;
		m_cashPrecision = source.m_cashPrecision;
		m_full = source.m_full;
		delete[] m_cashArray;
		delete[] m_stackArray;
		if (source.m_cashArray)
		{
			m_cashArray = new T[m_cashSize];
			for (uint8_t i = 0; i < m_cashSize; ++i)
				m_cashArray[i] = source.m_cashArray[i];
		}
		else
			m_cashArray = 0;
		if (source.m_stackArray)
		{
			m_stackArray = new T[m_stackSize];
			for (uint16_t i = 0; i < m_stackSize; ++i)
				m_stackArray[i] = source.m_stackArray[i];
		}
		else
			m_stackArray = 0;
		return *this;
	}

	T& operator[] (const uint16_t index)                                                              // перегрузка []
	{
		return m_stackArray[index];
	}

	uint16_t getLastIndex()                                                                           // получение последнего записанного индекса в стеке
	{
		if (m_full)
			return (m_stackSize - 1);
		else
			return (m_stackIndex - 1);
	}

	void setCashPrecision(uint8_t percent)                                                            // установка %, значения с отклонением выше которого отбрасываются при усреднениях
	{
		m_cashPrecision = percent;
	}

	
	void push(T data)                                                                                 // добавление данных
	{
		m_cashArray[m_cashIndex] = data;
		m_cashIndex++;
		if (m_cashIndex > (m_cashSize - 1))
		{
			cashToStack(m_cashSize);
		}
	}
	
	bool full()                                                                                       // проверка на заполненность стека
	{
		return m_full;
	}
	
	float getAverage()                                                                                // получение усредненного значения
	{
		if (m_cashIndex > 0)
		{
			cashToStack(m_cashIndex);
		}
		return averageValue(m_stackArray, getLastIndex());
	}
	
	float getMedian()                                                                                 // получение медианного значения
	{
		if (m_cashIndex > 0)
		{
			cashToStack(m_cashIndex);
		}
		return  medianValue(m_stackArray, getLastIndex());
	}

	float getFiltered(float (*referenceFcn)(T* array, uint16_t length), uint8_t maxDiffPercent)       // получение отфильтрованного значения
	{
		if (m_cashIndex > 0)
		{
			cashToStack(m_cashIndex);
		}
		return(filterValue(m_stackArray, getLastIndex(), referenceFcn, maxDiffPercent));
	}
};