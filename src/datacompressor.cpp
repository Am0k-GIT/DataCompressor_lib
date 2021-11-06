#include <Arduino.h>
#include "datacompressor.h"

void DataCompressor::cashToStack(uint8_t count)
{
	sortArray(m_cashArray, count, ascending);
	addToStack(filterValue(m_cashArray, count, medianValue, m_cashPrecision));
}

void DataCompressor::addToStack(float data)
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

DataCompressor::DataCompressor(uint8_t CashSize, uint16_t StackSize)
{
	m_cashSize = CashSize;
	m_stackSize = StackSize;
	m_cashArray = new float[m_cashSize];
	m_stackArray = new float[m_stackSize];
	m_full = 0;
}

DataCompressor::DataCompressor(const DataCompressor& source)
{
	m_cashSize = source.m_cashSize;
	m_stackSize = source.m_stackSize;
	m_cashIndex = source.m_cashIndex;
	m_stackIndex = source.m_stackIndex;
	m_cashPrecision = source.m_cashPrecision;
	m_full = source.m_full;
	if (source.m_cashArray)
	{
		m_cashArray = new float[m_cashSize];
		for (uint8_t i = 0; i < m_cashSize; ++i)
			m_cashArray[i] = source.m_cashArray[i];
	}
	else
		m_cashArray = 0;
	if (source.m_stackArray)
	{
		m_stackArray = new float[m_stackSize];
		for (uint16_t i = 0; i < m_stackSize; ++i)
			m_stackArray[i] = source.m_stackArray[i];
	}
	else
		m_stackArray = 0;
}

DataCompressor::~DataCompressor()
{
	delete[] m_cashArray;
	delete[] m_stackArray;
}

DataCompressor& DataCompressor::operator=(const DataCompressor& source)
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
		m_cashArray = new float[m_cashSize];
		for (uint8_t i = 0; i < m_cashSize; ++i)
			m_cashArray[i] = source.m_cashArray[i];
	}
	else
		m_cashArray = 0;
	if (source.m_stackArray)
	{
		m_stackArray = new float[m_stackSize];
		for (uint16_t i = 0; i < m_stackSize; ++i)
			m_stackArray[i] = source.m_stackArray[i];
	}
	else
		m_stackArray = 0;
	return *this;
}

float& DataCompressor::operator[] (const uint16_t index)
{
	return m_stackArray[index];
}

uint16_t DataCompressor::getLastIndex()
{
	if (m_full)
		return (m_stackSize - 1);
	else
		return (m_stackIndex - 1);
}

void DataCompressor::setCashPrecision(uint8_t percent)
{
	m_cashPrecision = percent;
}

void DataCompressor::push(float data)
{
	m_cashArray[m_cashIndex] = data;
	m_cashIndex++;
	if (m_cashIndex > (m_cashSize - 1))
	{
		cashToStack(m_cashSize);
	}
}

bool DataCompressor::full()
{
	return m_full;
}

float DataCompressor::getAverage()
{
	if (m_cashIndex > 0)
	{
		cashToStack(m_cashIndex);
	}
	return averageValue(m_stackArray, getLastIndex());
}

float DataCompressor::getMedian()
{
	if (m_cashIndex > 0)
	{
		cashToStack(m_cashIndex);
	}
	return  medianValue(m_stackArray, getLastIndex());
}

float DataCompressor::getFiltered(float (*referenceFcn)(float* array, uint16_t length), uint8_t maxDiffPercent)
{
	if (m_cashIndex > 0)
	{
		cashToStack(m_cashIndex);
	}

	return(filterValue(m_stackArray, getLastIndex(), referenceFcn, maxDiffPercent));
}